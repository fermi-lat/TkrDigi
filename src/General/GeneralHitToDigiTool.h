/*
 * @class GeneralHitToDigiTool
 *
 * @brief Converts Tkr hits into Tkr digis.
 * This tool is a merge of code used in the packages GlastDigi v4r7 and TkrDigi
 * v1r11p2.
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralHitToDigiTool.h,v 1.7 2005/08/16 22:00:27 lsrea Exp $
 */

#ifndef __GENERALHITTODIGITOOL_H__
#define __GENERALHITTODIGITOOL_H__

#include "../IHitToDigiTool.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"
#include "TkrUtil/ITkrGeometrySvc.h"
#include "TkrUtil/ITkrFailureModeSvc.h"
#include "TkrUtil/ITkrBadStripsSvc.h"
#include "TkrUtil/ITkrAlignmentSvc.h"

#include <string>


class GeneralHitToDigiTool : public AlgTool, virtual public IHitToDigiTool {

 public:

    /// Standard Gaudi Tool interface constructor
    GeneralHitToDigiTool(const std::string&, const std::string&,
                         const IInterface*);
    /// Initializes the tool
    StatusCode initialize();
    /// Runs the tool
    StatusCode execute();

    //static const double totThreshold() { return s_totThreshold; }
    static const int    maxHits()      { return s_maxHits;}

private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc*   m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*       m_gdSvc;
    /// Pointer to the tracker geometry service
    ITkrGeometrySvc*    m_tkrGeom;
    /// Pointer to the tracker failure mode service
    ITkrFailureModeSvc* m_tfmSvc;
    /// Pointer to the tracker bad strips service
    ITkrBadStripsSvc*   m_tbsSvc;
    /// Pointer to the tracker alignment service
    ITkrAlignmentSvc*   m_taSvc;
    /// Pointer to the tracker splits service
    ITkrSplitsSvc*      m_tspSvc;
    /// Pointer to the tracker ToT service
    ITkrToTSvc*      m_ttotSvc;

    /// if true, kill bad strips in digi
    bool   m_killBadStrips;
    /// if true, kill failed layers and towers in digi
    bool   m_killFailed;

    /// threshold for ToT (in Mips)
    double m_totThreshold;
    /// maximum number of hits per side
    static int    s_maxHits;

};

#endif
