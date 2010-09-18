/*
 * @file SimpleMcToHitTool.cxx
 *
 * @brief Converts MC hits into tkr hits, using the Simple method.
 * Extracted from TkrDigi/v1r11p2/src/TkrSimpleDigiAlg.cxx
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Simple/SimpleMcToHitTool.cxx,v 1.7 2007/08/08 20:50:52 lsrea Exp $
 */

#include "SimpleMcToHitTool.h"

#include "../SiPlaneMapContainer.h"
#include "../TkrVolumeIdentifier.h"

// Glast specific includes
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/MonteCarlo/McPositionHit.h"
#include "Event/MonteCarlo/McParticle.h"

// Gaudi system includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"


//static const ToolFactory<SimpleMcToHitTool>    s_factory;
//const IToolFactory& SimpleMcToHitToolFactory = s_factory;
DECLARE_TOOL_FACTORY(SimpleMcToHitTool);


SimpleMcToHitTool::SimpleMcToHitTool(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent) :
    AlgTool(type, name, parent) {
    // Declare the additional interface
    declareInterface<IMcToHitTool>(this);

    declareProperty("test", m_test = false);
    declareProperty("alignmentMode", m_alignmentMode=0);
}

StatusCode SimpleMcToHitTool::initialize() {
    // Purpose and Method: initializes the tool
    // Inputs: None
    // Outputs: a status code
    // Dependencies: several services
    // Restrictions and Caveats: None

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize " << endreq;

    setProperties();

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
        << "ssdgap "        << SiStripList::ssd_gap() 
        << " laddergap "    << SiStripList::ladder_gap()
        << " strips/wafer " << SiStripList::strips_per_die() 
        << " #ladders "     << SiStripList::n_si_dies() 
        << " waferside "    << SiStripList::die_width() 
        << " deadgap "      << SiStripList::guard_ring() 
        << endreq;

    // Get the Tkr Geometry service 
    sc = service("TkrGeometrySvc", m_tkrGeom, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
        return sc;
    }

    // Get the alignment service 
    m_taSvc = m_tkrGeom->getTkrAlignmentSvc();
    if ( !m_taSvc ) {
        log << MSG::WARNING << "Couldn't set up TkrAlignmentSvc!"
            << std::endl << "Will assume it is not required!" << endreq;
    }

    IService* iService = 0;
    sc = serviceLocator()->service("EventDataSvc", iService, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc !" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);

    return sc;
}


StatusCode SimpleMcToHitTool::execute() {
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

    // get the "event direction"
    HepVector3D eventDir = HepVector3D(0., 0., -1.);
    SmartDataPtr<Event::McParticleCol> mcParts(m_edSvc, EventModel::MC::McParticleCol);
    if (mcParts) { 
        Event::McParticleCol::const_iterator ipart=mcParts->begin();
        const Event::McParticle* firstPart = *ipart;
        const CLHEP::HepLorentzVector p = firstPart->initialFourMomentum();
        eventDir = HepVector3D(p.x(), p.y(), p.z()).unit();
    }

    // Look to see if the McPositionHitCol object is in the TDS
    SmartDataPtr<Event::McPositionHitCol>
        mcHits(m_edSvc, EventModel::MC::McPositionHitCol);
	if( !mcHits)  { 
		log << MSG::DEBUG;
		if (log.isActive() )
			log << "could not find \""<< "EventModel::MC::McPositionHitCol" 
				<<"\". ";
        log << endreq;
	}
    
    // Fill the map
    SiPlaneMapContainer::SiPlaneMap siPlaneMap = createSiHits(mcHits, eventDir);
    
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

    SiPlaneMapContainer* siPlaneMapCntr = new SiPlaneMapContainer(siPlaneMap);
    sc = m_edSvc->registerObject("/Event/tmp/siPlaneMapContainer",
                                 siPlaneMapCntr);
    if ( sc.isFailure() ) {
        log << MSG::ERROR
            << "could not register /Event/tmp/siPlaneMapContainer" << endreq;
        return sc;
    }

  return sc;
}


SiPlaneMapContainer::SiPlaneMap SimpleMcToHitTool::createSiHits(
    const Event::McPositionHitCol& hits, const HepVector3D& eventDir) {
    // Purpose and Method: reads a list of McPositionHits, and lists them in
    //                     SiStripLists according to their volume identifiers.
    // Inputs: a vector of McPositionHits
    // Outputs: a SiPlaneMap
    // Dependencies: None
    // Restrictions and Caveats: None

    MsgStream   log( msgSvc(), name() );

    SiPlaneMapContainer::SiPlaneMap siPlaneMap;
    
    int nHits = 0;
    if (&hits) nHits = (int) hits.size();
   
    log << MSG::DEBUG;
    if (log.isActive() )
        log << "Number of position hits found = " << nHits;
    log << endreq;

    if (nHits==0) return siPlaneMap;

    // This assumes that the number of ladders equals the number of
    // wafers/ladder.  Not true for the BFEM/BTEM!
    static const double ladder_pitch = SiStripList::die_width()
        + SiStripList::ladder_gap();
    static const double ssd_pitch = SiStripList::die_width()
        + SiStripList::ssd_gap();
    static const double waferOffset = 0.5 * (SiStripList::n_si_dies() - 1);

    for ( Event::McPositionHitCol::const_iterator ihit=hits.begin();
          ihit!=hits.end(); ++ihit ) {
        const Event::McPositionHit* hit = *ihit;
        
        const TkrVolumeIdentifier volId = hit->volumeID();
        
        // check for correct length
        if ( volId.size() != 9 )
            continue;
        // check that it's really a TKR hit (probably overkill)
        if ( !volId.isTowerTkr() )
            continue;
        
        // these are in the wafer frame
        HepPoint3D localEntry(hit->entryPoint());
        HepPoint3D localExit (hit->exitPoint());

        // move hit by alignment constants
        // the wafer constants are applied to the wafer coordinates
        HepVector3D transformAxis(0., 0., -1.0);
        if ( m_taSvc && m_taSvc->alignSim() ) {
            if(m_alignmentMode==0) {
                transformAxis = localExit-localEntry;
            } else {
                transformAxis = eventDir;
            } 
        }

        m_taSvc->moveMCHit(volId, localEntry, localExit, transformAxis);

        const TkrVolumeIdentifier planeId = volId.getPlaneId();
        if( siPlaneMap.find(planeId) == siPlaneMap.end())
            siPlaneMap[planeId]= new SiStripList;

        // now generate the plane coordinates
        // Since we know how the ladders and wafers are laid out
        //    we just translate the wafer coordinates
        const int ladder = volId.getLadder();
        const int wafer  = volId.getWafer();
        const HepVector3D offset((ladder-waferOffset)*ladder_pitch,
            (wafer-waferOffset)*ssd_pitch, 0);

        HepPoint3D planeEntry(localEntry + offset);
        HepPoint3D planeExit (localExit  + offset);

        // the entry into the planeMap is in plane coordinates
        siPlaneMap[planeId]->score(planeEntry, planeExit, hit, m_test);
          }

          return siPlaneMap;
    }
