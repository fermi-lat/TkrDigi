/*
 * @class GeneralHitToDigiTool
 *
 * @brief Converts Tkr hits into Tkr digis.
 * This tool is a merge of code used in the packages GlastDigi v4r7 and TkrDigi
 * v1r11p2.
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/General/GeneralHitToDigiTool.h,v 1.1 2004/02/24 13:57:34 kuss Exp $
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

    static const double totAt1Mip()    { return s_totAt1Mip; }
    static const double mevPerMip()    { return s_mevPerMip; }
    static const double totThreshold() { return s_totThreshold; }
    static const int    totMax()       { return s_totMax; }
    static const int    maxHits()      { return s_maxHits;}

private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc*   m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*       m_gdSvc;
    /// Pointer to the tracker geometry service
    ITkrGeometrySvc*    m_tgSvc;
    /// Pointer to the tracker failure mode service
    ITkrFailureModeSvc* m_tfmSvc;
    /// Pointer to the tracker bad strips service
    ITkrBadStripsSvc*   m_tbsSvc;
    /// Pointer to the tracker alignment service
    ITkrAlignmentSvc*   m_taSvc;

    /// if true, kill bad strips in digi
    bool   m_killBadStrips;
    /// if true, kill failed layers and towers in digi
    bool   m_killFailed;

    /// average tot response to a MIP
    static double s_totAt1Mip;
    /// average eloss in silicon per MIP (to calculate ToT from eloss
    static double s_mevPerMip;
    /// threshold for ToT (in Mips)
    static double s_totThreshold;
    /// maximum value of ToT
    static int    s_totMax;
    /// maximum number of hits per side
    static int    s_maxHits;

};

#endif