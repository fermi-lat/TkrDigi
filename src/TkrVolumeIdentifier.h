/**
 * @class TkrVolumeIdentifier
 *
 * @brief Provides methods to manipulate volume identifiers.
 *
 * TkrVolumeIdentifier inherits from idents::VolumeIdentifier.  It contains the
 * same information (no extra member variables), but methods to retrieve
 * information from a volume identifier.  This enables users to retrieve i.e.
 * the tray id without actually knowing where this tray id is stored.
 * Additionally, there are some extra constructors and copy constructors.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/VolumeIdentifier.h,v 1.1 2004/02/24 13:57:32 kuss Exp $
 */

#ifndef __TKRVOLUMEIDENTIFIER_H__
#define __TKRVOLUMEIDENTIFIER_H__

#include "idents/GlastAxis.h"
#include "idents/VolumeIdentifier.h"
#include "idents/TowerId.h"


class TkrVolumeIdentifier : public idents::VolumeIdentifier {

 public:

    /// Uses the default constructor from idents::VolumeIdentifier
    TkrVolumeIdentifier() : idents::VolumeIdentifier() {}

    /// Initializes from an idents::VolumeIdentifier.
    TkrVolumeIdentifier(const idents::VolumeIdentifier& id) :
        idents::VolumeIdentifier() {
        init(id.getValue(), id.size());
    }

    /**
     * Constructs a truncated volume identifier.  Ladder and wafer information
     * are not filled.
     * @param tower  a tower id (like in idents::TowerId.id())
     * @param layer  a biLayer number
     * @param view   the view
     */
    TkrVolumeIdentifier(const int& tower, const int& layer, const int& view);

    /**
     * Assigns an idents::VolumeIdentifier to a TkrVolumeIdentifier.
     */
    TkrVolumeIdentifier& operator=(const idents::VolumeIdentifier& id);

    // According to (1), the name of field 0 is "LATObject".
    int getLatObject()   const { return operator[](0); }
    // The following functions make sense only if isLatTowers()==true
    int getTowerY()      const { return operator[](1); }
    int getTowerX()      const { return operator[](2); }
    // According to (1), the name of field 3 is "TowerObject".
    int getTowerObject() const { return operator[](3); }
    // the following functions make sense only if isTowerTkr()==true
    int getTray()        const { return operator[](4); }
    int getView()        const { return operator[](5); }
    int getBotTop()      const { return operator[](6); }
    int getLadder()      const { return operator[](7); }
    int getWafer()       const { return operator[](8); }

    /// Returns an idents::TowerId
    idents::TowerId getTower() const {
        return idents::TowerId(getTowerX(), getTowerY()).id();
    }

    /// Returns the view as a GlastAxis::axis.  Some functions require this.
    idents::GlastAxis::axis getAxis() const {
        return getView()==0 ? idents::GlastAxis::X : idents::GlastAxis::Y;
    }

    /// Converts tray and botTop information into a layer counter
    int getLayer() const { return getTray() - 1 + getBotTop(); }

    /**
     * Returns a truncated idents::VolumeIdentifier.  Ladder and wafer
     * information are stripped off.  Assignment to a TkrVolumeIdentifier
     * is possible because of the explicit copy assignment
     * TkrVolumeIdentifier& operator=(const idents::VolumeIdentifier& id).
     */
    idents::VolumeIdentifier getPlaneId() const;

    /// Returns true if the element is in "LATTowers" (1).
    bool isLatTowers() const { return getLatObject() == 0; }

    /// Returns true if the element is in "TowerTKR" (1).
    bool isTowerTkr()  const { return isLatTowers() && getTowerObject() == 1; }

}; // end class

// References:
//
// (1) http://www.slac.stanford.edu/exp/glast/ground/software/geometry/docs/identifiers/geoId-RitzId.shtml

#endif

