/**
 * @file TkrVolumeIdentifier.cxx
 *
 * @brief Provides methods to manipulate volume identifiers.
 *
 * TkrVolumeIdentifier inherits from idents::VolumeIdentifier.  It contains the
 * same information (no extra member variables), but methods to retrieve
 * information from a volume identifier.  This enables users to retrieve i.e.
 * the tray id without actually knowing where this tray id is stored.
 * Additionally, there are some extra constructors, and copy constructor and
 * assignment..
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/VolumeIdentifier.cxx,v 1.2 2004/02/24 13:57:32 kuss Exp $
 */

#include "TkrVolumeIdentifier.h"


TkrVolumeIdentifier::TkrVolumeIdentifier(const int& tower, const int& layer,
                                         const int& view) {
    // Purpose and Method: Constructs a truncated volume identifier.  Ladder and
    //                     wafer information are not filled.
    // Inputs: a tower id, a biLayer number, and the view.
    // Outputs: none
    // Dependecies: none
    // Restrictions and Caveats: none

    // We want only the plane (not the wafer) identifier, so only the first 7
    // elements are included.
    init(0,0);
    append(0);  // LAT: 0
    const idents::TowerId towerId(tower);
    append(towerId.iy());
    append(towerId.ix());
    append(1);  // TKR: 1
    int tray, botTop;
    // make up the tray and botTop from bilayer and view
    if ( layer%2 == 0 ) {
        tray = layer + 1 - view;
        botTop = view;
    }
    else {
        tray = layer + view;
        botTop = 1 - view;
    }
    append(tray);
    append(view);
    append(botTop);
}


TkrVolumeIdentifier& TkrVolumeIdentifier::operator=(
                                           const idents::VolumeIdentifier& id) {
    // Purpose and Method: Assigns an idents::VolumeIdentifier to a
    //                     TkrVolumeIdentifier.
    // Inputs: an idents::VolumeIdentifier
    // Outputs: a TkrVolumeIdentifier
    // Dependecies: none
    // Restrictions and Caveats: none

    if ( this != &id )
        init(id.getValue(), id.size());
    return *this;
}


idents::VolumeIdentifier TkrVolumeIdentifier::getPlaneId() const {
    // Purpose and Method: Removes ladder and wafer information from the
    //                     volume identifier.  The resulting volId describes a
    //                     plane.
    // Inputs: none
    // Outputs: an idents::VolumeIdentifier
    // Dependecies: none
    // Restrictions and Caveats: if the volume identifier has a size smaller
    //                           than 7 (i.e. doesn't contain a plane id), the
    //                           output is shorter than the calling routine
    //                           might expect.

    idents::VolumeIdentifier planeId;
    const int imax = size() < 7 ? size() : 7;
    for ( int i=0; i<imax; ++i )
        planeId.append(operator[](i));
    return planeId;
}
