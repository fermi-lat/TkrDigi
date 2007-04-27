/**
 * @file DigiElem.cxx
 *
 * @brief DigiElem stores strip information for the digitization process
 *
 * authors: M. Brigida, N. Giglietto, M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/DigiElem.cxx,v 1.2 2004/03/18 11:35:08 ngigliet Exp $
 */

#include "DigiElem.h"


DigiElem::DigiElem(){
  for ( int i=0; i<Nbin; ++i )m_Ic[i] = 0;
}



DigiElem::DigiElem(const idents::VolumeIdentifier volId, const int strip,
                   const double* Ic,
		   Event::McPositionHit* hit) {
    // Purpose and Method: Constructs a DigiElem
    // Inputs: volume identifier, strip id, array of currents, pointer to
    //         associated McPositionHit
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: the dimension of m_Ic is hard-coded
 
    m_volId = volId;
    m_strip = strip;
    for ( int i=0; i<Nbin; i++ )
    m_Ic[i] = Ic[i];
    m_hits.push_back(hit);
}





void DigiElem::add(Event::McPositionHit* hit) {
    // Purpose and Method: adds a McPositionHit, if it is not in the hitList
    // Inputs: pointer to a McPositionHit
    // Outputs: none
    // Dependencies: none

    for ( hitList::const_iterator it=m_hits.begin(); it!=m_hits.end(); ++it )
      if ( *it == hit )
	return;
    m_hits.push_back(hit);
}


void DigiElem::add(const hitList& l) {
    // Purpose and Method: adds a vector of McPositionHits, by iterating through
    //                     the list and calling add(Event::McPositionHit*).
    // Inputs: a vector of McPositionHits
    // Outputs: none
    // Dependencies: none

    for ( hitList::const_iterator it=l.begin(); it!=l.end(); ++it )
	add(*it);
}
