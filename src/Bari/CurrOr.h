/**
 * @class CurrOr
 *
 * @brief Class to maninpulate dynamic arrays used in the digitization process
 * performing Map_OR on each plane.
 *
 * @authors  M. Brigida, M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/CurrOr.h,v 1.1 2004/02/27 10:14:14 kuss Exp $
 */

#ifndef CURROR_H
#define CURROR_H

#include "DigiElem.h"

#include <vector>


class CurrOr {

 public:

    CurrOr(){}
    ~CurrOr(){}

    typedef std::vector<DigiElem> DigiElemCol;

    const DigiElemCol& getList() const { return m_list; }
    unsigned int size()          const { return m_list.size(); }
    void clear() { m_list.clear(); }

    /// adds a DigiElem
    void add(const DigiElem*);
    void addnew(const DigiElem*);
    /**
     * adds a DigiElem via it's components
     * @param 1  volume identifier
     * @param 2  strip id
     * @param 3  array of currents 
     * @param 4  pointer to a McPositionHit
     */
    void add(const idents::VolumeIdentifier, const int, const double*,
             Event::McPositionHit*);

    /// adds a vector of DigiElem
    void add(const DigiElemCol&);

    void print() const;

 private:

    /// list of DigiElems
    DigiElemCol m_list;

    /**
     * getPos searches for a given strip in the list of DigiElems
     * @param 1  volume identifier
     * @param 2  strip id
     * @return   a DigiElemCol::iterator, pointing to the DigiElem representing
     *           the strip, or m_list.end().
     */
    DigiElemCol::iterator getPos(const idents::VolumeIdentifier, const int);

};

#endif

