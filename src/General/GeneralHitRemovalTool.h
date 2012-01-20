/*
* @class GeneralHitRemovalTool
*
* @brief Truncates the strip lists to account for finite controller and cable buffers.
* Called from TkrDigiAlg
*
* @authors Leon Rochester
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralHitRemovalTool.h,v 1.2 2008/12/08 01:55:04 lsrea Exp $
*/

#ifndef __GENERALHitRemovalTOOL_H__
#define __GENERALHitRemovalTOOL_H__

#include "../IHitRemovalTool.h"
#include "../SiPlaneMapContainer.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"
#include "TkrUtil/ITkrGeometrySvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include <string>

class GeneralHitRemovalTool : public AlgTool, virtual public IHitRemovalTool {

public:

    /// Standard Gaudi Tool interface constructor
    GeneralHitRemovalTool(const std::string&, const std::string&, const IInterface*);
    /// Initializes the tool
    StatusCode initialize();
    /// runs the tool
    StatusCode execute();
    /// truncates the digis after merging
    StatusCode truncateDigis();

private:

    // does the FailureMode and BadStrips
    int killBadHitsLoop(SiPlaneMapContainer::SiPlaneMap& siPlaneMap);
    // does the RC buffers
    int doRCBufferLoop(SiPlaneMapContainer::SiPlaneMap& siPlaneMap);
    // does the cable buffer
    int doCableBufferLoop(SiPlaneMapContainer::SiPlaneMap& siPlaneMap,
        bool& towersOutofOrder, bool& planesOutofOrder);

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc* m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*     m_gdSvc;
    /// Pointer to TkrGeometrySvc
    ITkrGeometrySvc*  m_tkrGeom;
    /// Pointer to TkrSplitsSvc
    ITkrSplitsSvc*    m_splitsSvc;
    /// Pointer to TkrFailureModeSvc;
    ITkrFailureModeSvc* m_failSvc;
    /// Pointer to TkrBadStripsSvc;
    ITkrBadStripsSvc* m_badStripsSvc;
    /// flag to kill bad strips
    bool m_killBadStrips;
    /// flag to kill failed layers
    bool m_killFailed;
    /// local flags
    bool m_doFailed;
    bool m_doBad;
    bool m_doTrunc;
};

#endif
