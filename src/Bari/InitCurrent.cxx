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
//#include <stdlib.h>  
#include <fstream>
//#include <math.h>
//#include <stdio.h>
#include "CLHEP/Random/Randomize.h"
#include "CLHEP/config/iostream.h"

#include "InitCurrent.h"

InitCurrent::InitCurrent()
{
    // trying to eliminate prospective memory leaks.  CURR was nowhere deleted.
    // I define it with dimension 250000 directly in the header file.
    // Anyway, one should define dimensions in constants and use these in the
    // code, instead of carrying numbers around in the code.
    //    CURR = new double[250000];
    for ( int jj=0; jj<100; jj++ )
        XXcurr[jj] = 0.0;
    for ( int tt=0; tt<ndim; tt++ )
        CURR[tt] = 0.0; 
}

InitCurrent::~InitCurrent()
{
}

StatusCode InitCurrent::OpenCurrent(std::string currents)
{
    StatusCode sc = StatusCode::SUCCESS;
    int ID1, ID2;
    double CurrPar[100];
    double tmp = 0.;
    int nval=0;    
    
    std::ifstream fin(currents.c_str()); 
    if (!fin) {
        return StatusCode::FAILURE;
    }
    for(int j=0; j<2500; j++){ 
        fin >> ID1 >> ID2 ;
	// now read nval elements not null
	fin >> nval;
        for(int k=0; k<nval; k++){
	  //        for(int k=0; k<100; k++){
            fin >> CurrPar[k];
            int ii = j*100 + k;
            if(ii  > ndim-1 ){
                std::cout<<"OpenCurr out of range *************"<< ii <<std::endl;
            }		
            tmp = CurrPar[k];
            CURR[ii] = tmp;
        }// loop k closed	
    } // loop j closed
    fin.close();
    return sc;
} 

void InitCurrent::GetCurrent(double* XX)
{  
    int k1 = 0;
    int k2, P2, P3;
    double X[50], Z[50]; //nbin
    k2 = k1 + 50;
    double Xpos = XX[0];
    double Zpos = XX[1];
    int N = 50;
    // double Xmin = -(Pitch); da chiedere # define!!!!!!
    //  double Xmax = Pitch;
    double Xmin = -(0.228);
    double Xmax = 0.228;
    double Zmin = -0.2;
    double Zmax = 0.2;
    double DeltaX = (Xmax - Xmin)/N;
    double DeltaZ = (Zmax - Zmin)/N;
    double dmin = DeltaZ/10.;      // distanza per le correnti interpolate
    for (int i = 0; i < N; i++){
        X[i] = Xmin + DeltaX * (i + 0.5);
        Z[i] = Zmin + DeltaZ * (i + 0.5);
    }
    int j;
    for(j = 0; j < 100; j++){
        XXcurr[j] = 0.;
    }
    double currpes=0.;
    double allpes=0.;
    double dist=0.;
    // check if ix iy are in the right limits
    int ix = int((Xpos - Xmin)/DeltaX - 0.5); // lower x bin index
    int iy = int((Zpos - Zmin)/DeltaZ - 0.5); // lower z bin indx
    if(ix < 0 || ix >= 50 || iy < 0 || iy >= 50) {  // protezione
        goto esci; 
    }
    for(j = 0; j < 100; j++){
        P3 = (iy + (ix * 50));
        P2 = ix * 50 + iy;
        if(P2>2499){
            std::cout<<"WARNING P2 greater than  limit!!! FIRST CALL "<<P2<<" " <<ix<<" "<<iy<<std::endl;
            goto esci;
        }
        dist = pow((pow((Xpos - X[ix]),2.) + pow((Zpos - Z[iy]),2.)),0.5);
        if(dist > dmin){
            allpes = 1./dist;
            currpes = CURR[P2*100 + j]/dist; //cc
        }
        else{
            XXcurr[j]=CURR[P2*100+j]; //cc
            goto esci;
        }
        P3 = (iy + ((ix + 1) * 50));
        P2 = ix * 50 + iy + 1; // OK
        if(P2>2499){
            //	  std::cout<<"WARNING P2 greater than  limit!!! SECOND CALL "<<P2<<" " <<ix<<" "<<iy<<std::endl;
        }
        if(P2 < 2500){
            if(dist > dmin){
                if((ix + 1) < 50) {
                    dist = pow( (pow((Xpos-X[ix + 1]),2.) + pow((Zpos - Z[iy]),2.)),0.5);
                    allpes += 1./dist;
                    currpes += CURR[P2*100 + j]/dist; //cc
                }
            }
            else{
                XXcurr[j] = CURR[P2*100 + j];//cc
                goto esci;
            }
        }
        
        P3 = (iy + 1 + ((ix + 1) * 50));
        P2 = (ix + 1) * 50 + iy+1; // OK
        if(P2>2499){
            //	  std::cout<<"WARNING P2 greater than  limit!!! THIRD CALL "<<P2<<" " <<ix<<" "<<iy<<std::endl;
        }
        if(P2 < 2500){
            if(dist > dmin){
                if((ix + 1) < 50 && (iy + 1) < 50) {
                    dist = pow((pow((Xpos - X[ix+1]),2.) + pow((Zpos - Z[iy+1]),2.)),0.5);
                    allpes += 1./dist;
                    currpes += CURR[P2*100 + j]/dist; //cc
                }
            }
            else{
                XXcurr[j]=CURR[P2*100 + j]; //cc
                goto esci;
            }
        }
        
        P3 = (iy+1 + (ix * 50));
        P2 = (ix+1) * 50 + iy; // OK
        if(P2>2499){
            //	  std::cout<<"WARNING P2 greater than  limit!!! FOURTH CALL "<<P2<<" " <<ix<<" "<<iy<<std::endl;
        }
        if(P2 < 2500) {
            if(dist > dmin){
                if((iy + 1) < 50) {
                    dist = pow((pow((Xpos - X[ix]),2.) + pow((Zpos - Z[iy+1]),2.)),0.5);
                    allpes += 1./dist;
                    currpes += CURR[P2*100 + j]/dist; //cc
                } 
            }
            else{
                XXcurr[j] = CURR[P2*100 + j];
                goto esci;
            }
        }
        XXcurr[j] = currpes/allpes;
esci:;
    }
}








