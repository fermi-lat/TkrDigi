/*
 * @class TkrDigiAlg
 *
 * @brief Tracker digitization algorithm.
 * TkrDigiAlg is the master algorithm of TkrDigi.  It calls several
 * sub-algorithms sequentially, currently:
 *     TkrDigiMcToHitAlg
 *     TkrDigiNoiseAlg
 *     TkrDigiHitToDigiAlg
 *     TkrHitRemovalAlg
 *     TkrChargeAlg

 * Each sub-algorithm can choose among different tools.  At the end, MC hits are
 * converted into tkr digis.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/GaudiAlg/TkrDigiAlg.h,v 1.5 2011/12/12 20:56:09 heather Exp $
 */

#ifndef __TKRDIGIALG_H__
#define __TKRDIGIALG_H__

#include "GaudiKernel/Algorithm.h"
#include "GlastSvc/GlastRandomSvc/IRandomAccess.h"

#include <string>


class TkrDigiAlg : public Algorithm {

 public:

    TkrDigiAlg(const std::string&, ISvcLocator*);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();

 private:

    /**
     * Type of tool to run.  Will be overwritten if in the initialization of the
     * particular tool another type is chosen.
     */
    std::string m_type;
    /// Pointers to the sub algorithms
    Algorithm* m_mcToHitAlg;
    Algorithm* m_hitRemovalAlg;
    Algorithm* m_noiseAlg;
    Algorithm* m_hitToDigiAlg;
    Algorithm* m_chargeAlg;

    IDataProviderSvc* m_edSvc;

    IRandomAccess *m_randTool;

};

#endif
