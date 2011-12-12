/*
 * @file TkrDigiMergeTruncationAlg.cxx
 *
 * @brief Calls an user-chosen tool to add noisy strips to the SiStripList.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/src/GaudiAlg/TkrDigiMergeTruncationAlg.cxx,v 1.1.146.1 2011/01/13 21:21:45 jrb Exp $
 */

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

#include "GaudiKernel/Algorithm.h"

#include "../IHitRemovalTool.h"

#include <string>


class TkrDigiMergeTruncationAlg : public Algorithm {

 public:

    TkrDigiMergeTruncationAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /// Type of tool to run
    std::string m_type;
    /// Pointer to the tool
    IHitRemovalTool* m_tool;
};


// Used by Gaudi for identifying this algorithm
//static const AlgFactory<TkrDigiMergeTruncationAlg>    Factory;
//const IAlgFactory& TkrDigiMergeTruncationAlgFactory = Factory;
DECLARE_ALGORITHM_FACTORY(TkrDigiMergeTruncationAlg);

TkrDigiMergeTruncationAlg::TkrDigiMergeTruncationAlg(const std::string& name,
                                 ISvcLocator* pSvcLocator)
    : Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type="General");
}


StatusCode TkrDigiMergeTruncationAlg::initialize() {
    // Purpose and Method: initializes TkrDigiMergeTruncationAlg
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

    return sc;
}


StatusCode TkrDigiMergeTruncationAlg::execute() {
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

    sc = m_tool->truncateDigis();

    return sc;
}

StatusCode TkrDigiMergeTruncationAlg::finalize()
{
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
