/**
 * @class IMcToHitTool
 *
 * @brief  Abstract interface to the McToHit tools.
 * Currently there are two choices, "Simple" or "Bari".
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/IMcToHitTool.h,v 1.1 2004/02/24 13:57:32 kuss Exp $
 */

#ifndef __IMCTOHITTOOL_H__
#define __IMCTOHITTOOL_H__

#include "GaudiKernel/IAlgTool.h"


static const InterfaceID IID_IMcToHitTool("IMcToHitTool", 1, 0);


class IMcToHitTool : virtual public IAlgTool {

 public:

    /// Interface ID
    static const InterfaceID& interfaceID() { return IID_IMcToHitTool; }

    /**
     * Method to perform the McToHit conversion.  Returns a status code upon
     * completion.
     */
    virtual StatusCode execute() = 0;

};

#endif
