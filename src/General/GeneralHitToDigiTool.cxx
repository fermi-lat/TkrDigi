/*
 * @file GeneralHitToDigiTool.cxx
 *
 * @brief Converts tkr hits into tkr digis.  This tool is a merge of code used
 * in the packages GlastDigi v4r7 and TkrDigi v1r11p2.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/General/GeneralHitToDigiTool.cxx,v 1.1 2004/02/24 13:57:34 kuss Exp $
 */

#include "GeneralHitToDigiTool.h"
#include "GeneralNoiseTool.h"

#include "../SiStripList.h"
#include "../SiPlaneMapContainer.h"
#include "../TkrVolumeIdentifier.h"

// Glast specific includes
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/TopLevel/DigiEvent.h"
#include "Event/Digi/TkrDigi.h"
#include "Event/MonteCarlo/McTkrStrip.h"
#include "Event/RelTable/RelTable.h"
#include "Event/RelTable/Relation.h"
#include "Event/MonteCarlo/McPositionHit.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"

#include <sstream>


static const ToolFactory<GeneralHitToDigiTool>    s_factory;
const IToolFactory& GeneralHitToDigiToolFactory = s_factory;

double GeneralHitToDigiTool::s_totAt1Mip    = 43.8;
double GeneralHitToDigiTool::s_mevPerMip    = 0.155;
double GeneralHitToDigiTool::s_totThreshold =GeneralNoiseTool::noiseThreshold();
int    GeneralHitToDigiTool::s_totMax       = 250;
int    GeneralHitToDigiTool::s_maxHits      = 63;


GeneralHitToDigiTool::GeneralHitToDigiTool(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent) :
    AlgTool(type, name, parent) {
    //Declare the additional interface
    declareInterface<IHitToDigiTool>(this);

    declareProperty("killBadStrips", m_killBadStrips = false );
    declareProperty("killFailed",    m_killFailed    = true  );
    declareProperty("totAt1Mip",     s_totAt1Mip);
    declareProperty("mevPerMip",     s_mevPerMip);
    declareProperty("totThreshold",  s_totThreshold);
    declareProperty("totMax",        s_totMax);
    declareProperty("maxHits",       s_maxHits);
}


StatusCode GeneralHitToDigiTool::initialize()
{
    // Purpose and Method: initializes the tool
    // Inputs: None
    // Outputs: status code
    // Dependencies: depends on several services
    // Restrictions and Caveats: None

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize " << name() << endreq;

    if ( s_totThreshold != GeneralNoiseTool::noiseThreshold() )
        log << MSG::WARNING
            << "You are declaring a different threshold for the ToT ("
            << totThreshold() << ") than for the noise ("
            << GeneralNoiseTool::noiseThreshold() << ")!" << endreq;
    //    std::exit(42);

    IService* iService = 0;
    sc = serviceLocator()->service("EventDataSvc", iService, true );
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc!" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);

    // Get the Tkr Geometry service 
    sc = service("TkrGeometrySvc", m_tgSvc, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
        return sc;
    }

    // Get the failure mode service 
    m_tfmSvc = m_tgSvc->getTkrFailureModeSvc();
    if ( !m_tfmSvc )
        log << MSG::WARNING
            << "Couldn't set up TkrFailureModeSvc!" << std::endl
            << "Will assume it is not required!" << endreq;

    // Get the alignment service 
    m_taSvc = m_tgSvc->getTkrAlignmentSvc();
    if ( !m_taSvc )
        log << MSG::WARNING
            << "Couldn't set up TkrAlignmentSvc!" << std::endl
            << "Will assume it is not required!" << endreq;

    // Get the bad strips service 
    m_tbsSvc = m_tgSvc->getTkrBadStripsSvc();
    if ( !m_tbsSvc )
        log << MSG::WARNING
            << "Couldn't set up TkrBadStripsSvc!" << std::endl
            << "Will assume it is not required!" << endreq;

    // Get the Glast detector service 
    sc = service("GlastDetSvc", m_gdSvc);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up GlastDetSvc!" << endreq;
        return sc;
    }

    // pass the GlastDetSvc pointer to the SiStripList static functions
    if ( SiStripList::initialize(m_gdSvc).isFailure() ) {
        log << MSG::ERROR << "Couldn't initialize SiStripList" << endreq;
        return sc;
    }

    log << MSG::INFO
        << "ssdgap " << SiStripList::ssd_gap() 
        << " laddergap " << SiStripList::ladder_gap()
        << " strips/wafer " << SiStripList::strips_per_die() 
        << " #ladders " << SiStripList::n_si_dies() 
        << " waferside " << SiStripList::die_width() 
        << " deadgap " << SiStripList::guard_ring() 
        << endreq;

    return sc;
}


StatusCode GeneralHitToDigiTool::execute()
{
    // Purpose and Method: creates TkrDigis from the SiPlaneMap
    // Inputs: None
    // Outputs: None
    // TDS Inputs: /Event/tmp/siPlaneMapContainer
    // TDS Outputs: EventModel::MC::McTkrStripCol, EventModel::Digi::TkrDigiCol,
    //              EventModel::Digi::TkrDigiHitTab
    // Dependencies: None
    // Restrictions and Caveats: The TDS output is a hard-coded string

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "execute " << name() << endreq;

    // Take care of insuring that the data area has been created
    DataObject* pDummy;
    sc = m_edSvc->retrieveObject(EventModel::Digi::Event, pDummy);
    if (sc.isFailure()) {
        sc = m_edSvc->registerObject(EventModel::Digi::Event, new Event::DigiEvent);
        if( sc.isFailure() ) {
            log << MSG::ERROR << "failed to register "
                << EventModel::Digi::Event << endreq;
            return sc;
        }
    }

    // Create the collection of hit strip objects
    Event::McTkrStripCol* strips = new Event::McTkrStripCol;
    sc = m_edSvc->registerObject(EventModel::MC::McTkrStripCol, strips);
    if (sc != StatusCode::SUCCESS){
        log << MSG::INFO << "failed to register "
            << EventModel::MC::McTkrStripCol << endreq;
        return sc;
    }

    //Create the collection of digi objects - will be empty at this point
    Event::TkrDigiCol* pTkrDigi = new Event::TkrDigiCol;
    
    // Add this new collection to the TDS
    sc = m_edSvc->registerObject(EventModel::Digi::TkrDigiCol, pTkrDigi);
    if (sc != StatusCode::SUCCESS){
        log << MSG::INFO << "failed to register "
            << EventModel::Digi::TkrDigiCol << endreq;
        return sc;
    }

    int nDigi[2] = { 0, 0 };
    int nStrip[2] = { 0, 0 };
    int nStrips = 0;

    // Create the relational table
    typedef Event::Relation<Event::TkrDigi,Event::McPositionHit> relType;
    typedef ObjectList<relType> tabType;
    
    Event::RelTable<Event::TkrDigi, Event::McPositionHit> digiHit;
    digiHit.init();
    tabType* pRelTab = digiHit.getAllRelations();
    
    sc = m_edSvc->registerObject(EventModel::Digi::TkrDigiHitTab, pRelTab);
    if (sc.isFailure()) {
        log << MSG::ERROR<< "failed to register "
            << EventModel::Digi::TkrDigiHitTab << endreq;
        return sc;
    }
    
    // retrieve the pointer to the SiPlaneMapContainer from TDS
    SmartDataPtr<SiPlaneMapContainer> pObject(m_edSvc,
                                              "/Event/tmp/siPlaneMapContainer");
    if ( !pObject ) {
        log << MSG::ERROR
            << "could not retrieve /Event/tmp/siPlaneMapContainer" << endreq;
        return sc;
    }

    SiPlaneMapContainer::SiPlaneMap& siPlaneMap = pObject->getSiPlaneMap();

    // finally make digis from the hits
    for ( SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();
          itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId tower = volId.getTower();
        const int theTower = tower.id();
        const int bilayer = volId.getLayer();
        const int view  = volId.getView();
        const idents::GlastAxis::axis axis = volId.getAxis();

        if ( m_killFailed && m_tfmSvc && !m_tfmSvc->empty() 
            && m_tfmSvc->isFailed(theTower, bilayer, view) )
            continue;
        
        // Loop over contained list of strips to remove the bad strips.
        // If the TDS SiPlaneMapContainer will become PDS one day, and also bad
        // strips should be kept, this code would have to be modified.
        SiStripList::iterator itStrip=sList->begin();
        for ( ;itStrip!=sList->end(); ++itStrip ) {
            const int stripId = itStrip->index();
            // kill bad strips
            if ( m_killBadStrips && m_tbsSvc && !m_tbsSvc->empty() 
                 && m_tbsSvc->isBadStrip(theTower, bilayer, axis, stripId) )
                itStrip->setBadStrip();
        }

        static const int breakPoint = SiStripList::n_si_strips() / 2;
        int ToT[2] = { 0, 0 };
        ToT[0] = sList->getToT0(breakPoint);
        ToT[1] = sList->getToT1(breakPoint);
        const int ToTlayer = sList->getToT1();  // full plane ToT (debugging)

        Event::TkrDigi* pDigi = new Event::TkrDigi(bilayer, axis, tower, ToT);
        nStrips = 0;

        log << MSG::DEBUG << "tower " << tower.id() << " bilayer " << bilayer
            << " view " << axis << " ToT " << ToT[0] << " " << ToT[1]
            << " ( " << ToTlayer << " )"
            << endreq;

        // now loop over contained list of strips
        for (itStrip=sList->begin(); itStrip!=sList->end(); ++itStrip ) {
            if (itStrip->badStrip()) continue;
            const int stripId = itStrip->index();
            nStrip[view]++;
            nStrips++;
            
            // add the strip to the correct controller
            if ( stripId < breakPoint )
                pDigi->addC0Hit(stripId);
            else
                pDigi->addC1Hit(stripId);
            log << MSG::DEBUG
                << "    strip " << itStrip->index()
                << " energy " << itStrip->energy()
                << " noise " << itStrip->noise()
                << endreq;
            // get the alignment offset of the strip
            HepPoint3D point(0);
            double deltaX = 0;
            double deltaY = 0;
            if ( m_taSvc && m_taSvc->alignSim() ) {
                HepPoint3D entry(0., 0., 0.);
                HepPoint3D  exit(0., 0., 1.);
                m_taSvc->moveMCHit(volId, entry, exit);
                deltaX = -entry.x();
                deltaY = -entry.y();
            }

            const SiStripList::hitList& hits = itStrip->getHits();

            // save the hit here
            Event::McTkrStrip* pStrip =
                new Event::McTkrStrip(volId, stripId,
                                      itStrip->energy(),
                                      itStrip->noise(), hits, deltaX, deltaY);
            strips->push_back(pStrip);
            
            // and add the relation
            std::ostringstream ost;
            ost << std::setw(4) << stripId;
            for ( Event::McTkrStrip::hitList::const_iterator
                      itHit=pStrip->begin(); itHit!=pStrip->end(); ++itHit ) {
                Event::McPositionHit* pHit = *itHit;
                relType *rel = new relType(pDigi, pHit);
                rel->addInfo(ost.str());
                //addRelation now does the right thing with duplicates
                // namely, appends the info to the existing info
                unsigned int size = digiHit.size();
                digiHit.addRelation(rel);
                if (size==digiHit.size()) delete rel;
            }
        }

		// keep only the "first" s_maxHits hits on each side
		// "first" means first to arrive at the controller, so lowest number strips
		// on the low side, highest number on the high side.
		// only need to check this if there are more than s_maxHits hits
		int size = pDigi->getNumHits();
		if (size>s_maxHits) {
			int ihit = 0;
			int count = 0;
			int lastC0Strip = pDigi->getLastController0Strip();
			while (ihit<pDigi->getNumHits()) {
				int index = pDigi->getHit(ihit);
				if (index>lastC0Strip) {
					// we're in the high side, remove the hit from the front
		    		size = pDigi->getNumHits();
					if (ihit >= size-s_maxHits) break; // were under the limit now, no more to remove
					pDigi->removeHit(index);
					nStrips--;
					continue;
				}
				count++;
				if (count<=s_maxHits) {
					// on the low side, but haven't hit the limit yet; keep counting
					ihit++;
					continue;
				}
				// over the limit on the low side; remove
				pDigi->removeHit(index);
				nStrips--;
			}
		}

        if ( nStrips > 0 ) {
            pTkrDigi->push_back(pDigi);
            nDigi[view]++;
        }
        else
            delete pDigi;
    }

    // sort by volume id
    std::sort(pTkrDigi->begin(), pTkrDigi->end(), Event::TkrDigi::digiLess());

    log << MSG::DEBUG << " X digis/strips: " << nDigi[0] << " " << nStrip[0]
        << " Y digis/strips: " << nDigi[1] << " " << nStrip[1] << endreq;

    return sc;
}
