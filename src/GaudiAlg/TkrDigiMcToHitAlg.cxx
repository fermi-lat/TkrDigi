/*
 * @file TkrDigiMcToHitAlg.cxx
 *
 * @brief Calls either Simple or Bari tool to convert MC hits into tkr hits.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/src/GaudiAlg/TkrDigiMcToHitAlg.cxx,v 1.2.656.1 2011/01/13 21:21:45 jrb Exp $
 */

#include "TkrDigiMcToHitAlg.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

// Used by Gaudi for identifying this algorithm
//static const AlgFactory<TkrDigiMcToHitAlg>    Factory;
//const IAlgFactory& TkrDigiMcToHitAlgFactory = Factory;
DECLARE_ALGORITHM_FACTORY(TkrDigiMcToHitAlg);

TkrDigiMcToHitAlg::TkrDigiMcToHitAlg(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type);
}


StatusCode TkrDigiMcToHitAlg::initialize() {
    // Purpose and Method: initializes TkrDigiMcToHitAlg
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

    // depending upon the type, set type of tool
    if ( m_type == "Simple" )
        sc = toolSvc()->retrieveTool("SimpleMcToHitTool", m_tool);
    else if ( m_type == "Bari" )
        sc = toolSvc()->retrieveTool("BariMcToHitTool", m_tool);
    else {
        log << MSG::FATAL << "no tool for m_type " << m_type << " found!"
            << endreq;
        return StatusCode::FAILURE;
    }
    if ( sc.isFailure() ) {
        log << MSG::FATAL << "could not retrieve " << m_tool->name() << endreq;
        return sc;
    }

    return sc;
}


StatusCode TkrDigiMcToHitAlg::execute() {
    // Purpose and Method: execution method (called once for every event)
    //                     Doesn't do anything but calls the chosen tool.
    // Inputs: none
    // Outputs: a status code
    // Dependencies: none
    // Restrictions and Caveats: none

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "execute" << endreq;

    sc = m_tool->execute();

    return sc;
}


StatusCode TkrDigiMcToHitAlg::finalize() {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
