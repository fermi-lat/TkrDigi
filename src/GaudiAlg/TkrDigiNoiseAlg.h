/**
 * @class TkrDigiNoiseAlg
 *
 * @brief Calls an user-chosen tool to add noisy strips to the SiStripList.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/GaudiAlg/TkrDigiNoiseAlg.h,v 1.1 2004/02/24 13:57:34 kuss Exp $
 */

#ifndef __TKRDIGINOISEALG_H__
#define __TKRDIGINOISEALG_H__

#include "../INoiseTool.h"

#include "GaudiKernel/Algorithm.h"

#include <string>


class TkrDigiNoiseAlg : public Algorithm {

 public:

    TkrDigiNoiseAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /// Type of tool to run
    std::string m_type;
    /// Pointer to the tool
    INoiseTool* m_tool;

};

#endif
