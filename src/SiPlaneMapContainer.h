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
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiPlaneMapContainer.h,v 1.1 2004/02/27 10:14:13 kuss Exp $
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
    SiPlaneMapContainer(const SiPlaneMap m): m_siPlaneMap(m) {}

    /// Deletes the contained SiStripLists
    SiPlaneMapContainer::~SiPlaneMapContainer() {
        for ( SiPlaneMap::iterator it=m_siPlaneMap.begin();
              it!=m_siPlaneMap.end(); ++it ) {
            delete (*it).second;
            (*it).second = 0;
        }
        m_siPlaneMap.clear();
    }

    /// Returns the SiPlaneMap
    SiPlaneMap& getSiPlaneMap() { return m_siPlaneMap; }

 private:

    SiPlaneMap m_siPlaneMap;

};

#endif

