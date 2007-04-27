//////////////////////////////////
//  class TkrTrigger
//  author: M.Brigida
//
//  Timng study
//////////////////////////////////



#ifndef TKRTRIGGER_H
#define TKRTRIGGER_H 1

#if !defined(__CINT__)

#endif

#include "TimeOr.h"

class TkrTrigger
{
public:
      
  TkrTrigger();
  ~TkrTrigger();
  double Efficiency(TimeOr*, int);
  double NewEfficiency(TimeOr*, int);

private:

  static const int Nlayer = 18;
  int i;
  int l;
  int t, t1, t0;
  int pp;
  int Tower, Layer, View;
  double T1;  
  double TC, TC1, TC0, TCC;
  double T1_0[Nlayer];
  double T1_1[Nlayer];
  double T1_2[2*Nlayer];
};

#endif
