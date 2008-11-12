/*
 * @file TkrDigiMergeAlg.cxx
 *
 * @brief Calls an user-chosen tool to merge TkrDigis from input overlay files
 *
 * @author Tracy Usher
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiMergeAlg.cxx,v 1.1 2008/10/15 15:35:06 usher Exp $
 */


#include "../IHitToDigiTool.h"

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/Digi/TkrDigi.h"
#include "LdfEvent/Gem.h"

#include "TkrUtil/ITkrGeometrySvc.h"
#include "TkrUtil/ITkrMakeClustersTool.h"
#include "TkrUtil/ITkrGhostTool.h"

#include <map>

class TkrDigiMergeAlg : public Algorithm {

 public:

    TkrDigiMergeAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /// Type of tool to run
    std::string         m_type;
    /// Pointer to the tracker geometry service
    ITkrGeometrySvc*    m_tkrGeom;
    /// Pointer to the tracker splits service
    ITkrSplitsSvc*      m_tspSvc;
    /// ptr to ghost tool
    ITkrGhostTool*      m_ghostTool;
    /// ptr to cluster tool
    ITkrMakeClustersTool* m_clustersTool;

};

// Used by Gaudi for identifying this algorithm
static const AlgFactory<TkrDigiMergeAlg>    Factory;
const IAlgFactory& TkrDigiMergeAlgFactory = Factory;

TkrDigiMergeAlg::TkrDigiMergeAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type="General");
}


StatusCode TkrDigiMergeAlg::initialize() {
    // Purpose and Method: initializes TkrDigiMergeAlg
    // Inputs: none
    // Outputs: a status code
    // Dependencies: value of m_type determining the type of tool to run
    // Restrictions and Caveats: none

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;

    if ( setProperties().isFailure() ) {
        log << MSG::ERROR << "setProperties() failed" << endreq;
        return StatusCode::FAILURE;
    }

    // Get the Tkr Geometry service 
    sc = service("TkrGeometrySvc", m_tkrGeom, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
        return sc;
    }

    // Get the splits service 
    m_tspSvc = m_tkrGeom->getTkrSplitsSvc();
    if ( !m_tspSvc ) {
        log << MSG::ERROR
            << "Couldn't set up TkrSplitsSvc!" << std::endl;
        return StatusCode::FAILURE;
    }

    m_ghostTool = 0;
    sc = toolSvc()->retrieveTool("TkrGhostTool",m_ghostTool) ;
    if (sc.isFailure()) {
        log<<MSG::WARNING << "Failed to retrieve ghost tool" << endreq ;
        return StatusCode::FAILURE ;
    }

    m_clustersTool = 0;
    sc = toolSvc()->retrieveTool("TkrMakeClustersTool",m_clustersTool) ;
    if (sc.isFailure()) {
        log<<MSG::WARNING << "Failed to retrieve ghost tool" << endreq ;
        return StatusCode::FAILURE ;
    }

    return sc;
}


StatusCode TkrDigiMergeAlg::execute() {
    // Purpose and Method: execution method (called once for every event)
    //                     Doesn't do anything but calls the chosen tool.
    // Inputs: none
    // Outputs: a status code
    // Dependencies: none
    // Restrictions and Caveats: none

    StatusCode sc = StatusCode::SUCCESS; 
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "execute" << endreq;

    // First, recover any overlay digis, to see if we have anything to do
    SmartDataPtr<Event::TkrDigiCol> overlayDigiCol(eventSvc(), EventModel::Overlay::TkrDigiCol);
    if(!overlayDigiCol) return StatusCode::SUCCESS;

    // Now recover the digis for this event
    SmartDataPtr<Event::TkrDigiCol> tkrDigiCol(eventSvc(), EventModel::Digi::TkrDigiCol);

    // Create a map of the simulation digis, indexing by identifier
    std::map<int, Event::TkrDigi*> idToDigiMap;

    // do the TkrVector shenanigans

    // make the tkrVector
    unsigned short towerBits = 0;
    m_ghostTool->calculateTkrVector(tkrDigiCol, towerBits);

    // get the tkrVector of the overlay
    bool isOverlay;
    unsigned short tkrVector;
    SmartDataPtr<LdfEvent::Gem> gemOverlay(eventSvc(), "Overlay/Gem");
    if(overlayDigiCol && gemOverlay!=0) {
        isOverlay = true;
        tkrVector = gemOverlay->tkrVector();
    }

    // OR the two sources
    tkrVector |= towerBits;

    DataObject* pNode = 0;
    LdfEvent::Gem* gem = new LdfEvent::Gem();

    sc = eventSvc()->retrieveObject("/Event/Gem", pNode);
    if ( sc.isFailure() ) {
        sc = eventSvc()->registerObject("/Event/Gem", gem);
        if( sc.isFailure() ) {
            log << MSG::ERROR << "could not register /Event/Gem" << endreq;
            return sc;
        }
    }

    // store the result in the event
    gem->setTkrVector( tkrVector );

    for(Event::TkrDigiCol::iterator tkrIter = tkrDigiCol->begin(); tkrIter != tkrDigiCol->end(); tkrIter++)
    {
        Event::TkrDigi* tkrDigi = *tkrIter;
        int             digiId  = (*tkrDigi);

        // Add this digi to our map
        idToDigiMap[digiId] = tkrDigi;
    }

    // Now loop through the overlay digis to merge into our simulated ones
    for(Event::TkrDigiCol::iterator overIter = overlayDigiCol->begin(); overIter != overlayDigiCol->end(); overIter++)
    {
        Event::TkrDigi* overDigi = *overIter;
        int             overId   = (*overDigi);

        std::map<int,Event::TkrDigi*>::iterator tkrIter = idToDigiMap.find(overId);

        // If this digi is not already in the TkrDigiCol then no merging needed, just add it
        if (tkrIter == idToDigiMap.end())
        {
            Event::TkrDigi* digi = new Event::TkrDigi();
            *digi = *overDigi;
            digi->setParent(0);

            tkrDigiCol->push_back(digi);
        }
        // Otherwise, we need to merge the digi
        else
        {
            // Pointer to the TDS digi
            Event::TkrDigi* tkrDigi = tkrIter->second;

            // Retrieve tower, bilayer and view information
            idents::TowerId         towerId = overDigi->getTower();
            int                     biLayer = overDigi->getBilayer();
            idents::GlastAxis::axis view    = overDigi->getView();

            // Get the break point for this tower, bi-layer and view
            int breakPoint = m_tspSvc->getSplitPoint(towerId.id(), biLayer, view);
            int lastStrip0 = tkrDigi->getLastController0Strip();

            // Recover the ToT (note that "addHit" will take care of ToT for us)
            int tot0 = tkrDigi->getToT(0) > overDigi->getToT(0) ? tkrDigi->getToT(0) : overDigi->getToT(0);
            int tot1 = tkrDigi->getToT(1) > overDigi->getToT(1) ? tkrDigi->getToT(1) : overDigi->getToT(1);

            // Merge hit strips, loop over hits in the overlay digi
            for(std::vector<int>::const_iterator overStripIter = overDigi->begin(); overStripIter != overDigi->end(); overStripIter++)
            {
                int overStripId = *overStripIter;

                // Check the last strip stuff
                if (overStripId < breakPoint && overStripId > lastStrip0) lastStrip0 = overStripId;

                // Go through following contortions to find correct slot to insert strip id
                bool notFound = true;
                for(std::vector<int>::iterator stripIter = tkrDigi->begin(); stripIter != tkrDigi->end(); stripIter++)
                {
                    if (overStripId >  *stripIter) continue;
                    
                    notFound = false;

                    if (overStripId == *stripIter) break;
                    tkrDigi->insert(stripIter, overStripId);
                    break;
                }

                if (notFound) tkrDigi->push_back(overStripId);
            }

            // Update the ToT and lastStrip info
            tkrDigi->setToT(0,tot0);
            tkrDigi->setToT(1,tot1);
            tkrDigi->setLastController0Strip(lastStrip0);
        }
    }

    // sort by volume id
    std::sort(tkrDigiCol->begin(), tkrDigiCol->end(), Event::TkrDigi::digiLess());

    return sc;
}


StatusCode TkrDigiMergeAlg::finalize() {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
