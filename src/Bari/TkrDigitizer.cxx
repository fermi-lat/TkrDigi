////////////////////////////////////////////////
// Authors: M.Brigida, N.Giglietto            //
//                                            //
//  march 20,03  obsolete calls deleted (MB)  //
//                                            //
////////////////////////////////////////////////

#include "TkrDigitizer.h"

// MK: shouldn't these two be member variables?  What do they actually do?
int b = 0;
int b1 = 0;


TkrDigitizer::TkrDigitizer() {
    // MWK: all these objects are deleted properly in the destructor, but do
    // they really need to be on the "heap", i.e. "new"?
    m_clusterPar  = new Cluster();
    m_clusterProp = new ClusterPropagator();
    m_newton      = new ClusterNewton();
    m_clusterCurr = new CurrOr();
    m_tot         = new Tot(); 
    m_totLayer    = new TotOr();
}


TkrDigitizer::~TkrDigitizer() {
    delete m_clusterPar;
    delete m_clusterProp;
    delete m_newton;
    delete m_clusterCurr;
    delete m_tot;
    delete m_totLayer;
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
    m_clusterProp->clean();
    m_clusterProp->setMapCurr(m_clusterCurr);
    m_clusterProp->setOpenCurr(OpenCurr);
    m_clusterPar->Clean(); 
    m_clusterPar->SetCluster(m_entry, m_exit, m_energy);
    int NumberCluster = m_clusterPar->GetNumberOfClusters();
    HepPoint3D* ClusCoord = m_clusterPar->GetClusCoord();
    double* ClusCharge = m_clusterPar->GetClusCharge();
    m_clusterProp->setClusterPropagator(ClusCoord, ClusCharge, NumberCluster,
					m_volId, m_hit);
}


// Clusterize --> analog section
void TkrDigitizer::clusterize(CurrOr* CurrentOr) {
    const CurrOr::DigiElemCol& l = m_clusterCurr->getList();
    int a  = l.back().getLayer();
    int a1 = l.back().getStrip();
    if( !(a == b && a1 == b1) )
	CurrentOr->clear();

    CurrentOr->add(l);
    /* debug
    for ( CurrOr::DigiElemCol::iterator it=l.begin(); it!=l.end(); ++it )
	std::cout << it->getTower() << " " << it->getLayer() << " "
		  << it->getView() << " " << it->getStrip() << " "
		  << it->getHits().size() << std::endl;
    */

    b1 = a1;
}


// Digitize --> Digital section
TotOr* TkrDigitizer::digitize(const CurrOr& CurrentOr) {
    const CurrOr::DigiElemCol& l = CurrentOr.getList();
    for ( CurrOr::DigiElemCol::const_iterator it=l.begin(); it!=l.end(); ++it ){
        double charge = 0;
        const double* I = it->getCurrent();
        m_newton->Newton(I);  // in microampere   
        double* Vout = m_newton->GetVout(); // MicroVolt
        double* Iout = m_newton->GetIout();
        for ( int ii=0; ii<50; ii++ )
            charge += Iout[ii];
        // charge on fired strip (Monica B., instead of energy)
        const double energy = CURRENT_TO_ENERGY * fabs(charge);

        m_tot->Put(Vout);
        const int tim1 = m_tot->Gett1();  // time1, in 10 ns step
        const int tim2 = m_tot->Gett2(); //  time2, in 10 ns step
	//      (tim2-tim1) = ToT per strip
        if ( tim1 > 0 )
	    m_totLayer->add(it->getVolId(), it->getStrip(), it->getHits(),
                            tim1, tim2, energy);
    } // end loop over cluster in a hit
    return m_totLayer;
}