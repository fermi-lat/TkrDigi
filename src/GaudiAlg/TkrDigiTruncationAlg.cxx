/*
 * @file TkrDigiTruncationAlg.cxx
 *
 * @brief Calls an user-chosen tool to add noisy strips to the SiStripList.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiMergeTruncationAlg.cxx,v 1.1 2008/12/08 01:55:04 lsrea Exp $
 */

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"

#include "Event/TopLevel/EventModel.h"
#include "../IHitRemovalTool.h"
#include "TkrUtil/ITkrHitTruncationTool.h"

#include <string>


class TkrDigiTruncationAlg : public Algorithm {

 public:

    TkrDigiTruncationAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /// Type of tool to run
    std::string m_type;
    /// Pointer to the tool
    IHitRemovalTool* m_tool;
    ITkrHitTruncationTool* m_trunc;

    bool m_trimDigis;
    int  m_trimCount;
    Event::TkrDigiCol* m_tkrDigiCol;
};


// Used by Gaudi for identifying this algorithm
static const AlgFactory<TkrDigiTruncationAlg>    Factory;
const IAlgFactory& TkrDigiTruncationAlgFactory = Factory;


TkrDigiTruncationAlg::TkrDigiTruncationAlg(const std::string& name,
                                 ISvcLocator* pSvcLocator)
    : Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type="General");
    declareProperty("trimDigis", m_trimDigis=false);
    declareProperty("trimCount", m_trimCount= 14);
}

StatusCode TkrDigiTruncationAlg::initialize() {
    // Purpose and Method: initializes TkrDigiTruncationAlg
    // Inputs: none
    // Outputs: a status code
    // Dependencies: value of m_type determining the type of tool to run.  If
    //               "none", no HitRemoval tool is used.
    // Restrictions and Caveats: none

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;

    if ( setProperties().isFailure() ) {
        log << MSG::ERROR << "setProperties() failed" << endreq;
        return StatusCode::FAILURE;
  }

    // depending upon the type, set type of Tools
    if ( m_type == "General" )
        sc = toolSvc()->retrieveTool("GeneralHitRemovalTool", m_tool);
    else if ( m_type == "" || m_type == "none" ) {
        // don't run a HitRemoval tool
        m_type = "none";
        log << MSG::INFO << "disabled, no HitRemoval tool will be used" << endreq;
    }
    else {
        log << MSG::FATAL << "no tool for m_type " << m_type<<" found!"<<endreq;
        return StatusCode::FAILURE;
    }

    if ( sc.isFailure() ) {
        log << MSG::FATAL << "could not retrieve " << m_tool->name() << endreq;
        return sc;
    }

    sc = toolSvc()->retrieveTool("TkrHitTruncationTool", m_trunc);

    return sc;
}


StatusCode TkrDigiTruncationAlg::execute() {
    // Purpose and Method: execution method (called once for every event)
    //                     Doesn't do anything but calls the chosen tool.
    // Inputs: none
    // Outputs: a status code
    // Dependencies: none
    // Restrictions and Caveats: none

    StatusCode sc = StatusCode::SUCCESS;
    if ( m_type == "" || m_type == "none" )
        // don't run a HitRemoval tool
        return sc;
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "execute" << endreq;

    m_tkrDigiCol = SmartDataPtr<Event::TkrDigiCol>(eventSvc(),
        EventModel::Digi::TkrDigiCol);
    if(!m_tkrDigiCol) return StatusCode::SUCCESS;
    unsigned nDigis = m_tkrDigiCol->size();

    // analyze the Digis, as presented -> truncation information
    m_trunc->addEmptyDigis();
    m_trunc->analyzeDigis();

    if(m_trimDigis) {
        // now apply the truncation, and redo the analysis
        m_tool->doTrimDigis(true);
        m_tool->setTrimCount(m_trimCount);
        sc = m_tool->truncateDigis();
        m_trunc->setTrimCount(m_trimCount);
        sc = m_trunc->trimDigis();
        m_tool->doTrimDigis(false);
    }

    m_trunc->removeEmptyTruncs();
    if(m_tkrDigiCol->size()>nDigis) m_trunc->removeEmptyDigis();

    return sc;
}

StatusCode TkrDigiTruncationAlg::finalize()
{
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
