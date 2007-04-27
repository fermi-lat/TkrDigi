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

  void xtoid(float,int&,int&);
  void setClusterPropagator(HepPoint3D*,double*,int,idents::VolumeIdentifier,
  		      Event::McPositionHit*); 
  void setMapCurr(CurrOr* m) { m_mapCurr = m; }
  void setOpenCurr(InitCurrent* m) { m_current = m; }
     
 private:
  CurrOr* m_mapCurr;
  InitCurrent* m_current;  
  
  int ID1, ID[5], Nflag;
  int Id1, Id2, Id11, Id22;
  double* Icurr;
  double* Icurr1;
  double Ic[5];
  int j, nt, jj;

  double XV[2], XVel[2], XVhole[2];
  double Qclu;
  double XX, XX0, ZZ0;                    // in mm
  double SigmaEl, SigmaHole;
  double Rphi;
  int nflag;
};

#endif








