/**
 * @class SiPlaneMapContainer
 *
 * @brief  SiPlaneMapContainer serves as a TDS container for a SiPlaneMap.
 * A TDS object needs to inherit from DataObject (or ContainedObject).  Gaudi
 * (v11r4p0) provides TDS containers for vectors and lists, but not for maps.
 * This here is a work-around.  Should I call it ObjectMap?
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/SiPlaneMapContainer.h,v 1.8 2004/02/24 13:57:32 kuss Exp $
*/

#ifndef __SIPLANEMAPCONTAINER_H__
#define __SIPLANEMAPCONTAINER_H__

#include "SiStripList.h"

#include "idents/VolumeIdentifier.h"

#include "GaudiKernel/DataObject.h"

#include <map>


class SiPlaneMapContainer : public DataObject {

 public:
 
    typedef std::map<idents::VolumeIdentifier, SiStripList*> SiPlaneMap;

    /// Initializes the container with a SiPlaneMap
    SiPlaneMapContainer(const SiPlaneMap m): m_SiPlaneMap(m) {}

    /// Returns the SiPlaneMap
    SiPlaneMap& getSiPlaneMap() { return m_SiPlaneMap; }

 private:

    SiPlaneMap m_SiPlaneMap;

};

#endif
