/*
 * @file BariMcToHitTool.cxx
 *
 * @brief Converts MC hits into tkr hits, using the Bari method.
 * Extracted from GlastDigi/v4r6/src/Tkr/TkrBariDigiAlg.cxx
 *
 * @authors Nico Giglietto, Monica Brigida, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/BariMcToHitTool.cxx,v 1.2 2004/03/09 20:06:30 lsrea Exp $
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

#include "xml/IFile.h"

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
    log << MSG::INFO << "initialize " << endreq;

    // Set a default current file
    declareProperty("CurrentsFile",
            m_CurrentsFile="$(TKRDIGIROOT)/src/Bari/correnti");

    // Do the currents file (once) - LSR
    // OpenCurrent now returns a status code - LSR    
    xml::IFile::extractEnvVar(&m_CurrentsFile);
    // trying to eliminate memory leak candidates
    //    m_openCurr = new InitCurrent();
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

    //Look to see if the McPositionHitCol object is in the TDS
    SmartDataPtr<Event::McPositionHitCol>
        hits(m_edSvc, EventModel::MC::McPositionHitCol);
    if ( !hits ) { 
        log << MSG::WARNING << "could not find "
            << EventModel::MC::McPositionHitCol << endreq;
        return  sc;
    }

    //    CurrOr* CurrentOr = new CurrOr();
    CurrOr CurrentOr;
    TkrDigitizer Digit;
    Digit.Clean();
    int kk = 0;

    static const double ladder_pitch = SiStripList::die_width()
        + SiStripList::ladder_gap();
    static const double ssd_pitch    = SiStripList::die_width()
        + SiStripList::ssd_gap();
    static const double waferOffset  = 0.5*(SiStripList::n_si_dies() - 1);
            
    // start hits loop 
    for ( Event::McPositionHitCol::iterator it=hits->begin(); it!=hits->end();
          it++ ) {   
        Event::McPositionHit* pHit = *it;
        const TkrVolumeIdentifier volId = pHit->volumeID();
            
        // this is to remove ACD hits, which are now also McPositionHits
        if ( volId.size() != 9 )
            continue;
        if ( !volId.isTowerTkr() )
            continue;
            
        const int tower   = volId.getTower().id();
        const int bilayer = volId.getLayer();
        const int view    = volId.getView();
        const int ladder  = volId.getLadder(); // needed for the offset
        const int wafer   = volId.getWafer(); // needed for the offset
            
        // Here we shift the coordinate from the wafer to the plane
        // this assumes that the number of ladders equals the number of
        // wafers/ladder.  This is not true for the BFEM/BTEM!
        const HepVector3D offset((ladder-waferOffset)*ladder_pitch,
                           (wafer-waferOffset)*ssd_pitch, 0);
        HepPoint3D entry(pHit->entryPoint()+offset);
        HepPoint3D exit (pHit->exitPoint() +offset);
            
        const double energy = pHit->depositedEnergy();

        log << MSG::DEBUG;
		if (log.isActive() ) 
        log << "Hit " << ++kk << " tower " << tower
            << " layer " << bilayer << " view "  << view
            <<" ene " << energy << endreq
            << "      "
            << " entry(" << entry.x()<<", "<<entry.y()<<", "<<entry.z() 
            << ") exit (" << exit.x()<<", "<<exit.y()<<", "<<exit.z() << ")";
        log << endreq;
            
        // set variables
        // getPlaneId() returns a volId with ladder and wafer info stripped
        Digit.set(energy, entry, exit, volId.getPlaneId(), pHit);
        // analog section
        Digit.setDigit(&m_openCurr);
        Digit.clusterize(&CurrentOr);
    } // end of loop over hits

    // digital section
    const TotOr* const DigiOr = Digit.digitize(CurrentOr);

    log << MSG::DEBUG;
    if (log.isActive()) 
        log << " END DIGITAL SECTION: " << kk << " MC hits found; "
        << DigiOr->size() << " digits stored in the TDS";
    log << endreq;

    // Take care of insuring that the data area has been created
    DataObject* pNode = 0;
    sc = m_edSvc->retrieveObject("/Event/tmp", pNode);
    if ( sc.isFailure() ) {
        sc = m_edSvc->registerObject("/Event/tmp", new DataObject);
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

