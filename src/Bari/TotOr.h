/**
 * @class TotOr
 *
 * @brief Class to manipulate dynamic arrays used in the digitization process
 * performing Map_OR on each plane
 *
 * @author  M. Brigida, N. Giglietto (original authors)
 * @author  M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/Bari/TotOr.h,v 1.1 2004/02/24 13:57:33 kuss Exp $
 */


#ifndef TotOr_h
#define TotOr_h 1

#include "../SiPlaneMapContainer.h"


class TotOr {

 public:

    TotOr(){}
    ~TotOr(){}

    SiPlaneMapContainer::SiPlaneMap getList() const { return m_list; }
    unsigned int size() const { return m_list.size(); }

    /**
     * adds a strip to m_list, either to an existing SiStripList, or as a new
     * one.
     * @param 1  volume identifier
     * @param 2  strip id
     * @param 3  vector of associated McPositionHits
     * @param 4  start ToT for this strip
     * @param 5  stop ToT for this strip
     * @param 6  deposited energy
     */
    void add(const idents::VolumeIdentifier, const int,
             const SiStripList::hitList&, const int, const int, const double);

 private:

    /// map of SiStripList and volume identifiers
    SiPlaneMapContainer::SiPlaneMap m_list;

};

#endif
