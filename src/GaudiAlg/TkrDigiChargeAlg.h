/**
 * @class TkrDigiChargeAlg
 *
 * @brief Calls an user-chosen tool to add noisy strips to the SiStripList.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiChargeAlg.h,v 1.1 2004/02/27 10:14:15 kuss Exp $
 */

#ifndef __TKRDIGINOISEALG_H__
#define __TKRDIGINOISEALG_H__

#include "../IChargeTool.h"

#include "GaudiKernel/Algorithm.h"

#include <string>


class TkrDigiChargeAlg : public Algorithm {

 public:

    TkrDigiChargeAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /// Type of tool to run
    std::string m_type;
    /// Pointer to the tool
    IChargeTool* m_tool;

};

#endif
