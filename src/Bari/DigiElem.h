/**
 * @class DigiElem
 *
 * @brief DigiElem stores strip information for the digitization process
 *
 * authors: M. Brigida, N. Giglietto, M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/DigiElem.h,v 1.2 2004/03/18 11:35:08 ngigliet Exp $
 */

#ifndef DIGIELEM_H
#define DIGIELEM_H

#include "../TkrVolumeIdentifier.h"

#include "Event/MonteCarlo/McPositionHit.h"

#include <vector>


class DigiElem {

 public:

  DigiElem();
    /**
     * Constructor for a DigiElem
     * @param 1  volume identifier
     * @param 2  strip id
     * @param 3  array of currents
     * @param 4  pointer to associated McPositionHit
     */
    DigiElem(const idents::VolumeIdentifier, const int, const double*,
             Event::McPositionHit*);
    ~DigiElem(){}

    typedef std::vector<Event::McPositionHit*> hitList;
    static const int Nbin = 1; 
    /// adds more current to each element of m_Ic
    void add(const double* I) { for ( int i=0; i<Nbin; ++i ) m_Ic[i] += I[i]; }
     /// adds a McPositionHit
    void add(Event::McPositionHit*);
    /// adds a vector ofMcPositionHits
    void add(const hitList&);

    idents::VolumeIdentifier getVolId() const { return m_volId; }
    int getTower()             const { return m_volId.getTower().id(); }
    int getLayer()             const { return m_volId.getLayer(); }
    int getView()              const { return m_volId.getView(); }
    int getStrip()             const { return m_strip; }
    const hitList& getHits()   const { return m_hits; }
    const double* getCurrent() const { return m_Ic; }
 
 private:

    /// vector of McPositionHits
    hitList m_hits;
    /**
     * use a TkrUtil volume identifier, which allows the use of get methods
     * (e.g. getView()).
     */
    TkrVolumeIdentifier m_volId;
    /// strip id;
    int m_strip;
    /// array of currents
    double m_Ic[Nbin];

};

#endif
