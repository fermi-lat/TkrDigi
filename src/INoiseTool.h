/**
 * @class INoiseTool
 *
 * @brief Abstract interface to the Noise tools.
 * Currently there is but one, "General", but this for future extensions.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/INoiseTool.h,v 1.1 2004/02/24 13:57:32 kuss Exp $
 */

#ifndef __INOISETOOL_H__
#define __INOISETOOL_H__

#include "GaudiKernel/IAlgTool.h"


static const InterfaceID IID_INoiseTool("INoiseTool", 1, 0);


class INoiseTool : virtual public IAlgTool {

 public:

    /// Interface ID
    static const InterfaceID& interfaceID() { return IID_INoiseTool; }

    /**
     * Method to add noise to hits and to generate noise hits.  Returns a status
     * code upon completion.
     */
    virtual StatusCode execute() = 0;

};

#endif
