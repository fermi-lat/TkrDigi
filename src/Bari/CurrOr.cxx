/**
 * @file CurrOr.cxx
 *
 * @brief Class to maninpulate dynamic arrays used in the digitization process
 * performing Map_OR on each plane.
 *
 * @authors  M. Brigida, M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/CurrOr.cxx,v 1.1 2004/02/27 10:14:14 kuss Exp $
 */

#include "CurrOr.h"

#include <iostream>




void CurrOr::print() const {
    for ( DigiElemCol::const_iterator it=m_list.begin(); it!=m_list.end(); ++it)
	std::cout << " Tower " << it->getTower()
		  << " Layer " << it->getLayer()
		  << " View "  << it->getView()
		  << " Strip " << it->getStrip()
		  << std::endl;
}


// private functions

CurrOr::DigiElemCol::iterator CurrOr::getPos(const idents::VolumeIdentifier
                                             volId, const int strip) {
    // Purpose and Method: searches for a given strip in the list of DigiElems.
    //                     If found, an iterator to that DigiElem is returned,
    //                     otherwise end().
    // Inputs: volume identifier and strip id
    // Outputs: a DigiElemCol::iterator
    // Dependencies: none
    // Restrictions and Caveats: none

    DigiElemCol::iterator it;
    for ( it=m_list.begin(); it!=m_list.end(); ++it )
        if ( it->getStrip() == strip
             && it->getVolId().getValue() == volId.getValue() )
            return it;
    return it;
}


void CurrOr::add(const DigiElem* d) {
    // Purpose and Method: adds a DigiElem to the list.  If it already exists,
    //                     just add the current and McPositionHits.
    // Inputs: pointer to a DigiElem
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: none

    DigiElemCol::iterator it = getPos(d->getVolId(), d->getStrip());
    if ( it == m_list.end() )            // new plane to add
    	m_list.push_back(*d);
    else { // add to existing plane
      it->add(d->getCurrent());
      it->add(d->getHits());
    }
}
void CurrOr::addnew(const DigiElem* d) {
    // Purpose and Method: adds a DigiElem to the list.  If it already exists,
    //                     just add the current and McPositionHits.
    // Inputs: pointer to a DigiElem
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: none

      DigiElemCol::iterator it = getPos(d->getVolId(), d->getStrip());
      if ( it != m_list.end() ){
       it->add(d->getCurrent());
       it->add(d->getHits());
      } 
      m_list.push_back(*d); //add in every case
}

void CurrOr::add(const idents::VolumeIdentifier volId, const int strip,
                 const double* I, Event::McPositionHit* pHit) {
    // Purpose and Method: builds a DigiElem and calls add(DigiElem)
    // Inputs: volume identifier, strip id, array of currents, and a pointer to
    // a McPositionHit.
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: none

    DigiElem temp(volId, strip, I, pHit);
    add(&temp);
}


void CurrOr::add(const DigiElemCol& l) {
    // Purpose and Method: iterates over a vector of DigiElems and calls
    // add(DigiElem) for each of them.
    // Inputs: a vector of DigiElems
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: none
  for ( DigiElemCol::const_iterator it=l.begin(); it!=l.end(); ++it )
    addnew(&*it);
}
