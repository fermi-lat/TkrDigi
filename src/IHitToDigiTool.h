/**
 * @class IHitToDigiTool
 *
 * @brief Abstract interface to the HitToDigi tools.
 * Currently there is but one, "General", but this for future extensions.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/IHitToDigiTool.h,v 1.1 2004/02/24 13:57:32 kuss Exp $
 */

#ifndef __IHITTODIGITOOL_H__
#define __IHITTODIGITOOL_H__

#include "GaudiKernel/IAlgTool.h"


static const InterfaceID IID_IHitToDigiTool("IHitToDigiTool", 1, 0);


class IHitToDigiTool : virtual public IAlgTool {

 public:

    /// Interface ID
    static const InterfaceID& interfaceID() { return IID_IHitToDigiTool; }

    /**
     * Method to perform the HitToDigi conversion.  Returns a status code upon
     * completion.
     */
    virtual StatusCode execute() = 0;

};

#endif
