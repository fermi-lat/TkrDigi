/**
 * @file Digi.cxx
 *
 * @brief Digi stores strip information for the digitization process
 *
 * authors: M. Brigida, N. Giglietto, M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/Digi.cxx,v 1.2 2004/03/18 11:35:08 ngigliet Exp $
 */

#include "Digi.h"


Digi::Digi(){
}


Digi::Digi(const idents::VolumeIdentifier volId, const int strip, 
	   Event::McPositionHit* hit, const double tim1, const double tim2)
 {
    // Purpose and Method: Constructs a Digi
    // Inputs: volume identifier, strip id, array of currents, pointer to
    //         associated McPositionHit
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: the dimension of m_Ic is hard-coded
 
   m_volId = volId;
   m_strip.clear();
   m_strip.push_back(strip);
   m_hits.push_back(hit);
   m_tim1 = tim1;
   m_tim2 = tim2;
}

void Digi::set(int& T,int& L, int& V, int& S, double& T1, double& T2){
  m_tower = T;
  m_layer = L;
  m_view = V;
  m_strip.push_back(S);
  m_tim1 = T1;
  m_tim2 = T2;

}


void Digi::add(Event::McPositionHit* hit) {
    // Purpose and Method: adds a McPositionHit, if it is not in the hitList
    // Inputs: pointer to a McPositionHit
    // Outputs: none
    // Dependencies: none

    for ( hitList::const_iterator it=m_hits.begin(); it!=m_hits.end(); ++it )
	if ( *it == hit )
	    return;
    m_hits.push_back(hit);
}


void Digi::add(const hitList& l) {
    // Purpose and Method: adds a vector of McPositionHits, by iterating through
    //                     the list and calling add(Event::McPositionHit*).
    // Inputs: a vector of McPositionHits
    // Outputs: none
    // Dependencies: none

    for ( hitList::const_iterator it=l.begin(); it!=l.end(); ++it )
	add(*it);
}
