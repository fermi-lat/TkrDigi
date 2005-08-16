/**
 * @class IHitRemovalTool
 *
 * @brief Abstract interface to the HitRemoval Tool.
 * Currently there is but one, "General", but this for future extensions.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/INoiseTool.h,v 1.1 2004/02/27 10:14:13 kuss Exp $
 */

#ifndef __IHitRemovalTOOL_H__
#define __IHitRemovalTOOL_H__

#include "GaudiKernel/IAlgTool.h"


static const InterfaceID IID_IHitRemovalTool("IHitRemovalTool", 1, 0);


class IHitRemovalTool : virtual public IAlgTool {

 public:

    /// Interface ID
    static const InterfaceID& interfaceID() { return IID_IHitRemovalTool; }

    /**
     * Method to truncate hits to account for finite controller and cable buffers,
     * as well as failed planes and bad strips
     * Returns a status code upon completion.
     */
    virtual StatusCode execute() = 0;

};

#endif
