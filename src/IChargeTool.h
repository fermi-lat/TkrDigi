/**
 * @class IChargeTool
 *
 * @brief Abstract interface to the Charge Effects tool.
 * Currently there is but one, "General", but this for future extensions.
 *
 * @author Michael Kuss
 *
 * $Header $
 */

#ifndef __ICHARGETOOL_H__
#define __ICHARGETOOL_H__

#include "GaudiKernel/IAlgTool.h"
//#include "SiLayerList.h"


static const InterfaceID IID_IChargeTool("IChargeTool", 1, 0);


class IChargeTool : virtual public IAlgTool {

 public:

    /// Interface ID
    static const InterfaceID& interfaceID() { return IID_IChargeTool; }

    /**
     * Method to add charge effects to hits .  Returns a status
     * code upon completion.
     */
    virtual StatusCode execute() = 0;

};

#endif
