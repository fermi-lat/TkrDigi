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

int    Cluster::nhitmax      = 15000;
double Cluster::SiPitch      = SiStripList::si_strip_pitch();;

// class constructor

Cluster::Cluster()
{ 
     NumberOfCluster = 0;
}

Cluster::~Cluster()
{ 
}


void Cluster::Clean()
{
    NumberOfCluster = 0;
}

// Xi e Xf in mm
void Cluster::SetCluster(HepPoint3D Xi, HepPoint3D Xf, double edepos) 
{
    int PairNumber = static_cast<int>((edepos/eV)/3.6); //# e-h (energy from keV --> eV)
    
    // random numbers  
    double cr = RandGauss::shoot(0.,1.)*44. + 1664.; // mean and sigma from full simulation 
    // mean hit number in 400 micron

    HepVector3D segment = Xf-Xi;
    double      Len     = segment.mag();
    HepVector3D dir     = segment.unit();

    NumberOfCluster = static_cast<int>((cr/0.4)/100.); // number of cluster 
    double xl = Len / NumberOfCluster; 
    if (NumberOfCluster == 0){
        std::cout << "cluster " << NumberOfCluster << " " <<
            cr << " " <<  xl << "\n";
        NumberOfCluster=1;
    }
    
    int qqq = (PairNumber / NumberOfCluster);
    
    double t = 0.; // track length  
    
    HepPoint3D XClust;
    double QClust;
    int ierr = 0;

    double rr=0.;
    double Qtot = 0;
    for( int i = 0; i < NumberOfCluster; i++)
    {	
        if (i >= nhitmax) break; 
        ierr = 0;
  		rr=RandFlat::shoot()*Len;    // positions uniformely distributed along the track
        t = rr;

        HepPoint3D XClust = Xi + t*dir;
        
        QClust = qqq * 1.6e-7; //pCoulomb
        Qtot += QClust;
        SetSingleClusterCoordinates(XClust, i);
        SetSingleClusterCharge(QClust, i);       
    }
}
