/*
 * @class GeneralNoiseTool
 *
 * @brief Adds electronic noise to strips and noisy strips to the SiStripList.
 * Uses TkrSimpleDigiAlg::addNoise() of TkrDigi v1r11p2.
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/General/GeneralNoiseTool.h,v 1.1 2004/02/24 13:57:34 kuss Exp $
 */

#ifndef __GENERALNOISETOOL_H__
#define __GENERALNOISETOOL_H__

#include "../INoiseTool.h"
#include "../SiLayerList.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include <string>


class GeneralNoiseTool : public AlgTool, virtual public INoiseTool {

 public:

    /// Standard Gaudi Tool interface constructor
    GeneralNoiseTool(const std::string&, const std::string&, const IInterface*);
    /// Initializes the tool
    StatusCode initialize();
    /// runs the tool
    StatusCode execute();

    static const double noiseThreshold() { return s_noiseThreshold; }
    static const double noiseSigma()     { return s_noiseSigma; }
    static const double noiseOccupancy() { return s_noiseOccupancy; }

 private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc* m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*     m_gdSvc;

    /// list of all SiLayers found in the detector model
    SiLayerList m_layers;
    /// energy deposit above which hit is recorded (MeV)
    static double s_noiseThreshold;
    /// amount to fluctuate hit strips (MeV)
    static double s_noiseSigma;
    /// frequency of noise hits
    static double s_noiseOccupancy;  

};

#endif
