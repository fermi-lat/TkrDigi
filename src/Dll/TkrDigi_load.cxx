/** 
* @file TkrDigi_load.cpp
* @brief This is needed for forcing the linker to load all components
* of the library.
*
*  $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Dll/TkrDigi_load.cxx,v 1.1.1.1 2002/05/26 17:17:19 burnett Exp $
*/

#include "GaudiKernel/DeclareFactoryEntries.h"

DECLARE_FACTORY_ENTRIES(TkrDigi) {
    DECLARE_ALGORITHM( TkrSimpleDigiAlg );
    DECLARE_TOOL( TkrDigiRandom );
} 

