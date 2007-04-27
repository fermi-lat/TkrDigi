//////////////////////////////////
//  class TkrTrigger
//  author: M.Brigids
//
//  Timng study   (April 2007)
//////////////////////////////////

#include <iostream.h> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>  
#include <fstream.h>
#include <string>
#include <stdio.h>
#include <fstream>
#include <iostream>


#include "TkrTrigger.h"

TkrTrigger::TkrTrigger()
{}

TkrTrigger::~TkrTrigger()
{}

double TkrTrigger::Efficiency(TimeOr* TotLayer, int Tw){

  t  = 0;
  i  = 0;
  TC = 0;
  for(l = 0; l < Nlayer; l++){
    T1_0[l] = 0;
    T1_1[l] = 0;
  }
  for(l = 0; l < TotLayer->size(); l++){
    Tower = TotLayer->Tower(l);  // di mylist
    Layer = TotLayer->Layer(l);
    View  = TotLayer->View(l);
    T1    = TotLayer->Tim1(l);    // in ns 
    if(Tw == Tower){
      t++;
      if(View==0){T1_0[Layer] = T1;}
      if(View==1){T1_1[Layer] = T1;}
    }
  }
  //trigger

  //coincidence
  if(t >= 6){
    for(pp =17 ; pp>1; pp--){
      if(T1_0[pp] !=0 && T1_1[pp] !=0 && T1_0[pp-1] !=0 && T1_1[pp-1] !=0 && T1_0[pp-2] !=0 && T1_1[pp-2] !=0){
	TC = T1_0[pp];
	if(TC > T1_1[pp])   TC = T1_1[pp];
	if(TC > T1_0[pp-1]) TC = T1_0[pp-1];
	if(TC > T1_0[pp-2]) TC = T1_0[pp-2];
	if(TC > T1_1[pp-1]) TC = T1_1[pp-1];
	if(TC > T1_1[pp-2]) TC = T1_1[pp-2];
      }
      if(TC > 0) break; // min ---> nobreak
    } // for
  } // t > 6
  else{
    // std::cout << " WARNING, No TKR Trigger " << std::endl;
    TC = 0;
  }

  return TC;
}



double TkrTrigger::NewEfficiency(TimeOr* TotLayer, int Tw){
  t   = 0;
  i   = 0;
  TC  = 0.;
  for(l = 0; l < Nlayer; ++l){
    T1_0[l] = 0.;
    T1_1[l] = 0.;
  }
  for(l = 0; l < 2*Nlayer; ++l){
    T1_2[l] = 0;
  }

  for(l = 0; l < TotLayer->size(); l++){
    Tower = TotLayer->Tower(l); 
    Layer = TotLayer->Layer(l);
    View  = TotLayer->View(l);
    T1    = TotLayer->Tim1(l);    // in ns 
    if(Tw == Tower && T1 >0){
      T1_2[t] = T1;
      t++;
      //
      if(View==0){
	T1_0[Layer] = T1;
	t0++;
      }
      if(View==1){
	T1_1[Layer] = T1;
	t1++;
      }
    }
  }
  //trigger
  //coincidence
  if(t >= 6 && t0 >=3 && t1 >=3){
    TC = T1_2[0];
    for(pp = 1 ; pp < t; ++pp){
      if(TC > T1_2[pp]) TC = T1_2[pp];
    } // for
   } // t > 6
  else{
    //  std::cout << " WARNING, No TKR Trigger " << std::endl;
    TC = 0;
  }
  return TC;
}

        
