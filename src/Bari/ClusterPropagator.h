//########################################################################
//#  $Id:     ClusterPropagator.h                                        #
//#               Method to propagate clusters towards strips            #
//#                                                                      #
//#                                     authors: M.Brigida, N.Giglietto  #
//#                                                                      #
//#  23-Aug-02 changed y to x; removed Xstrip, AllCurr -- LSR            #
//########################################################################


#ifndef ClusterPropagator_h
#define ClusterPropagator_h 1

#include "CLHEP/Geometry/Point3D.h"
#include "idents/VolumeIdentifier.h"
#include "Event/MonteCarlo/McPositionHit.h"

#include "InitCurrent.h"
#include "CurrOr.h"

class ClusterPropagator {
 public:
    ClusterPropagator(){}
    ~ClusterPropagator(){}

    void clean(){}  // clean doesn't do anything!
    void xtoid(float,int&,int&);
    void setClusterPropagator(HepPoint3D*,double*,int,idents::VolumeIdentifier,
			      Event::McPositionHit*);
    void setMapCurr(CurrOr* m) { m_mapCurr = m; }
    void setOpenCurr(InitCurrent* m) { m_current = m; }
     
 private:
    CurrOr* m_mapCurr;
    InitCurrent* m_current;  
};

#endif








