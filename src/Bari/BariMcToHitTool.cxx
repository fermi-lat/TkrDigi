/*
* @file BariMcToHitTool.cxx
*
* @brief Converts MC hits into tkr hits, using the Bari method.
* Extracted from GlastDigi/v4r6/src/Tkr/TkrBariDigiAlg.cxx
*
* @authors Nico Giglietto, Monica Brigida, Leon Rochester, Michael Kuss
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/BariMcToHitTool.cxx,v 1.14 2007/05/28 10:09:20 ngigliet Exp $
*/

#include "BariMcToHitTool.h"

#include "TkrDigitizer.h"
#include "InitCurrent.h"
#include "TotOr.h"
#include "CurrOr.h"

#include "../SiStripList.h"
#include "../SiPlaneMapContainer.h"
#include "../TkrVolumeIdentifier.h"

// Glast specific includes
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "facilities/Util.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/MonteCarlo/McPositionHit.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"


static const ToolFactory<BariMcToHitTool>    s_factory;
const IToolFactory& BariMcToHitToolFactory = s_factory;


BariMcToHitTool::BariMcToHitTool(const std::string& type,
                                 const std::string& name,
                                 const IInterface* parent) :
AlgTool(type, name, parent)
{
    // Declare the additional interface
    declareInterface<IMcToHitTool>(this);
    declareProperty("Type", m_type);
}

StatusCode BariMcToHitTool::initialize()
{
    // Purpose and Method: initializes the tool
    // Inputs: None
    // Outputs: a status code
    // Dependencies: several services
    // Restrictions and Caveats: None

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << " BariMcToHitTool initialize" << endreq;

    // Set a default current file
    // new file currents is more compact
    declareProperty("CurrentsFile",
		    m_CurrentsFile="$(TKRDIGIDATAPATH)/Bari_charge.txt");
    // Do the currents file (once) - LSR
    facilities::Util::expandEnvVar(&m_CurrentsFile);
    // trying to eliminate memory leak candidates
 
    sc = m_openCurr.OpenCurrent(m_CurrentsFile); 
    if ( sc.isFailure() ) {
        log <<  MSG::ERROR <<  "currents file " <<  m_CurrentsFile
            << "not found, check jobOptions!" << endreq;
        return sc;
    }
    log << MSG::INFO << "Opening currents file " << m_CurrentsFile << endreq;


    IService* iService = 0;
    sc = serviceLocator()->service("EventDataSvc", iService, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc !" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);

    // get the pointer to the detector service once - LSR
    IGlastDetSvc* gsv = 0;
    sc = service("GlastDetSvc", gsv);
    if( sc.isFailure() ) {
        log << MSG::ERROR << "could not find GlastDetSvc !" << endreq;
        return sc;
    }

    sc = service("TkrGeometrySvc", m_tkrGeom, true);
    if( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
        //        log << MSG::ERROR << "could not find TkrGeometrySvc !" << endreq;
        return sc;
    }

    // ToT Svc
    sc = service("TkrToTSvc", pToTSvc, true);
    if( sc.isFailure() ) {
      log << MSG::ERROR << "Couldn't set up TkrToTSvc!" << endreq;
      //        log << MSG::ERROR << "could not find TkrToTSvc !" << endreq;
      return sc;
    }

    // initialize detector interface -- Brigida
    SiStripList::initialize(gsv); 

    log << MSG::INFO
        << "SiStripList init: ssdgap " << SiStripList::ssd_gap()
        << " laddergap " << SiStripList::ladder_gap()
        << " strips/wafer " << SiStripList::strips_per_die()
        << " ndies " << SiStripList::n_si_dies() 
        << " waferside " << SiStripList::die_width()
        << " dead gap " <<  SiStripList::guard_ring()
        << endreq;

    return sc;
}

StatusCode BariMcToHitTool::execute()
{
    // Purpose and Method: Converts McPositionHits into a map of SiStripLists
    // Inputs: None
    // Outputs: a status code
    // TDS Inputs: EventModel::MC::McPositionHitCol
    // TDS Outputs: /Event/tmp/siPlaneMapContainer
    // Dependencies: None
    // Restrictions and Caveats: The TDS output is a hard-coded string

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "execute " << endreq;

    int kk = 0; // to count hits
    CurrOr CurrentOr; /// no!!
    TkrDigitizer Digit;
    Digit.Clean();
  
    //Look to see if the McPositionHitCol object is in the TDS
    SmartDataPtr<Event::McPositionHitCol>
        hits(m_edSvc, EventModel::MC::McPositionHitCol);
    if ( !hits ) { 
        log << MSG::WARNING << "could not find "
            << EventModel::MC::McPositionHitCol << endreq;
    } else {

        // This assumes that the number of ladders equals the number of
        // wafers/ladder.  Not true for the BFEM/BTEM!
        static const double ladder_pitch = SiStripList::die_width()
            + SiStripList::ladder_gap();
        static const double ssd_pitch    = SiStripList::die_width()
            + SiStripList::ssd_gap();
        static const double waferOffset  = 0.5*(SiStripList::n_si_dies() - 1);

        ITkrAlignmentSvc* alsv = m_tkrGeom->getTkrAlignmentSvc();

        // start hits loop 
        Event::McPositionHitCol::iterator it;        
        for (it=hits->begin(); it!=hits->end(); it++ ) {   
            Event::McPositionHit* pHit = *it;
            const TkrVolumeIdentifier volId = pHit->volumeID();

            // this is to remove ACD hits, which are now also McPositionHits
            if ( volId.size() != 9 )
                continue;
            if ( !volId.isTowerTkr() )
                continue;

            // these are in the wafer frame
            HepPoint3D localEntry(pHit->entryPoint());
            HepPoint3D localExit (pHit->exitPoint());

            // move hit by alignment constants
            // the wafer constants are applied to the wafer coordinates
            if ( alsv && alsv->alignSim() )
                alsv->moveMCHit(volId, localEntry, localExit);

            // now generate the plane coordinates
            // Since we know how the ladders and wafers are laid out
            //    we just translate the wafer coordinates

            const int ladder = volId.getLadder();
            const int wafer  = volId.getWafer();

            const HepVector3D offset((ladder-waferOffset)*ladder_pitch,
                (wafer-waferOffset)*ssd_pitch, 0);

            HepPoint3D planeEntry(localEntry + offset);
            HepPoint3D planeExit (localExit  + offset);

            // set variables
            // getPlaneId() returns a volId with ladder and wafer info stripped
            const double energy = pHit->depositedEnergy();

            log << MSG::DEBUG;
	    if (log.isActive() ) {
                const int tower   = volId.getTower().id();
                const int bilayer = volId.getLayer();
                const int view    = volId.getView();
                log << "Hit " << ++kk << " tower " << tower
                    << " layer " << bilayer << " view "  << view
                    <<" ene " << energy << endreq
                    << "      "
                    << " entry(" << planeEntry.x()<<", "<<planeEntry.y()<<", "<<planeEntry.z() 
                    << ") exit (" << planeExit.x()<<", "<<planeExit.y()<<", "<<planeExit.z() << ")";
                log << endreq;
	    }
	    /// bari Digi call -- Monica
            Digit.set(energy, planeEntry, planeExit, volId.getPlaneId(), pHit);
            // analog section
	    Digit.setDigit(&m_openCurr);
	    //   
	    Digit.clusterize(&CurrentOr);
        } // end of loop over hits
    }
    // digital section
    const TotOr* const DigiOr = Digit.digitize(CurrentOr, pToTSvc);

    log << MSG::DEBUG;
    if (log.isActive()) 
        log << " END DIGITAL SECTION: " << kk << " MC hits found; "
        << DigiOr->size() << " digits stored in the TDS";
    log << endreq;

    // Take care of insuring that the data area has been created
    log << MSG::DEBUG << "we should create /Event/tmp" << endreq;
    DataObject* pNode = 0;
    sc = m_edSvc->retrieveObject("/Event/tmp", pNode);
    if ( sc.isFailure() ) {
        sc = m_edSvc->registerObject("/Event/tmp", new DataObject);
        log << MSG::DEBUG << "registering /Event/tmp" << endreq;
        if( sc.isFailure() ) {
            log << MSG::ERROR << "could not register /Event/tmp" << endreq;
            return sc;
        }
    }

    // Store the SiPlaneMap in TDS
    SiPlaneMapContainer::SiPlaneMap siPlaneMap = DigiOr->getList();
    SiPlaneMapContainer* siPlaneMapCntr = new SiPlaneMapContainer(siPlaneMap);
    sc=m_edSvc->registerObject("/Event/tmp/siPlaneMapContainer",siPlaneMapCntr);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not register /Event/tmp/siPlaneMapContainer"
            << endreq;
        return sc;
    }

    return sc;
}
