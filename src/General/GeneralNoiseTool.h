/*
 * @class GeneralNoiseTool
 *
 * @brief Adds electronic noise to strips and noisy strips to the SiStripList.
 * Uses TkrSimpleDigiAlg::addNoise() of TkrDigi v1r11p2.
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralNoiseTool.h,v 1.1 2004/02/27 10:14:15 kuss Exp $
 */

#ifndef __GENERALNOISETOOL_H__
#define __GENERALNOISETOOL_H__

#include "../INoiseTool.h"
#include "TkrUtil/ITkrToTSvc.h"
#include "../SiLayerList.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"


#include <string>


class GeneralNoiseTool : public AlgTool, virtual public INoiseTool {

 public:

    /// Standard Gaudi Tool interface constructor
    GeneralNoiseTool(const std::string&, const std::string&, const IInterface*);
    /// Initializes the tool
    StatusCode initialize();
    /// runs the tool
    StatusCode execute();

    double noiseThreshold() const { return m_noiseThreshold; }
    double dataThreshold()  const { return m_noiseThreshold; }
    double triggerThreshold() const { return m_trigThreshold; }
    double noiseSigma()     const { return m_noiseSigma; }
    double noiseOccupancy() const { return m_noiseOccupancy; }

 private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc* m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*     m_gdSvc;
    /// Pointer to the ToT service
    ITkrToTSvc*       m_totSvc;

    /// list of all SiLayers found in the detector model
    SiLayerList m_layers;
    /// energy deposit above which hit is recorded (MeV)
    double m_noiseThreshold;
    /// energy deposit above which the hit triggers (MeV);
    double m_trigThreshold;
    /// amount to fluctuate hit strips (MeV)
    double m_noiseSigma;
    /// frequency of noise hits
    double m_noiseOccupancy; 
    /// do full threshold analysis
    bool   m_fullThreshold;

};

#endif
