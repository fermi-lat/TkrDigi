/*
 * @file TkrDigiHitRemovalAlg.cxx
 *
 * @brief Calls an user-chosen tool to add noisy strips to the SiStripList.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiHitRemovalAlg.cxx,v 1.2 2004/03/09 20:06:30 lsrea Exp $
 */

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"

#include "GaudiKernel/Algorithm.h"

#include "../IHitRemovalTool.h"

#include <string>


class TkrDigiHitRemovalAlg : public Algorithm {

 public:

    TkrDigiHitRemovalAlg(const std::string&, ISvcLocator*);
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
static const AlgFactory<TkrDigiHitRemovalAlg>    Factory;
const IAlgFactory& TkrDigiHitRemovalAlgFactory = Factory;


TkrDigiHitRemovalAlg::TkrDigiHitRemovalAlg(const std::string& name,
                                 ISvcLocator* pSvcLocator)
    : Algorithm(name, pSvcLocator) {
    // variable to select the tool type
    declareProperty("Type", m_type="General");
}


StatusCode TkrDigiHitRemovalAlg::initialize() {
    // Purpose and Method: initializes TkrDigiHitRemovalAlg
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


StatusCode TkrDigiHitRemovalAlg::execute() {
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

    sc = m_tool->execute();

    return sc;
}

StatusCode TkrDigiHitRemovalAlg::finalize()
{
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    return StatusCode::SUCCESS;
}
