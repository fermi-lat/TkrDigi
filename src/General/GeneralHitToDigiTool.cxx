/*
* @file GeneralHitToDigiTool.cxx
*
* @brief Converts tkr hits into tkr digis.  This tool is a merge of code used
* in the packages GlastDigi v4r7 and TkrDigi v1r11p2.
*
* @author Michael Kuss
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralHitToDigiTool.cxx,v 1.8 2005/04/11 22:49:50 lsrea Exp $
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

double GeneralHitToDigiTool::s_totThreshold =GeneralNoiseTool::noiseThreshold();
//int    GeneralHitToDigiTool::s_maxHits      = 64;


GeneralHitToDigiTool::GeneralHitToDigiTool(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent) :
AlgTool(type, name, parent) {
    //Declare the additional interface
    declareInterface<IHitToDigiTool>(this);

    declareProperty("killBadStrips", m_killBadStrips = true );
    declareProperty("killFailed",    m_killFailed    = true );
    declareProperty("totThreshold",  s_totThreshold);
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
    log << MSG::INFO << "initialize " << endreq;

    setProperties();

    if(!m_killFailed || !m_killBadStrips) {
        log << MSG::ERROR 
            << "killFailed and killBadStrips now set in GeneralTruncationTool" 
            << endreq;
        return StatusCode::FAILURE;
    }

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
    sc = service("TkrGeometrySvc", m_tkrGeom, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
        return sc;
    }

    // Get the failure mode service 
    m_tfmSvc = m_tkrGeom->getTkrFailureModeSvc();
    if ( !m_tfmSvc )
        log << MSG::WARNING
        << "Couldn't set up TkrFailureModeSvc!" << std::endl
        << "Will assume it is not required!" << endreq;

    // Get the alignment service 
    m_taSvc = m_tkrGeom->getTkrAlignmentSvc();
    if ( !m_taSvc )
        log << MSG::WARNING
        << "Couldn't set up TkrAlignmentSvc!" << std::endl
        << "Will assume it is not required!" << endreq;

    // Get the bad strips service 
    m_tbsSvc = m_tkrGeom->getTkrBadStripsSvc();
    if ( !m_tbsSvc )
        log << MSG::WARNING
        << "Couldn't set up TkrBadStripsSvc!" << std::endl
        << "Will assume it is not required!" << endreq;

    // Get the splits service 
    m_tspSvc = m_tkrGeom->getTkrSplitsSvc();
    if ( !m_tspSvc ) {
        log << MSG::ERROR
            << "Couldn't set up TkrSplitsSvc!" << std::endl;
        return StatusCode::FAILURE;
    }

    // Get the ToT service 
    m_ttotSvc = m_tkrGeom->getTkrToTSvc();
    if ( !m_ttotSvc ) {
        log << MSG::ERROR
            << "Couldn't set up TkrToTSvc!" << std::endl;
        return StatusCode::FAILURE;
    }

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
    log << MSG::DEBUG << "execute " << endreq;

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
    SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();
    for ( ; itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId tower = volId.getTower();
        const int theTower = tower.id();
        //const int tray  = volId.getTray();
        //const int face  = volId.getBotTop();
        const int bilayer = volId.getLayer();
        const int view  = volId.getView();
        const idents::GlastAxis::axis axis = volId.getAxis();

        // Failed planes and bad strips now handled in TkrDigiTruncationTool

        //  breakPoint is defined as the highest C0 strip, 
        //       ordinarily 767 for the flight instrument
        int breakPoint = m_tspSvc->getSplitPoint(theTower, bilayer, view);
        int ToT[2] = { 0, 0 };
        sList->getToT(ToT, theTower, bilayer, view, m_ttotSvc, breakPoint);
        int ToTLayer;
        // full plane ToT (debugging)
        sList->getToT(&ToTLayer, theTower, bilayer, view, m_ttotSvc);

        Event::TkrDigi* pDigi = new Event::TkrDigi(bilayer, axis, tower, ToT);
        nStrips = 0;

        log << MSG::DEBUG;
        if (log.isActive()) {
            log << "tower " << tower.id() << " bilayer " << bilayer
                << " view " << axis << " ToT " << ToT[0] << " " << ToT[1]
                << " ( " << ToTLayer << " )" ;
        }
        log << endreq;

        // now loop over contained list of strips
        SiStripList::iterator itStrip=sList->begin();
        for (itStrip=sList->begin(); itStrip!=sList->end(); ++itStrip ) {
            if (itStrip->badStrip()) continue;
            const int stripId = itStrip->index();
            nStrip[view]++;
            nStrips++;

            // add the strip to the correct controller
            if ( stripId <= breakPoint )
                pDigi->addC0Hit(stripId);
            else
                pDigi->addC1Hit(stripId);
            log << MSG::DEBUG;
            if (log.isActive()) 
                log << "    strip " << itStrip->index()
                << " energy " << itStrip->energy()
                << " noise " << itStrip->noise();
            log << endreq;

            const SiStripList::hitList& hits = itStrip->getHits();

            // save the hit here
            Event::McTkrStrip* pStrip =
                new Event::McTkrStrip(volId, stripId,
                itStrip->energy(),
                itStrip->noise(), hits);
            strips->push_back(pStrip);

            // and add the relation
            std::ostringstream ost;
            ost << std::setw(4) << stripId;
            Event::McTkrStrip::hitList::const_iterator itHit=pStrip->begin();
            for (; itHit!=pStrip->end(); ++itHit ) {
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
        pTkrDigi->push_back(pDigi);
        nDigi[view]++;
    }

    // sort by volume id
    std::sort(pTkrDigi->begin(), pTkrDigi->end(), Event::TkrDigi::digiLess());

    // Cable truncation now handled in TkrDigiTruncationTool

    log << MSG::DEBUG;
    if (log.isActive()) {
        log << " X digis/strips: " << nDigi[0] << " " << nStrip[0]
        << " Y digis/strips: " << nDigi[1] << " " << nStrip[1];
    }
    log << endreq;

    return sc;
}
