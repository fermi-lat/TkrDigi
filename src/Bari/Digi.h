/**
 * @class Digi
 *
 * @brief Digi stores strip information for the digitization process
 *
 * authors: M. Brigida, N. Giglietto, M. Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/Bari/Digi.h,v 1.2 2004/03/18 11:35:08 ngigliet Exp $
 */

#ifndef DIGI_H
#define DIGI_H

#include "../TkrVolumeIdentifier.h"

#include "Event/MonteCarlo/McPositionHit.h"
#include "../SiStripList.h"
#include <vector>
#include "../SiPlaneMapContainer.h"

class Digi{

 public:

  Digi();
    /**
     * Constructor for a Digi
     * @param 1  volume identifier
     * @param 2  strip id
     * @param 4  pointer to associated McPositionHit
     */
  Digi(const idents::VolumeIdentifier, const int strip,Event::McPositionHit*, const double , const double);
    ~Digi(){}

  typedef std::vector<Event::McPositionHit*> hitList;
  //typedef std::vector<SiPlaneMapContainer::SiPlaneMap> StripList;
    /// adds a McPositionHit
  void add(Event::McPositionHit*);
    /// adds a vector ofMcPositionHits
  void add(const hitList&);

    idents::VolumeIdentifier getVolId() const { return m_volId; }
  //    int Tower()             const { return m_volId.getTower().id(); }
  // int Layer()             const { return m_volId.getLayer(); }
  // int View()              const { return m_volId.getView(); }
    int Tower()             const { return m_tower; }
    int Layer()             const { return m_layer; }
    int View()              const { return m_view; }
  std::vector<int> Strip()  const { return m_strip; }
    double Tim1()              const { return m_tim1; }
    double Tim2()              const { return m_tim2; }
    const hitList& getHits()   const { return m_hits; }
  
  void set(int&,int&, int&, int&, double&, double&);
  void set_t1(double& t1){m_tim1 = t1;}
  void set_t2(double t2){m_tim2 = t2;}
  void clear(){;}
  //  std::vector<int> StripList(){return m_strip;}
 
  void push_back(int &Sstrip){m_strip.push_back(Sstrip);}
  int size(){return m_strip.size();}

 private:

    /// vector of McPositionHits
  hitList m_hits;
  // StripList m_strip;
    /**
     * use a TkrUtil volume identifier, which allows the use of get methods
     * (e.g. getView()).
     */
  TkrVolumeIdentifier m_volId;
  std::vector<int> m_strip;
  double m_tim1;
  double m_tim2;
  int m_tower;
  int m_layer;
  int m_view;
};

#endif
