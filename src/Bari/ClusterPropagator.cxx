//########################################################################
//#  $Id:     ClusterPropagator.cc                                       #
//#               Method to propagate clusters towards strips            #
//#                                                                      #    
//#                                     authors: M.Brigida, N.Giglietto  #
//#                                                                      #
//#  23-Aug-02 changed y to x; removed Xstrip, AllCurr -- LSR            #
//########################################################################

#include "CLHEP/Random/Randomize.h"
#include "CLHEP/config/iostream.h"

#include "ClusterPropagator.h"
#include "../SiStripList.h"


// method to associate to a position the two near strip
void  ClusterPropagator::xtoid(float xpos, int &id1, int &id2)
{
    // if out of silicon wafer  id1=id2=-1
    double XX1 = 0., XX2 = 0.;
    double dist1, dist2;
    id1 = -1;
    id2 = -1;
    unsigned int id = SiStripList::stripId(xpos);
    if (id != 65535) {  // no id found (see SiliconPlaneGeometry)
        unsigned int yy = id +1, jj = id -1;
        XX1 = SiStripList::calculateBin(yy);
        XX2 = SiStripList::calculateBin(jj);
        dist1 = fabs(xpos*1000. - XX1*1000.);
        dist2 = fabs(xpos*1000. - XX2*1000.);
        id1 = id;
        if(dist1 > dist2) {
            id2 = id1 - 1;
        }
        else{
            id2 = id1 + 1;
        }
    }
}

//##################################################################
// propagate each cluster of the track inside the silicon element
// and evaluate the currents induced into the closest strips 
// currents are in MICROAMPS <<<=========================
// Authors Brigida, Giglietto
//###################################################################
void ClusterPropagator::setClusterPropagator(HepPoint3D* XClus, double* QClus,
					     int NClus, 
                                             idents::VolumeIdentifier volId,
					     Event::McPositionHit* pHit)
{ 
    // the coordinates of the track are in the *local* coordinate system... 
    // in this system, x always is the measurement direction
    
    int ID1, ID2;
    double* Icurr;
    double XV[2];
    int nt;
    double Qclu = 0.;
    double XX = 0., XX0 = 0.; // in mm
    for (int j = 0; j< NClus; j++) {// loop over cluster
        XX = XClus[j].x();  // in the local frame, x is the measured coordinate -- LSR
        xtoid(XX, ID1, ID2); // two strip fired
        if(ID1<0) {
            std::cout << "ID1 out of detector" << std::endl;
            continue;       // goto next cluster
        }
        XX0 = SiStripList::calculateBin(ID1);
        Qclu = QClus[j]; 
        XV[0] = XX - XX0;
        XV[1] = XClus[j].z(); // mm respect to wafer SR
        m_current->GetCurrent(XV);
        Icurr = m_current->Get();   
        for (nt = 0;nt < 100;nt++){
            Icurr[nt] = Icurr[nt] * Qclu;     // val in microAmp
        }
        m_mapCurr->add(volId, ID1, Icurr, pHit);
        // NOW STRIP ID2
        if(ID2 < 0) {
            std::cout << "ID2 out of detector "<< ID2 << std::endl;
            continue;    // goto next cluster
        }             
        XX0 = SiStripList::calculateBin(ID2);
        XV[0] =  XX - XX0;
        XV[1] = XClus[j].z(); // mm, respect to wafer SR 
        m_current->GetCurrent(XV);
        Icurr = m_current->Get();
        for (nt = 0; nt < 100; nt++){
            Icurr[nt] = Icurr[nt] * Qclu;     // val in microAmp
        }
        m_mapCurr->add(volId, ID2, Icurr, pHit);
    }
    // END CLUSTER LOOP
}
