//##################################################################
//#  $Id:       Cluster.cxx                                        #
//#                                                                #
//#  Module for cluster parameter determination                    #
//#                                                                #
//#  INPUT: Xin Xout of track in silicon layer                     #
//#  OUTPUT : PosX --> cluster position                            #
//#           Qclus -->  charge deposed for cluster                #
//#                                                                #
//#                                    Monica Brigida              #
//#                                                                #
//#  23-Aug-02 change to Tower, Layer, View   LSR                  #
//##################################################################

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/config/iostream.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "../SiStripList.h"

#include "Cluster.h"
#include "TMath.h"

int    Cluster::nhitmax      = 15000;
double Cluster::SiPitch      = SiStripList::si_strip_pitch();;

// class constructor

Cluster::Cluster()
{ 
  NumberOfCluster = 0;
}

Cluster::~Cluster()
{}


void Cluster::Clean()
{
  NumberOfCluster = 0;
}

// method to associate to a position the two near strip
void  Cluster::xtoid(float xpos, int &id1, int &id2)
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
        id1 = id;
        if(dist1 > dist2) {
            id2 = id1 - 1;
        }
        else{
            id2 = id1 + 1;

        }
    }
}

// Xi e Xf in mm

void Cluster::SetCluster(HepPoint3D Xi, HepPoint3D Xf, double edepos) 
 {
   t    = 0.; // track length  
   ierr = 0;
   rr   = 0.;
   Qtot = 0;

   HepVector3D segment = Xf-Xi;
   double      Len     = segment.mag();
   HepVector3D dir     = segment.unit();

   // Bari 1
   PairNumber       = static_cast<int>((edepos/CLHEP::eV)/3.6); 
   NumberOfCluster  = static_cast<int>((Len/0.04)*5)+1; // 0.04 10 clus verticali
   if ( NumberOfCluster > nhitmax ) { NumberOfCluster = nhitmax; }
   if ( NumberOfCluster <= 0 ){ NumberOfCluster = 1; }
   
   qqq = (PairNumber / NumberOfCluster);
   for( int i = 0; i < NumberOfCluster; i++){	
     t      = CLHEP::RandFlat::shoot()*Len;
     XClust = Xi + t*dir;
     cr     = CLHEP::RandGauss::shoot(0.,1.)*(sqrt(0.1*qqq)); 
     QClust = qqq + cr;
     SetSingleClusterCoordinates(XClust, i);
     SetSingleClusterCharge(QClust, i);       
   } 
} 

