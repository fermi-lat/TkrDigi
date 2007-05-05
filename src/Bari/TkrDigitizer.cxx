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

#include <string>

const double TkrDigitizer::Tack0    = 1000.; // ns
const double TkrDigitizer::TriReq   = 1000.; //ns
const double TkrDigitizer::Gain0    = 100.; // mV/fC
const double TkrDigitizer::RmsGain0 = 6.; // mV/fC
const double TkrDigitizer::Vth      = 125.; // mV = 1/4 MIP, 1 MIP => 5 fC => 500 mV
const double TkrDigitizer::Vsat     = 1100.; // mV, Saturation voltage output   


TkrDigitizer::TkrDigitizer() {
    m_clusterPar  = new Cluster();
    m_clusterProp = new ClusterPropagator();
    m_clusterCurr = new CurrOr();
    m_totLayer    = new TotOr();  
    m_clusterPar->Clean();
}


TkrDigitizer::~TkrDigitizer() {
    delete m_clusterPar;
    delete m_clusterProp;
    delete m_clusterCurr;
    delete m_totLayer;
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
    energy = 0.;
    charge = 0;
    tim1   = 0;
    tim2   = 0;
    // Bari1
    iit = 0;
    T1Trig = 99999999.;
    for ( CurrOr::DigiElemCol::const_iterator it=l.begin(); it!=l.end(); ++it ){// loop
      const double* PNum = it->getCurrent();     
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
	  e       = CURRENT_TO_ENERGY * (fabs(Qstr));
	  e       = e *1000.;
	  lTower  = it->getTower();
	  lLayer  = it->getLayer();
	  lView   = it->getView();
	  index   = it->getStrip();
	  std::cout << " test iit " << iit << " " << e 
	  << " " << lTower << " " << lLayer << std::endl;  
	  int iToT    = pToTSvc->getRawToT(e, lTower, lLayer, lView, index);
	  double gain = pToTSvc->getGain(lTower, lLayer, lView, index);
	  int totMax    = pToTSvc->getMaxToT();
	*/
      } 
      else {
	DeltaT = -1.;
	ToT    = -1;
      }
      T1[iit] = DeltaT;        // ns
      T2[iit] = ToT + DeltaT;  // ns
      QQ[iit] = Qstr;          // fC
      
      iit++;
      if (iit == 5000) break;
    } // end loop
    
    iit  = 0;
    Tack   = T1Trig  + TriReq + Tack0;
    for ( CurrOr::DigiElemCol::const_iterator it=l.begin(); it!=l.end(); ++it ){
      if (T1Trig > 0 && T2[iit] > Tack ) {
	energy = CURRENT_TO_ENERGY * (fabs(QQ[iit]));
	energy = energy *1000.;     // keV
	tim1   = static_cast<int>(Tack) / 10;          // time1, in 10 ns step
	tim2   = static_cast<int>(T2[iit]-Tack) / 10; //  time2, in 10 ns step
	//	std::cout << " Bari DIGI STORE " << energy  << "- charge= " << QQ[iit]
	//  << " times " << tim1 << " " << tim2 << " strip ID " << it->getStrip()<< std::endl;
	m_totLayer->add(it->getVolId(), it->getStrip(), it->getHits(),tim1, tim2, energy);
      }
      iit++;
    }// end for
    return m_totLayer;
}


