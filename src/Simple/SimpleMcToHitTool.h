/*
 * @class SimpleMcToHitTool
 *
 * @brief Converts MC hits into tkr hits, using the Simple method.
 * Extracted from TkrDigi/v1r11p2/src/TkrSimpleDigiAlg.cxx
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Simple/SimpleMcToHitTool.h,v 1.3 2004/12/14 03:07:34 lsrea Exp $
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
                                                 Event::McPositionHitVector& pos,
                                                 const HepVector3D& dir=HepVector3D(0., 0., 1.0));

 private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc*   m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*       m_gdSvc;
    /// Pointer to the tracker geometry service
    ITkrGeometrySvc*    m_tkrGeom;
    /// Pointer to the tracker alignment service
    ITkrAlignmentSvc*   m_taSvc;
    /// flag to turn on test mode, deposits 0.155 MeV for every hit
    bool m_test;
    /// flag to set alignment mode: 0 = original, 1 = single event axis
    int  m_alignmentMode;
};

#endif
