#ifndef InitCurrent_h
#define InitCurrent_h 1

//#                                                              
//#  23-Aug-02 modify to return error code   LSR

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
    
    void GetCurrent(double*);
    inline double* Get(){return XXcurr;}
    
    
private:
    
    //#define Pitch = 0.2; // millimetr
    double Pitch;
    //#define Thick = 0.4; // millimetri
    double Thick;
    //#define nbin = 50;
    int nbin;
  static const int ndim=250000;

    double CURR[ndim];
    double XXcurr[100];
    int ID1, ID2, IDD;
    double CurrPar[100];
    char fileName[80];  
    double CurrentField[100];
    double Vaa[100];
    InitCurrent* OpenCurr;
    double pippo;
    
    
    
};
#endif
