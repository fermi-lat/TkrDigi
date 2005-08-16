/*
 * @file TkrDigiAlg.cxx
 *
 * @brief Tracker digitization algorithm.
 * TkrDigiAlg is the master algorithm of TkrDigi.  It calls several
 * sub-algorithms sequentially, currently:
 *     TkrDigiMcToHitAlg
 *     TkrDigiHitRemovalAlg
 *     TkrDigiNoiseAlg
 *     TkrDigiHitToDigiAlg
 * Each sub-algorithm can choose among different tools.  At the end, MC hits are
 * converted into tkr digis.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiAlg.cxx,v 1.2 2004/03/09 20:06:30 lsrea Exp $
 */

#include "TkrDigiAlg.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"


// Definitions for use within Gaudi
static const AlgFactory<TkrDigiAlg>    Factory;
const IAlgFactory& TkrDigiAlgFactory = Factory;


TkrDigiAlg::TkrDigiAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type="Simple");
}


// public member functions
StatusCode TkrDigiAlg::initialize() {
    // Purpose and Method: initializes tracker digitization
    // Inputs: none
    // Outputs: a status code
    // Dependencies: value of m_type determining the type of digitization to run
    // Restrictions and Caveats: none

    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;

    if ( setProperties().isFailure() ) {
        log << MSG::ERROR << "setProperties() failed" << endreq;
        return StatusCode::FAILURE;
    }

    // creating the sub algorithms

    if ( createSubAlgorithm("TkrDigiMcToHitAlg", "TkrDigiMcToHitAlg",
                            m_mcToHitAlg).isFailure() ) {
        log << MSG::ERROR << "could not open TkrDigiMcToHitAlg" << endreq;
        return StatusCode::FAILURE;
    }

    if ( createSubAlgorithm("TkrDigiNoiseAlg", "TkrDigiNoiseAlg",
                            m_noiseAlg).isFailure() ) {
        log << MSG::ERROR << "could not open TkrDigiNoiseAlg" << endreq;
        return StatusCode::FAILURE;
    }

    if ( createSubAlgorithm("TkrDigiHitRemovalAlg", "TkrDigiHitRemovalAlg",
                            m_hitRemovalAlg).isFailure() ) {
        log << MSG::ERROR << "could not open TkrDigiHitRemovalAlg" << endreq;
        return StatusCode::FAILURE;
    }

    if ( createSubAlgorithm("TkrDigiHitToDigiAlg", "TkrDigiHitToDigiAlg",
                            m_hitToDigiAlg).isFailure() ) {
        log << MSG::ERROR << "could not open TkrDigiHitToDigiAlg" << endreq;
        return StatusCode::FAILURE;
    }

    // Sets the property controlling the type of Tool to be used.
    m_mcToHitAlg->setProperty(  "Type", m_type);
    // Currently only one choice, "General".
    m_noiseAlg->setProperty(    "Type", "General");
    // Currently only one choice, "General"
    m_hitRemovalAlg->setProperty( "Type", "General");
    // Currently only one choice, "General".
    m_hitToDigiAlg->setProperty("Type", "General");

    return StatusCode::SUCCESS;
}


StatusCode TkrDigiAlg::execute() {
    // Purpose and Method: execution method (called once for every event).
    //                     Doesn't do anything but calls other algorithms.
    // Inputs: none
    // Outputs: a status code
    // Dependencies: none
    // Restrictions and Caveats: none

    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "execute" << endreq;

    // loading the sub algorithms

    if( m_mcToHitAlg->execute().isFailure() ) {
        log << MSG::ERROR << m_mcToHitAlg->name() << " FAILED to execute!"
            << endreq;
        return StatusCode::FAILURE;
    }

    if( m_noiseAlg->execute().isFailure() ) {
        log << MSG::ERROR << m_noiseAlg->name() << " FAILED to execute!"
            << endreq;
        return StatusCode::FAILURE;
    }

    if( m_hitRemovalAlg->execute().isFailure() ) {
        log << MSG::ERROR << m_hitRemovalAlg->name() << " FAILED to execute!"
            << endreq;
        return StatusCode::FAILURE;
    }


    if( m_hitToDigiAlg->execute().isFailure() ) {
        log << MSG::ERROR << m_hitToDigiAlg->name() << " FAILED to execute!"
            << endreq;
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}


StatusCode TkrDigiAlg::finalize() {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
