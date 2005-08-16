/*
 * @file TkrDigi_load.cxx
 *
 * @brief This is needed for forcing the linker to load all components
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Dll/TkrDigi_load.cxx,v 1.3 2004/02/27 10:14:15 kuss Exp $
 */

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(TkrDigi)
{
  DECLARE_ALGORITHM(TkrDigiAlg);
  DECLARE_ALGORITHM(TkrDigiMcToHitAlg);
  DECLARE_ALGORITHM(TkrDigiNoiseAlg);
  DECLARE_ALGORITHM(TkrDigiHitRemovalAlg);
  DECLARE_ALGORITHM(TkrDigiHitToDigiAlg);
  DECLARE_TOOL     (BariMcToHitTool);
  DECLARE_TOOL     (SimpleMcToHitTool);
  DECLARE_TOOL     (GeneralNoiseTool);
  DECLARE_TOOL     (GeneralHitRemovalTool);
  DECLARE_TOOL     (GeneralHitToDigiTool);
  DECLARE_TOOL     (TkrDigiRandom); 
}
