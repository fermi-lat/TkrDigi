/*
* @file GeneralHitRemovalTool.cxx
*
* @brief Truncates strips in the StripLists.
* due to:
*    failed planes
*    bad strips
*    finite controller and cable buffers
*
* @author Leon Rochester
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralHitRemovalTool.cxx,v 1.3 2005/08/25 17:23:55 lsrea Exp $
*/

#include "GeneralHitRemovalTool.h"
#include "TkrUtil/ITkrGeometrySvc.h"

#include "../SiPlaneMapContainer.h"
#include "../TkrVolumeIdentifier.h"
#include "../SiStripList.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include <string>


static const ToolFactory<GeneralHitRemovalTool>    s_factory;
const IToolFactory& GeneralHitRemovalToolFactory = s_factory;


GeneralHitRemovalTool::GeneralHitRemovalTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent) :
AlgTool(type, name, parent) {
    // Declare the additional interface
    declareInterface<IHitRemovalTool>(this);

    // Declaring the properties.
    declareProperty("killFailed",    m_killFailed    = true);
    declareProperty("killBadStrips", m_killBadStrips = true);
}

namespace {
    bool debug = false;
}

StatusCode GeneralHitRemovalTool::initialize() {
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

    // Get TkrGeometrySvc 
    sc = service("TkrGeometrySvc", m_tkrGeom);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up GlastDetSvc!" << endreq;
        return sc;
    }

    IService* iService = 0;
    sc = serviceLocator()->service("EventDataSvc", iService, true );
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc!" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);

    return sc;
}


StatusCode GeneralHitRemovalTool::execute() {
    // Purpose and Method:  truncate hits
    // Inputs: StripLists
    // Outputs: hits flagged for removal in the StripLists
    // TDS Inputs: /Event/tmp/siStripList
    // TDS Outputs: /Event/tmp/siSripList
    // Dependencies: None
    // Restrictions and Caveats: The TDS output is a hard-coded string

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());


    // retrieve the pointer to the SiPlaneMapContainer from TDS
    SmartDataPtr<SiPlaneMapContainer> pObject(m_edSvc,
        "/Event/tmp/siPlaneMapContainer");
    if ( !pObject ) {
        log << MSG::ERROR
            << "could not retrieve /Event/tmp/siPlaneMapContainer" << endreq;
        return sc;
    }

    SiPlaneMapContainer::SiPlaneMap& siPlaneMap = pObject->getSiPlaneMap();
    SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();

    //Start with the failure modes and bad strips
    ITkrFailureModeSvc* failSvc      = m_tkrGeom->getTkrFailureModeSvc();
    ITkrBadStripsSvc*   badStripsSvc = m_tkrGeom->getTkrBadStripsSvc();
    ITkrSplitsSvc*      splitsSvc    = m_tkrGeom->getTkrSplitsSvc();

    for ( ; itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId towerId     = volId.getTower();
        const int tower = towerId.id();
        //const int tray  = volId.getTray();
        //const int face  = volId.getBotTop();
        const int bilayer  = volId.getLayer();
        const int view     = volId.getView();
        const idents::GlastAxis::axis axis = volId.getAxis();

        SiStripList::iterator itStrip; 
        if(m_killFailed) {
            if ( m_killFailed && failSvc && !failSvc->empty()) { 
                if( failSvc->isFailed(tower, bilayer, view) ) {
                    for (itStrip=sList->begin() ;itStrip!=sList->end(); ++itStrip ) {
                        itStrip->setStripStatus(SiStripList::FAILEDPLANE);
                    }
                    continue;
                }
            }
        }
        // Loop over contained list of strips to remove the bad strips.
        if(m_killBadStrips) {
            for (itStrip=sList->begin();itStrip!=sList->end(); ++itStrip ) {
                const int stripId = itStrip->index();
                if ( m_killBadStrips && badStripsSvc && !badStripsSvc->empty()) {
                    if ( badStripsSvc->isBadStrip(tower, bilayer, axis, stripId) ) {
                        itStrip->setStripStatus(SiStripList::BADSTRIP);
                    }
                }
            }
        }
        // Truncate the controller buffers
        // The lost strips are the ones furthest away from the controller 
        int maxLow  = splitsSvc->getMaxStrips(tower, bilayer, view, 0);
        int maxHigh = splitsSvc->getMaxStrips(tower, bilayer, view, 1);
        int breakpoint = splitsSvc->getSplitPoint(tower, bilayer, view);

        // quick test
        int test    = std::min(maxLow, maxHigh);
        if (sList->size()>test) {
            int liveCount  = 0;
            // for the low end, we pass maxLow strips, and kill the rest
            bool first = true;
            for (itStrip=sList->begin();itStrip!=sList->end(); ++itStrip ) {
                const int stripId = itStrip->index();
                if (stripId>breakpoint) break;
                if (itStrip->badStrip()) continue;
                liveCount++;
                if (liveCount>maxLow) {
                    itStrip->setStripStatus(SiStripList::RCBUFFER);
                    if(debug) {
                        if (first) std::cout << "RCBuffer overflow: " ;
                        first = false;
                        std::cout << itStrip->index() << " " ;
                    }
                }
            }
            // same for the high end, but going backwards
            liveCount = 0;
            SiStripList::reverse_iterator itRev;
            for (itRev=sList->rbegin();itRev!=sList->rend(); ++itRev ) {
                const int stripId = itRev->index();
                if (stripId<=breakpoint) break;
                if(itRev->badStrip()) continue;
                liveCount++;
                if (liveCount>maxHigh) {
                    itRev->setStripStatus(SiStripList::RCBUFFER);
                    if(debug) {
                        if (first) std::cout << "RCBuffer: " ;
                        std::cout << itStrip->index() << " " ;
                    }
                }
            }
            if (!first) std::cout << std::endl;
        }
    }

    // now, after everything else, the cable buffers

    bool towersOutOfOrder = false;
    bool planesOutOfOrder = false;
    int tower0 = -1;
    int cableBufferSize = splitsSvc->getCableBufferSize();
    for (itMap=siPlaneMap.begin() ; itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId towerId   = volId.getTower();
        const int tower = towerId.id();
        int cableCount[8];  
        if(tower!=tower0) {
            // clear the counters for a new tower
            if(tower<tower0) towersOutOfOrder = true;
            tower0 = tower;
            int i;
            for(i=0;i<8;++i) { cableCount[i] = 0;}
        }
        const int bilayer = volId.getLayer();
        const int view    = volId.getView();
        //const int tray    = volId.getTray();
        //const int face    = volId.getBotTop();

        int breakpoint = splitsSvc->getSplitPoint(tower, bilayer, view);
        SiStripList::iterator itStrip=sList->begin();
        int strip0 = -1;
        bool first = true;
        for ( ;itStrip!=sList->end(); ++itStrip ) {
            const int stripId = itStrip->index();
            if(stripId< strip0) planesOutOfOrder = true;
            strip0 = stripId;
            int end = (stripId>breakpoint ? 1 : 0);
            int index = splitsSvc->getCableIndex(bilayer, view, end);
            if(!itStrip->badStrip()) cableCount[index]++;
            if(cableCount[index]>cableBufferSize) {
                itStrip->setStripStatus(SiStripList::CCBUFFER);
                if (debug) {
                    if (first) std::cout << "CCBuffer overflow :";
                    first = false;
                    std::cout << itStrip->index() << " ";
                }
            }
        }
        if (!first) std::cout << std::endl;
    }
    if(towersOutOfOrder || planesOutOfOrder) {
        log << MSG::INFO << "There is a problem with the ordering of the SiStrips... " 
            << " this should *not* happen!" << endreq;
        return StatusCode::FAILURE;
    }
    return sc;
}
