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
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiAlg.cxx,v 1.5 2008/10/01 20:31:57 lsrea Exp $
 */

#include "TkrDigiAlg.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "Event/MonteCarlo/McParticle.h"

#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"


// Definitions for use within Gaudi
//static const AlgFactory<TkrDigiAlg>    Factory;
//const IAlgFactory& TkrDigiAlgFactory = Factory;
DECLARE_ALGORITHM_FACTORY(TkrDigiAlg);


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

    if (toolSvc()->retrieveTool("TkrDigiRandom", m_randTool).isFailure()) 
        log << MSG::WARNING << "Failed to create TkrDigiRandom" << endreq;

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

    if ( createSubAlgorithm("TkrDigiChargeAlg", "TkrDigiChargeAlg",
                            m_chargeAlg).isFailure() ) {
        log << MSG::ERROR << "could not open TkrDigiChargeAlg" << endreq;
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
    // "General", or skip for Bari.
    m_noiseAlg->setProperty(    "Type", (m_type=="Simple"? "General": "none"));
    // "General", or skip for Bari.
    m_chargeAlg->setProperty(    "Type", (m_type=="Simple"? "General": "none"));
    // Currently only one choice, "General"
    m_hitRemovalAlg->setProperty( "Type", "General");
    // Currently only one choice, "General".
    m_hitToDigiAlg->setProperty("Type", "General");

    IService* iService = 0;
    StatusCode sc = serviceLocator()->service("EventDataSvc", iService, true);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc !" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);


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

    // check that G4Generator ran successfully
    // if not, exit gracefully

    SmartDataPtr<Event::McParticleCol> 
        mcParts(m_edSvc, EventModel::MC::McParticleCol);

    if (!mcParts||mcParts->size()<=1) {
        return StatusCode::SUCCESS;
    }

    // loading the sub algorithms

    if( m_mcToHitAlg->execute().isFailure() ) {
        log << MSG::ERROR << m_mcToHitAlg->name() << " FAILED to execute!"
            << endreq;
        return StatusCode::FAILURE;
    }

    if( m_chargeAlg->execute().isFailure() ) {
        log << MSG::ERROR << m_chargeAlg->name() << " FAILED to execute!"
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
