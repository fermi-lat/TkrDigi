//////////////////////////////////
// Authors: M.Brigida, N.Giglietto
///////////////////////////////////
#ifndef TkrDigitizer_h
#define TkrDigitizer_h 1

#include "InitCurrent.h"
#include "Cluster.h"
#include "CurrOr.h"
#include "ClusterPropagator.h"
#include "ClusterNewton.h"
#include "Tot.h"
#include "TotOr.h"

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
    /// noise+preamp+shaper for each fired strip
    ClusterNewton* m_newton;
    /// TOT information
    Tot* m_tot;
    /// Or of ToT in a layer
    TotOr* m_totLayer;

};

#endif
