/*
 * @class SimpleMcToHitTool
 *
 * @brief Converts MC hits into tkr hits, using the Simple method.
 * Extracted from TkrDigi/v1r11p2/src/TkrSimpleDigiAlg.cxx
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/Simple/SimpleMcToHitTool.h,v 1.1 2004/02/24 13:57:34 kuss Exp $
 */

#ifndef __SIMPLEMCTOHITTOOL_H__
#define __SIMPLEMCTOHITTOOL_H__

#include "../IMcToHitTool.h"

#include "../SiPlaneMapContainer.h"

#include "TkrUtil/ITkrGeometrySvc.h"
#include "TkrUtil/ITkrAlignmentSvc.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include <string>


class SimpleMcToHitTool : public AlgTool, virtual public IMcToHitTool {

 public:

    /// Standard Gaudi Tool interface constructor
    SimpleMcToHitTool(const std::string&,const std::string&,const IInterface*);
    /// Initializes the tool
    StatusCode initialize();
    /// Runs the tool
    StatusCode execute();
    /** Fills a SiPlaneMap with information based on the McPositionHitVector
     * @param a McPositionHitVector
     * @return a SiPlaneMap
     */
    SiPlaneMapContainer::SiPlaneMap createSiHits(const
                                                 Event::McPositionHitVector&);

 private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc*   m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*       m_gdSvc;
    /// Pointer to the tracker geometry service
    ITkrGeometrySvc*    m_tgSvc;
    /// Pointer to the tracker alignment service
    ITkrAlignmentSvc*   m_taSvc;

};

#endif
