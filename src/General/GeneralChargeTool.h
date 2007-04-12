/*
 * @class GeneralChargeTool
 *
 * @brief Adds electronic noise to strips and noisy strips to the SiStripList.
 * Uses TkrSimpleDigiAlg::addCharge() of TkrDigi v1r11p2.
 *
 * @authors Toby Burnett, Leon Rochester, Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/General/GeneralChargeTool.h,v 1.1 2004/02/27 10:14:15 kuss Exp $
 */

#ifndef __GENERALCHARGETOOL_H__
#define __GENERALCHARGETOOL_H__

#include "../IChargeTool.h"

#include "TkrUtil/ITkrToTSvc.h"

#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

#include <string>

#include "../SiPlaneMapContainer.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"



class GeneralChargeTool : public AlgTool, virtual public IChargeTool {

 public:

     enum {nFrac = 10};

    /// Standard Gaudi Tool interface constructor
    GeneralChargeTool(const std::string&, const std::string&, const IInterface*);
    /// Initializes the tool
    StatusCode initialize();
    /// runs the tool
    StatusCode execute();

 private:

    /// Pointer to the event data service (aka "eventSvc")
    IDataProviderSvc* m_edSvc;
    /// Pointer to the Glast detector service
    IGlastDetSvc*     m_gdSvc;
    /// Pointer to the ToT service
    ITkrToTSvc*       m_totSvc;
    ///
    double m_chargeFrac[nFrac];

};

#endif
