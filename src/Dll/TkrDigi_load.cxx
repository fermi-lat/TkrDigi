/*
 * @file TkrDigi_load.cxx
 *
 * @brief This is needed for forcing the linker to load all components
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Dll/TkrDigi_load.cxx,v 1.7 2008/12/02 00:04:35 usher Exp $
 */

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(TkrDigi)
{
  DECLARE_ALGORITHM(TkrDigiAlg);
  DECLARE_ALGORITHM(TkrDigiMcToHitAlg);
  DECLARE_ALGORITHM(TkrDigiNoiseAlg);
  DECLARE_ALGORITHM(TkrDigiHitRemovalAlg);
  DECLARE_ALGORITHM(TkrDigiHitToDigiAlg);
  DECLARE_ALGORITHM(TkrDigiChargeAlg);
  DECLARE_ALGORITHM(TkrDigiMergeTruncationAlg);

  DECLARE_TOOL     (BariMcToHitTool);
  DECLARE_TOOL     (SimpleMcToHitTool);
  DECLARE_TOOL     (GeneralNoiseTool);
  DECLARE_TOOL     (GeneralHitRemovalTool);
  DECLARE_TOOL     (GeneralHitToDigiTool);
  DECLARE_TOOL     (GeneralChargeTool);
  DECLARE_TOOL     (TkrDigiRandom); 
}
