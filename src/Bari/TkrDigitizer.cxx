////////////////////////////////////////////////
// Authors: M.Brigida, N.Giglietto            //
//                                            //
//  march 20,03  obsolete calls deleted (MB)  //
//                                            //
//  march 18,04  clusterize method update (MB)//
//                                            //
// April 2007 new release revised  (MB)       //
////////////////////////////////////////////////

#include "TkrDigitizer.h"
#include "TMath.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/config/iostream.h"
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TkrUtil/ITkrToTSvc.h"
#include <string>


TkrDigitizer::TkrDigitizer() {
    m_clusterPar  = new Cluster();
    m_clusterProp = new ClusterPropagator();
    m_clusterCurr = new CurrOr();
    m_totLayer    = new TotOr();
    m_totOr       = new TimeOr();
    TRIGGER       = new TkrTrigger();
    m_clusterPar->Clean();
}


TkrDigitizer::~TkrDigitizer() {
    delete m_clusterPar;
    delete m_clusterProp;
    delete m_clusterCurr;
    delete m_totLayer;
    delete TRIGGER;
    delete m_totOr;
}

void TkrDigitizer::Clean() {
   m_clusterPar->Clean();    
}

void TkrDigitizer::set(double energy, HepPoint3D entry, HepPoint3D exit,
		       idents::VolumeIdentifier volId,
		       Event::McPositionHit* pHit) {
    m_entry  = entry;
    m_exit   = exit;
    m_energy = energy;
    m_volId  = volId;
    m_hit    = pHit;
}


// SetDigit --> analog section

void TkrDigitizer::setDigit(InitCurrent* OpenCurr) {
  m_clusterProp->setMapCurr(m_clusterCurr);
  m_clusterProp->setOpenCurr(OpenCurr);
  m_clusterPar->SetCluster(m_entry, m_exit, m_energy);
  int NumberCluster = m_clusterPar->GetNumberOfClusters();
  double* ClusCharge = m_clusterPar->GetClusCharge();
  HepPoint3D* ClusCoord = m_clusterPar->GetClusCoord();
  m_clusterProp->setClusterPropagator(ClusCoord, ClusCharge, NumberCluster,
					  m_volId, m_hit);
}



// Clusterize --> analog section
void TkrDigitizer::clusterize(CurrOr* CurrentOr) {
  const CurrOr::DigiElemCol& l = m_clusterCurr->getList();
  CurrentOr->clear(); 
  CurrentOr->add(l);
}


// Digitize --> Digital section
TotOr* TkrDigitizer::digitize(const CurrOr& CurrentOr) {
    const CurrOr::DigiElemCol& l = CurrentOr.getList();
    const ITkrToTSvc* pToTSvc;
    energy = 0.;
    charge = 0;
    tim1   = 0;
    tim2   = 0;
    // Bari1
    iit = 0;
    T1Trig = 99999999.;
    for ( CurrOr::DigiElemCol::const_iterator it=l.begin(); it!=l.end(); ++it ){// loop
      double* PNum = it->getCurrent();     
      PP          = 0;   
      ToT         = 0;
      DeltaT      = 0;
      Qstr        = 0.;
      PP          = PNum[0];
      cr          = CLHEP::RandGauss::shoot(0.,1.)*(sqrt(0.1*PP)); // random stat fluctuation
      er          = CLHEP::RandGauss::shoot(0.,1.)*(1500.);        // random el noise fluctuation
      CPNum       = PP + er;
      
      if(CPNum > 0){Qstr = CPNum * 1.67E-4;}                       //fCoulomb
      Gain = Gain0 + CLHEP::RandGauss::shoot(0.,1.)*RmsGain0;
      V    =  Qstr * Gain;
      if(V > Vsat) V = Vsat;
      if(V > Vth){
	if(Qstr < 40.){ToT = 1828.4 * Qstr + 1443.;}   // ns
	else{ToT = 328.67 * Qstr + 60668.;}            // ns
	DeltaT  = -90.945* TMath::Log(Qstr) + 743.51;  //ns
	if(DeltaT < 0) DeltaT = 0;
	// T1Trig (semplified version)
	if (DeltaT < T1Trig) T1Trig = DeltaT;
	//
	// load ToT from calibration 
	/*	
	  double e    = CURRENT_TO_ENERGY * (fabs(Qstr));
	  e           = e *1000.;
	  int tower   = it->getTower();
	  int layer   = it->getLayer();
  	  int view    = it->getView();
	  int index   = it->getStrip();
	  std::cout << " test iit " << iit << " " << e << " " << tower << " " << layer<< std::endl;  
	  //  int iToT    = pToTSvc->getRawToT(e, tower, layer, view, index);
	  double gain = pToTSvc->getGain(tower, layer, view, index);
	*/
      } 
      else {
	DeltaT = -1.;
	ToT    = -1;
      }
      T1[iit] = DeltaT;        // ns
      T2[iit] = ToT + DeltaT;  // ns
      QQ[iit] = Qstr;          // fC
      /////// timing study
      lTower = it->getTower();
      lLayer = it->getLayer();
      lView  = it->getView();
      lStrip = it->getStrip();
      Tim1   = T1[iit];
      Tim2   = T2[iit];
      // per hit strip
      if (Tim1 != -1 && Tim2 != -2)m_totOr->Add(lTower, lLayer, lView, lStrip, Tim1, Tim2);
      /////////
      iit++;
      if (iit == 5000) break;
    } // end loop
    //
    iit  = 0;
    // T1Trig (complete version)
    for(ii = 0; ii < NTw; ii++){
      n_t1trig[ii] = 0.;
      T1TrigN      = TRIGGER->NewEfficiency(m_totOr,ii);
      if(T1TrigN > 0){n_t1trig[ii] = T1TrigN;}
    }
    //
    Tack   = T1Trig  + TriReq + Tack0;
    for ( CurrOr::DigiElemCol::const_iterator it=l.begin(); it!=l.end(); ++it ){
      // T1Trig =  n_t1trig[it->getTower()];
      // Tack   = T1Trig  + TriReq + Tack0;
      if (T1Trig > 0 && T2[iit] > Tack ) {
	energy = CURRENT_TO_ENERGY * (fabs(QQ[iit]));
	energy = energy *1000.;     // keV
	tim1   = Tack/10.;          // time1, in 10 ns step
	tim2   =(T2[iit]-Tack)/10.; //  time2, in 10 ns step
	//	std::cout << " Bari DIGI STORE " << energy  << "- charge= " << QQ[iit]
	//  << " times " << tim1 << " " << tim2 << " strip ID " << it->getStrip()<< std::endl;
	m_totLayer->add(it->getVolId(), it->getStrip(), it->getHits(),tim1, tim2, energy);
      }
      iit++;
    }// end for
    return m_totLayer;
}


