
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
// TU: Hacks for CLHEP 1.9.2.2 and beyond
#ifndef HepPoint3D
typedef HepGeom::Point3D<double> HepPoint3D;
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
    void SetCluster(/*int, int, int,*/ HepPoint3D, HepPoint3D, double);
    void Clean();  
    
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
    
};

#endif
