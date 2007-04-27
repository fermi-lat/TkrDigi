#ifndef InitCurrent_h
#define InitCurrent_h 1

//#                                                              
//#  April 2007 modified in order to return charge

#include "GaudiKernel/StatusCode.h"


#include "GaudiKernel/StatusCode.h"
#include <string>

class InitCurrent
{
public:
    
  InitCurrent();
  ~InitCurrent();
    
  // definizione dei metodi
  // double OpenCurrent(int);
  StatusCode OpenCurrent(std::string);    
  void GetCharge(double*);
  inline double* GetCh(){return XXcharge;}
    
    
private:
    
  static const int ndim = 250000;
  static const int Nbin = 50;
  static const int N    = 12;

  double* CURR;
  double XXcharge[N];
  double CurrPar[N];
  double X[Nbin], Z[Nbin]; //nbin

  int ID1, ID2, IDD;
  char fileName[80];  
  int k1, k2, P2, P3;
  int j;
  double Xpos;
  double Zpos;

  static const double Xmin = -(0.114);
  static const double Xmax = 0.114;
  static const double Zmin = 0.;
  static const double Zmax = 0.4;
  double DeltaX;
  double DeltaZ;

  InitCurrent* OpenCurr;
  double pippo;
     
};
#endif
