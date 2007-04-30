//////////////////////////////////
// Authors: M.Brigida, N.Giglietto
///////////////////////////////////
#ifndef TkrDigitizer_h
#define TkrDigitizer_h 1

#include "InitCurrent.h"
#include "Cluster.h"
#include "CurrOr.h"
#include "ClusterPropagator.h"
#include "TkrTrigger.h"
#include "TotOr.h"
#include "TimeOr.h"

#include <string>
#include "idents/VolumeIdentifier.h"

#include "Event/MonteCarlo/McPositionHit.h"

#define CURRENT_TO_ENERGY 1E4/1.6 * 3.6E-6 // Sadrozinski, PDG
// current in uA
// * 1ns -> 1uA = 1fC
// 1fC = 1E4/1.6 electrons
// 1 electron = 3.6E-6 MeV

class TkrDigitizer {

 public:

    TkrDigitizer();
    ~TkrDigitizer();
    
    void set(double, HepPoint3D, HepPoint3D, idents::VolumeIdentifier,
	     Event::McPositionHit*);
    /// set digit paramter
    void setDigit(InitCurrent*);
    /// set cluster parameter
    void clusterize(CurrOr*);
    void Clean();
    /// digitization
    TotOr* digitize(const CurrOr&);

 private:

  //NG to compile in VC8
  static const double Tack0/*    = 1000.*/; // ns
  static const double TriReq/*   = 1000.*/; //ns
  static const double Gain0/*    = 100.*/; // mV/fC
  static const double RmsGain0/* = 6.*/; // mV/fC
  static const double Vth/*      = 125.*/; // mV = 1/4 MIP, 1 MIP => 5 fC => 500 mV
  static const double Vsat/*     = 1100.*/; // mV, Saturation voltage output   
  static const int Tmax        = 5000;
  static const int NTw         = 16;

  double T1[Tmax];
  double T2[Tmax];
  double QQ[Tmax];

  double energy, charge;
  int tim1, tim2;
  int ii, iit;
  int lTower;
  int lLayer;
  int lView;
  int lStrip;
  double CorrPairNum,CPNum;
  double PP, cr, er;
  double Gain;
  double ToT;
  double DeltaT;
  double V, Vtemp, Qstr;
  double Tim1;
  double Tim2;
  double T1Trig;
  double T1TrigN; 
  double Tack;
  double n_t1trig[NTw];
 
  double m_energy;
  idents::VolumeIdentifier m_volId;
  Event::McPositionHit* m_hit;
  HepPoint3D m_entry;
  HepPoint3D m_exit;
  /// cluster propagator
  ClusterPropagator* m_clusterProp;
  /// Param of cluster
  Cluster* m_clusterPar;
  /// set current signals from each cluster
  CurrOr* m_clusterCurr;
  /// Or of ToT in a layer
  TotOr* m_totLayer;
  // of of time for trigger
  TimeOr* m_totOr;
  TkrTrigger* TRIGGER;

};
#endif
