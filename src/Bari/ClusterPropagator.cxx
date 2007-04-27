//########################################################################
//#  $Id:     ClusterPropagator.cc                                       #
//#               Method to propagate clusters towards strips            #
//#                                                                      #    
//#                                     authors: M.Brigida, N.Giglietto  #
//#                                                                      #
//#  23-Aug-02 changed y to x; removed Xstrip, AllCurr -- LSR            #
//########################################################################

#include "CLHEP/Random/Randomize.h"
#include "CLHEP/config/iostream.h"
#include "CLHEP/Random/RandGauss.h"
#include "TMath.h"
#include "ClusterPropagator.h"
#include "../SiStripList.h"


// method to associate to a position the two near strip
void  ClusterPropagator::xtoid(float xpos, int &id1, int &id2)
{
  // if out of silicon wafer  id1=id2=-1
  double XX1 = 0., XX2 = 0.;
  double dist1, dist2;
  id1 = -1;
  id2 = -1;
  unsigned int id = SiStripList::stripId(xpos);
  if (id != 65535) {  // no id found (see SiliconPlaneGeometry)
    unsigned int yy = id +1, jj = id -1;
    XX1 = SiStripList::calculateBin(yy);
    XX2 = SiStripList::calculateBin(jj);
    dist1 = fabs(xpos*1000. - XX1*1000.);
    dist2 = fabs(xpos*1000. - XX2*1000.);
    //
    id1 = id;
    if(dist1 > dist2){id2 = id1 - 1;}
    else{id2 = id1 + 1;}
  }
}

//##################################################################
// propagate each cluster of the track inside the silicon element
// and evaluate the currents induced into the closest strips 
// currents are in MICROAMPS <<<=========================
// Authors Brigida, Giglietto
//###################################################################


void ClusterPropagator::setClusterPropagator(HepPoint3D* XClus, double* QClus,int NClus, 
					      idents::VolumeIdentifier volId,
					      Event::McPositionHit* pHit)
{ 
    // the coordinates of the track are in the *local* coordinate system... 
    // in this system, x always is the measurement direction
    
  Qclu      = 0.;
  XX        = 0.;
  XX0       = 0.;                    // in mm
  ZZ0       = -0.2;
  SigmaEl   = 0.;
  SigmaHole = 0.;
  Rphi      =  0;
  nflag     = -1;

  for (j = 0; j< NClus; j++) {              // loop over cluster
    XX = XClus[j].x();                      // in the local frame, x is the measured coordinate -- LSR
    xtoid(XX, Id1, Id2);                   // ID1 main strip fired
    ID1 = Id1;
    if(ID1<0) {continue;}                   // goto next cluster
    XX0   = SiStripList::calculateBin(ID1);
    Qclu  = QClus[j];                       //pair number
    XV[0] = XX - XX0;
    XV[1] = XClus[j].z() - ZZ0;            // mm respect to wafer SR
    m_current->GetCharge(XV);               // GET charge
    Icurr     = m_current->GetCh();   
    SigmaEl   = Icurr[5]  * 10.*(CLHEP::RandGauss::shoot(0.,1.));
    SigmaHole = Icurr[11] * 10.*(CLHEP::RandGauss::shoot(0.,1.));

  bb:;
    Rphi       =  (CLHEP::RandFlat::shoot(0.,360.));
    XVel[0] = XClus[j].x() + (TMath::Cos(Rphi))*SigmaEl;
    XVel[1] = XClus[j].z() + (TMath::Sin(Rphi))*SigmaEl;
    
    xtoid(XVel[0], Id1,Id2);           // Id1 New main strip fired by electron 
    if(Id1 < 0) goto bb;
    nflag = int(Id1/384);
    XX0   = SiStripList::calculateBin(Id1);
    XVel[0] = XVel[0] - XX0;
    XVel[1] = XVel[1] - ZZ0;
    m_current->GetCharge(XVel);        //  GET charge
    Icurr1     = m_current->GetCh();
    
    ID[0] = Id1 - 2;                   // check in the vol
    ID[1] = Id1 - 1;
    ID[2] = Id1;
    ID[3] = Id1 + 1;
    ID[4] = Id1 + 2;

    for (jj = 0; jj < 5; jj++){
      Nflag = int(ID[jj]/384);
      Icurr[0] = Icurr1[jj]*Qclu;
      if(Nflag == nflag) m_mapCurr->add(volId, ID[jj], Icurr , pHit);
    }

  exit:;
    Rphi       =  (CLHEP::RandFlat::shoot(0.,360.));
    XVhole[0] = XClus[j].x() + (TMath::Cos(Rphi))*SigmaHole;
    XVhole[1] = XClus[j].z() + (TMath::Sin(Rphi))*SigmaHole;
    
    xtoid(XVhole[0], Id11,Id22);      // Id11 New main strip fired by hole
    if(Id11 < 0) goto exit;           // check if ID in the vol
    
    nflag = int(Id11/384);
    XX0   = SiStripList::calculateBin(Id11);
    XVhole[0] = XVhole[0] - XX0;
    XVhole[1] = XVhole[1] - ZZ0;
    m_current->GetCharge(XVhole);     // GET charge
    Icurr1     = m_current->GetCh();

    ID[0] = Id11 - 2;                // check strip in the vol
    ID[1] = Id11 - 1;
    ID[2] = Id11;
    ID[3] = Id11 + 1;
    ID[4] = Id11 + 2;

    for (jj = 0; jj < 5; jj++){
      Nflag = int(ID[jj]/384);
      Icurr[0] = Icurr1[jj+6]*Qclu;
      if(Nflag == nflag) m_mapCurr->add(volId, ID[jj], Icurr , pHit);
    }
  }
  // END CLUSTER LOOP
}

