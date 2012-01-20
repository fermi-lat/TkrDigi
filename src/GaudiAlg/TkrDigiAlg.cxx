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
static const AlgFactory<TkrDigiAlg>    Factory;
const IAlgFactory& TkrDigiAlgFactory = Factory;
namespace {
    // for looping over the sub-algorithms
    const int nAlgs = 7;
    Algorithm* ptrAlg[nAlgs];
    std::string algName[nAlgs] = {
        "TkrDigiMcToHitAlg",    "TkrDigiChargeAlg",    "TkrDigiNoiseAlg",
        "TkrDigiHitRemovalAlg", "TkrDigiHitToDigiAlg", "TkrFillTDInfoAlg",
        "TkrDigiMergeTruncationAlg"
    };
    typedef enum algType { MCTOHIT,    CHARGE,    NOISE, 
                           HITREMOVAL, HITTODIGI, FILLTDINFO, 
                           TRUNCATION };
}


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

    int iAlg;
    for(iAlg=0;iAlg<nAlgs;++iAlg) {
        ptrAlg[iAlg] = 0;
        if ( createSubAlgorithm(algName[iAlg], algName[iAlg],
                                ptrAlg[iAlg]).isFailure() ) {

            log << MSG::ERROR << "could not open " << algName[iAlg] << endreq;

        return StatusCode::FAILURE;
    }


    }

    // These algorithms require special initialization:
    ptrAlg[MCTOHIT]->setProperty(  "Type", m_type);
    // "General", or skip for Bari.
    ptrAlg[NOISE]->setProperty(    "Type", (m_type=="Simple"? "General": "none"));
    // "General", or skip for Bari.
    ptrAlg[CHARGE]->setProperty(    "Type", (m_type=="Simple"? "General": "none"));
    // Currently only one choice, "General"
    ptrAlg[HITREMOVAL]->setProperty( "Type", "General");
    // Currently only one choice, "General".
    ptrAlg[HITTODIGI]->setProperty("Type", "General");

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

    int iAlg;
    for(iAlg=0;iAlg<nAlgs;++iAlg) {
        if( ptrAlg[iAlg]->execute().isFailure() ) {
            log << MSG::ERROR << algName[iAlg] << " FAILED to execute!"
            << endreq;
        return StatusCode::FAILURE;
    }





    }

    return StatusCode::SUCCESS;
}


StatusCode TkrDigiAlg::finalize() {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
