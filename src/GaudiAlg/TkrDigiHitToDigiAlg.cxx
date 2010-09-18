/*
 * @file TkrDigiHitToDigiAlg.cxx
 *
 * @brief Calls an user-chosen tool to convert tkr hits into tkr digis.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiHitToDigiAlg.cxx,v 1.2 2004/03/09 20:06:30 lsrea Exp $
 */

#include "TkrDigiHitToDigiAlg.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"


// Used by Gaudi for identifying this algorithm
//static const AlgFactory<TkrDigiHitToDigiAlg>    Factory;
//const IAlgFactory& TkrDigiHitToDigiAlgFactory = Factory;
DECLARE_ALGORITHM_FACTORY(TkrDigiHitToDigiAlg);

TkrDigiHitToDigiAlg::TkrDigiHitToDigiAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type="General");
}


StatusCode TkrDigiHitToDigiAlg::initialize() {
    // Purpose and Method: initializes TkrDigiHitToDigiAlg
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

    // depending upon the type, set type of Tools
    if ( m_type == "General" )
        sc = toolSvc()->retrieveTool("GeneralHitToDigiTool", m_tool);
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


StatusCode TkrDigiHitToDigiAlg::execute() {
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


StatusCode TkrDigiHitToDigiAlg::finalize() {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
