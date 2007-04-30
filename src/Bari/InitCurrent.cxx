//########################################################################
//#  $Id:           InitCurrent.cc                                       #
//#                                                                      #
//#  to download the current for each cluster                            #
//# INPUT = cluster position XX                                          #
//#                                                                      #
//#                                                                      #
//#  23-Aug-02 change to return error code   LSR                         #
//########################################################################

#include <iostream>
#include <fstream>
#include "CLHEP/Random/Randomize.h"
#include "CLHEP/config/iostream.h"

#include "InitCurrent.h"

const double InitCurrent::Xmin = -0.114;
const double InitCurrent::Xmax = 0.114;
const double InitCurrent::Zmin = 0.;
const double InitCurrent::Zmax = 0.4;

InitCurrent::InitCurrent()
{
    // trying to eliminate prospective memory leaks.  CURR was nowhere deleted.
    // I define it with dimension 250000 directly in the header file.
    // Anyway, one should define dimensions in constants and use these in the
    // code, instead of carrying numbers around in the code.

    // But this seems to add bit chunks of memory to the program, even when Bari
    // is not being used, because FluxSvc and RandomSvc instantiate these things
    // and apparently don't close them!

    CURR = 0;
}

InitCurrent::~InitCurrent()
{
    if (CURR) delete CURR;
}

StatusCode InitCurrent::OpenCurrent(std::string currents)
{
    StatusCode sc = StatusCode::SUCCESS;
    double tmp = 0.;
    int nval   = 0; 

    if (!CURR) CURR = new double[ndim];
    for ( int jj = 0; jj < N; jj++ )
        XXcharge[jj] = 0.0;
    for ( int tt = 0; tt < ndim; tt++ )
        CURR[tt] = 0.0; 

    
    std::ifstream fin(currents.c_str()); 
    if (!fin) {
        return StatusCode::FAILURE;
    }
    for(int j=0; j<2500; j++){ 
        fin >> ID1 >> ID2 ;
	// now read nval elements not null
	fin >> nval;
        for(int k=0; k<nval; k++){
            fin >> CurrPar[k];
            int ii = j*nval + k;
            if(ii  > ndim-1 )std::cout<<"OpenCurr out of range ***"<< ii <<std::endl;		
            tmp = CurrPar[k];
            CURR[ii] = tmp;
        }// loop k closed	
    } // loop j closed
    fin.close();
    return sc;
} 


void InitCurrent::GetCharge(double* XX)
{  
  k1 = 0;
  k2 = k1 + Nbin;
  Xpos = XX[0];
  Zpos = XX[1];
  DeltaX = (Xmax - Xmin)/Nbin;
  DeltaZ = (Zmax - Zmin)/Nbin;

  for (j = 0; j < Nbin; j++){
    X[j] = Xmin + DeltaX * (j + 0.5);
    Z[j] = Zmin + DeltaZ * (j + 0.5);
  }
 
  for(j = 0; j < N ; j++){XXcharge[j] = 0.;}

  // check if ix iy are in the right limits
  int ix = int((Xpos - Xmin)/DeltaX - 0.5); // lower x bin index
  int iy = int((Zpos - Zmin)/DeltaZ - 0.5); // lower z bin indx
  if(ix < 0 || ix >= Nbin || iy < 0 || iy >= Nbin) {  // protezione
    // std::cout<< "WARNING " << ix << " " << iy << std::endl;
    goto esci; 
  }
  P3 = (iy + (ix * Nbin));
  P2 = ix * Nbin + iy;
  if(P2>2499){
    std::cout<<"WARNING P2 greater than  limit!!! "<<P2<<" " <<ix<<" "<<iy<<std::endl;
    goto esci;
  }
  for(j = 0; j < N ; j++){XXcharge[j] = CURR[P2*12+j];}
 esci:;
}

