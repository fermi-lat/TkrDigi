
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

#ifndef Cluster_h
#define Cluster_h 1

#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"
// TU: Hacks for CLHEP 1.9.2.2 and beyond
#ifndef HepPoint3D
typedef HepGeom::Point3D<double> HepPoint3D;
typedef HepGeom::Vector3D<double> HepVector3D;
#endif

class Cluster      //class declaration
{
public:  
    
    Cluster();
    ~Cluster();


    
    HepPoint3D* GetClusCoord() { return XCluster;} 
    double* GetClusCharge()    { return QCluster;} 
    int* GetClusID()           { return IDCluster;}
    int GetNumberOfClusters()  { return NumberOfCluster;}  
    int GetTower()             { return Tower;}
    int GetLayer()             { return Layer;}
    int GetView()              { return View;}
    
    static double SiPitch; 
    inline double GetPitch(){return SiPitch;}
    void SetCluster(HepPoint3D, HepPoint3D, double);
    void Clean();  
    void xtoid(float, int&, int&);

    double* GetStripCharge()    { return QStrip;} 
    int* GetStripID()           { return IDStrip;}
    int GetNumberOfStrip()  { return NumberOfStrip;}  

private:
    
    inline void SetSingleClusterCoordinates(HepPoint3D XClust, int Number)
    {XCluster[Number] = XClust;};
    inline void SetSingleClusterCharge(double QCharge, int Number)
    {QCluster[Number] = QCharge;};
    inline void SetSingleClusterID(int ID, int Number)
    {IDCluster[Number]= ID;};
    
    
    int View; // 0 X 1 Y
    int Layer;
    int Tower;
    static int nhitmax; //15000
    HepPoint3D XCluster[15000];
    double QCluster [15000];
    int IDCluster[15000]; 
    int NumberOfCluster; // MAX 5000
    double Frac;
    double QStrip[5000];
    int IDStrip[5000]; 

  int NumberOfStrip; // MAX 5000
  
  double Etot;
  int Id1, Id2;
  int ID1, ID2, ID11, ID21;
  int NumFiredStrip;
  int PairNumber;
  double EClus;
  double len;
  double L;
  double dist;
  double charge;

  double qqq;

  double t; // track length  
  HepVector3D XPos;
  HepPoint3D XClust;
  double QClust;
  int ierr;
   
  double rr;
  double Qtot;
  double xl;
  double  cr;

  inline void clear(){
    //NG vector dimension is 5000
    for(int i=0; i< 5000; i++){
      QStrip[i]= 0.;
      IDStrip[i] = 0;
    }
  };
    
  inline void SetSingleStripCharge(double QCharge, int Number)
  { QStrip[Number] = QCharge;};

  inline void SetStripID(int ID, int Number)
  { IDStrip[Number] = ID; };

};

#endif
