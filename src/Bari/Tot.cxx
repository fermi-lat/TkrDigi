///////////////////////////////////
// Simulate the TOT logic
// Author N.GIGLIETTO
///////////////////////////////////////////

#include "CLHEP/Random/RandGauss.h"
#include "Tot.h"

Tot::Tot(){
}
Tot::~Tot(){
}

void Tot::Put(double* varray){
  // Count the maximum number of consecutive bins over threshold
  // the input array must be 10000 bins voltage signal in mV
  // Author N.Giglietto
  int ndim = 10000;
  double treshold = RandGauss::shoot(160.,7.); // one for each strip    <==== microV 
  //std::cout<<"threshold = "<<treshold<<std::endl;
  // reset all variables
  int lcount = 0;
  lcountmax = 0;
  int index = -1;
  llasttime = -1;
  lfirsttime = -1;
  // varray is in microVOLT <<---------------------------------
  for (int l = 0; l<ndim; l++){
    if(varray[l]/1000.>treshold){
    // std::cout << varray[l]/1000. << "\n";
      if(index <0) {
	// if the previous bin was fired counts
	//	  std::cout<<"index < 0 "<< l<<std::endl;
	   lcount = l;
	   lcountmax = l;
	   index = 1;
	   lfirsttime = l;
      }
    }
    else{
      if(index>0){ // is going under threshold
		 // std::cout<<"index > 0 "<< l<<std::endl;
	   lcountmax = l-lcount; // gives the time over threshold in ns
	   llasttime = l;
	   break;
      }
    }
  }
  if(lfirsttime != -1){
    if(llasttime == -1) {llasttime=ndim+1;} // overflow signal
  }
}
