/*
 * @class BariMcToHitTool
 *
 * @brief Converts MC hits into tkr hits, using the Bari method.
 * Extracted from GlastDigi/v4r6/src/Tkr/TkrBariDigiAlg.cxx
 *
 * @authors Nico Giglietto, Monica Brigida, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/Bari/BariMcToHitTool.h,v 1.1 2004/02/24 13:57:32 kuss Exp $
 */

#ifndef __BARIMCTOHITTOOL_H__
#define __BARIMCTOHITTOOL_H__

#include "../IMcToHitTool.h"
#include "InitCurrent.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

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

};

#endif
