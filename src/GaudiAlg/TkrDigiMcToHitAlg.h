/*
 * @class TkrDigiMcToHitAlg
 *
 * @brief Calls either Simple or Bari tool to convert MC hits into tkr hits.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/GaudiAlg/TkrDigiMcToHitAlg.h,v 1.1 2004/02/24 13:57:34 kuss Exp $
 */

#ifndef __TKRDIGIMCTOHITALG_H__
#define __TKRDIGIMCTOHITALG_H__

#include "../IMcToHitTool.h"

#include "GaudiKernel/Algorithm.h"

class TkrDigiMcToHitAlg : public Algorithm {

 public:

    TkrDigiMcToHitAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /// Type of tool to run
    std::string m_type;
    /// Pointer to the tool
    IMcToHitTool* m_tool;

};

#endif
