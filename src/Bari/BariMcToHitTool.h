/*
 * @class BariMcToHitTool
 *
 * @brief Converts MC hits into tkr hits, using the Bari method.
 * Extracted from GlastDigi/v4r6/src/Tkr/TkrBariDigiAlg.cxx
 *
 * @authors Nico Giglietto, Monica Brigida, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/BariMcToHitTool.h,v 1.4 2007/04/27 10:09:11 ngigliet Exp $
 */

#ifndef __BARIMCTOHITTOOL_H__
#define __BARIMCTOHITTOOL_H__

#include "../IMcToHitTool.h"
#include "InitCurrent.h"

#include "TkrUtil/ITkrGeometrySvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "TkrUtil/ITkrToTSvc.h"
#include "../GaudiAlg/TkrDigiAlg.h"
#include <string>


class BariMcToHitTool : public AlgTool, virtual public IMcToHitTool {

 public:

  /// Standard Gaudi Tool interface constructor
  BariMcToHitTool(const std::string&, const std::string&, const IInterface*);
  /// Initializes the tool
  StatusCode initialize();
  /// Runs the tool
  StatusCode execute();
  

private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc* m_edSvc;
    /// File which stores the "correnti" information
    std::string       m_CurrentsFile;
    /// Extracted current information
    InitCurrent       m_openCurr;
    /// pointer to geometry svc
    ITkrGeometrySvc* m_tkrGeom;
    /// pointer to ToT svc
    ITkrToTSvc* pToTSvc;
    std::string m_type;
    /// Pointers to the sub algorithms
    TkrDigiAlg* m_BamcToHitAlg;
 
};

#endif
