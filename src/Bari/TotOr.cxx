/**
 * @file TotOr.cxx
 *
 * @brief Class to manipulate dynamic arrays used in the digitization process
 * performing Map_OR on each plane
 *
 * @author  M. Brigida, N. Giglietto (original authors)
 * @author  M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/Bari/TotOr.cxx,v 1.1 2004/02/24 13:57:33 kuss Exp $
 */

#include "TotOr.h"


void TotOr::add(const idents::VolumeIdentifier volId, const int strip,
                const SiStripList::hitList& hits, const int t1, const int t2,
                const double energy) {
    // Purpose and Method: adds a strip to m_list.  If there is no SiStripList
    //                     associated with the volume identifier, create a new
    //                     SiStripList, otherwise append to the existing one.
    // Inputs: volume identifier, strip id, vector of associated McPositionHits,
    //         start and stop ToT for this strip, and the deposited energy.
    // Dependencies: none

    SiPlaneMapContainer::SiPlaneMap::iterator it = m_list.find(volId);

    if ( it == m_list.end() ) { // new plane to add
	m_list[volId] = new SiStripList;
        // refresh the iterator
	it = m_list.find(volId);
    }

    // add the strip
    it->second->addStrip(strip, energy, &hits, t1, t2);
}
