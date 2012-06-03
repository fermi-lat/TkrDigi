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
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralHitRemovalTool.cxx,v 1.8 2012/04/25 04:55:02 heather Exp $
*/

#include "GeneralHitRemovalTool.h"
#include "TkrUtil/ITkrGeometrySvc.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/Digi/TkrDigi.h"

#include "../TkrVolumeIdentifier.h"
#include "../SiStripList.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include <string>


//static const ToolFactory<GeneralHitRemovalTool>    s_factory;
//const IToolFactory& GeneralHitRemovalToolFactory = s_factory;
DECLARE_TOOL_FACTORY(GeneralHitRemovalTool);


GeneralHitRemovalTool::GeneralHitRemovalTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent) :
AlgTool(type, name, parent) {
    // Declare the additional interface
    declareInterface<IHitRemovalTool>(this);

    // Declaring the properties.
    declareProperty("killFailed",    m_killFailed    = true);
    declareProperty("killBadStrips", m_killBadStrips = true);
    declareProperty("trimDigis"    , m_trimDigis     = false);
    declareProperty("trimCount"    , m_trimCount     = 14);
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

    m_splitsSvc    = m_tkrGeom->getTkrSplitsSvc();
    m_failSvc      = m_tkrGeom->getTkrFailureModeSvc();
    m_badStripsSvc = m_tkrGeom->getTkrBadStripsSvc();

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

    // full treatment...
    m_doFailed = m_doBad = m_doTrunc = true;
    killBadHitsLoop(siPlaneMap);
    //doRCBufferLoop(siPlaneMap);

    // next, the cable buffers
    bool towersOutOfOrder=false, planesOutOfOrder=false;
    //doCableBufferLoop(siPlaneMap, 
    //    towersOutOfOrder, planesOutOfOrder);

    //if(towersOutOfOrder || planesOutOfOrder) {
    //    log << MSG::INFO << "There is a problem with the ordering of the SiStrips... " 
    //       << " this should *not* happen!" << endreq;
    //    return StatusCode::FAILURE;
    //}
    return sc;
}

StatusCode GeneralHitRemovalTool::truncateDigis()
{
    // Purpose and Method:  truncate hits in merged digis
    // Inputs: none
    // Outputs: StatusCode
    // TDS Inputs: None
    // TDS Outputs: Modified TkrDigiCol
    // Dependencies: None
    // Restrictions and Caveats: None

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());

    // get the digis
    SmartDataPtr<Event::TkrDigiCol> tkrDigiCol(m_edSvc,
        EventModel::Digi::TkrDigiCol);
    

    // make a siPlaneMap
    SiPlaneMapContainer::SiPlaneMap siPlaneMap;

    // this map connects an SiStripList to a digi,
    //  so I don't have to hunt for it.
    typedef std::map< SiStripList*, Event::TkrDigi*> StripDigiMap;
    StripDigiMap stripDigiMap;

    // make and fill siStripLists from the Digi
    Event::TkrDigiCol::iterator it = tkrDigiCol->begin();
    int count = 0;
    for (;it!=tkrDigiCol->end(); ++it) {
        Event::TkrDigi* digi = *(it);

        // make up the VolumeId
        idents::TowerId id(digi->getTower());
        int layer = digi->getBilayer();
        int view  = digi->getView();
        //std::cout << "Digi " << count << " " << digi->getTower().id() << " " << layer << " "
        //    << view << std::endl;
        count++;
        int tray, botTop;
        m_tkrGeom->layerToTray(layer, view, tray, botTop);

        idents::VolumeIdentifier volId;
        volId.append(0);
        volId.append(id.iy());
        volId.append(id.ix());
        volId.append(1);
        volId.append(tray);
        volId.append(view);
        volId.append(botTop);

        Event::McPositionHit* ptr = 0;
        SiStripList* siList = new SiStripList();

        int i;
        for (i=0;i<digi->getNumHits(); ++i) {
            int strip = digi->getHit(i);
            siList->addStrip(strip, 0.0, ptr);
        }
        
        siPlaneMap[volId] = siList;
        // save the pointer to the digi for this stripList
        stripDigiMap[siList] = digi;
    }    

    // do the truncation loops
    //skip the steps for failed and bad, don't need them here
    m_doFailed = m_doBad = false;
    m_doTrunc = true;

    int nKilledBadHits = 0, nRemovedRCHits = 0, nRemovedCCHits = 0;

    if(m_doFailed || m_doBad) nKilledBadHits = killBadHitsLoop(siPlaneMap);
    if(m_doTrunc) nRemovedRCHits = doRCBufferLoop(siPlaneMap);
    bool towersOutOfOrder = false, planesOutOfOrder = false;
    if(m_doTrunc) nRemovedCCHits = doCableBufferLoop(siPlaneMap, 
        towersOutOfOrder, planesOutOfOrder);

    if(towersOutOfOrder || planesOutOfOrder) {
        log << MSG::INFO << "There is a problem with the ordering of the SiStrips... " 
            << " this should *not* happen!" << endreq;
        return StatusCode::FAILURE;
    }

    // remove digis if necessary, stop when it's all done!
    int nRemovedStrips = nRemovedRCHits+nRemovedCCHits;
    if(nRemovedStrips>0) {
        int stripCount = 0;
        SiPlaneMapContainer::SiPlaneMap::iterator itMap = siPlaneMap.begin();
        for(;itMap!=siPlaneMap.end()&&stripCount<nRemovedStrips;++itMap) {
            SiStripList* siList = itMap->second;
            Event::TkrDigi* thisDigi = stripDigiMap[siList];

            SiStripList::iterator itList = siList->begin();
            for(; itList!=siList->end(); ++itList) {
                if((itList->stripStatus()&SiStripList::NODATA)!=0) {
                    stripCount++;
                    int strip = itList->index();

                    //Here we remove the strip from the Digi
                    thisDigi->removeHit(strip);                    
                }
            }
            // if the HitList is empty, delete the digi
            // this can happen after hit truncation along a cable
            // for maxHit==14, this won't happen, because by construction
            // the cable buffer never fills (well, hardly ever!)
            if(thisDigi->getNumHits()==0) {
                if(debug) log << MSG::DEBUG << "digi removed: tower " << thisDigi->getTower().id() <<
                    " layer " << thisDigi->getBilayer()  << " view " << thisDigi->getView()
                    << endreq;
                delete thisDigi;
            }
        }
    }

    // make sure the stripDigiMap goes away!
    StripDigiMap::iterator mapIter = stripDigiMap.begin();      
    for(;mapIter!=stripDigiMap.end(); ++mapIter) {
        SiStripList* stripList = mapIter->first;
        stripList->clear();
        delete stripList;
    }
    stripDigiMap.clear();

    return sc;
}

int GeneralHitRemovalTool::killBadHitsLoop(
    SiPlaneMapContainer::SiPlaneMap& siPlaneMap)
{

    int removed = 0;
    SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();
    for ( ; itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId towerId     = volId.getTower();
        const int tower = towerId.id();
        const int bilayer  = volId.getLayer();
        const int view     = volId.getView();
        const idents::GlastAxis::axis axis = volId.getAxis();

        SiStripList::iterator itStrip; 
        if(m_killFailed&&m_doFailed) {
            if ( m_killFailed && m_failSvc && !m_failSvc->empty()) { 
                if( m_failSvc->isFailed(tower, bilayer, view) ) {
                    for (itStrip=sList->begin() ;itStrip!=sList->end(); ++itStrip ) {
                        itStrip->setStripStatus(SiStripList::FAILEDPLANE);
                        removed++;
                    }
                    continue;
                }
            }
        }
        // Loop over contained list of strips to remove the bad strips.
        if(m_killBadStrips&&m_doBad) {
            for (itStrip=sList->begin();itStrip!=sList->end(); ++itStrip ) {
                const int stripId = itStrip->index();
                if ( m_killBadStrips && m_badStripsSvc && !m_badStripsSvc->empty()) {
                    if ( m_badStripsSvc->isBadStrip(tower, bilayer, axis, stripId) ) {
                        itStrip->setStripStatus(SiStripList::BADSTRIP);
                        removed++;
                    }
                }
            }
        }
    }
    return removed;
}
int GeneralHitRemovalTool::doRCBufferLoop(
    SiPlaneMapContainer::SiPlaneMap& siPlaneMap)
{
    int removed = 0;
    SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();
    for ( ; itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId towerId     = volId.getTower();
        const int tower = towerId.id();
        const int bilayer  = volId.getLayer();
        const int view     = volId.getView();
        const idents::GlastAxis::axis axis = volId.getAxis();
        SiStripList::iterator itStrip; 
        // Truncate the controller buffers
        // The lost strips are the ones furthest away from the controller 
        int maxLow, maxHigh;
        if(m_trimDigis) {
            maxLow  = m_trimCount;
            maxHigh = m_trimCount;
        } else {
            maxLow  = m_splitsSvc->getMaxStrips(tower, bilayer, view, 0);
            maxHigh = m_splitsSvc->getMaxStrips(tower, bilayer, view, 1);
        }
        int breakpoint = m_splitsSvc->getSplitPoint(tower, bilayer, view);

        // quick test
        int test    = std::min(maxLow, maxHigh);
        if (sList->size()>test&&m_doTrunc) {
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
                    removed++;
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
                    removed++;
                    if(debug) {
                        if (first) std::cout << "RCBuffer: " ;
                        std::cout << itStrip->index() << " " ;
                    }
                }
            }
            if (!first) std::cout << std::endl;
        }
    }
    return removed;
}

int GeneralHitRemovalTool::doCableBufferLoop(
    SiPlaneMapContainer::SiPlaneMap& siPlaneMap, 
    bool& towersOutOfOrder, bool& planesOutOfOrder)
{
    towersOutOfOrder = false;
    planesOutOfOrder = false;
    int removed = 0;
    int tower0 = -1;
    int cableBufferSize = m_splitsSvc->getCableBufferSize();
    SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();
    for (itMap=siPlaneMap.begin() ; itMap!=siPlaneMap.end(); ++itMap ) {
        SiStripList* sList = itMap->second;
        const TkrVolumeIdentifier volId = itMap->first;  
        const idents::TowerId towerId   = volId.getTower();
        const int tower = towerId.id();
        int cableCount[8];  
        if(tower!=tower0) {
            // clear the counters for a new tower
            if(tower<tower0) {
                towersOutOfOrder = true;
            }
            tower0 = tower;
            int i;
            for(i=0;i<8;++i) { cableCount[i] = 0;}
        }
        const int bilayer = volId.getLayer();
        const int view    = volId.getView();

        int breakpoint = m_splitsSvc->getSplitPoint(tower, bilayer, view);
        SiStripList::iterator itStrip=sList->begin();
        int strip0 = -1;
        bool first = true;
        for ( ;itStrip!=sList->end(); ++itStrip ) {
            const int stripId = itStrip->index();
            if(stripId< strip0) {
                planesOutOfOrder = true;
            }
            strip0 = stripId;
            int end = (stripId>breakpoint ? 1 : 0);
            int index = m_splitsSvc->getCableIndex(bilayer, view, end);
            if(!itStrip->badStrip()) cableCount[index]++;
            if(cableCount[index]>cableBufferSize) {
                itStrip->setStripStatus(SiStripList::CCBUFFER);
                removed++;
                if (debug) {
                    if (first) std::cout << "CCBuffer overflow :";
                    first = false;
                    std::cout << itStrip->index() << " ";
                }
            }
        }
        if (!first) std::cout << std::endl;
    }
    return removed;
}
