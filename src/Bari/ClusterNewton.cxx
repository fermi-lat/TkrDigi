//########################################################################
//#  $Id:     ClusterNewton.cc                                           #
//#                                                                      #
//#                                                                      #
//# input: Current for strips fired Icurr[100]                           #
//#                                                                      #
//#output : Voltage for strips fired Vout[10000]                         #
//#                                          output in 10ns step         #
//#--------------------------------------------------AUTHOR N.Giglietto  #
//#		modified for Non-linear electronics    M.Brigida         #
//########################################################################

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandExponential.h"
#include "CLHEP/config/iostream.h"
#include "ClusterNewton.h"


ClusterNewton::ClusterNewton()
{
  ndim = 10000;
  ndim2 = 100;
  Vout = new double[ndim];
  Iout = new double[ndim];
////////////////////////////////////////
/*        ELETTRONICA */

/* CCCCCCCCCCCCCCCCC      DETECTOR PARAMETER  CCCCCCCCCCCCCCCCCCCCCCCCCCCCC */
    ri = (float)2e11;
/*       ! RESISTENZA DI INGRESSO DEL RIVELATORE */
    ci = (float)1.07e-11;
/*       ! CAPACITA' DI INGRESSO DEL RIVELATORE */
/* CCCCCCCCCCCCCCC       PARAMETRI DEL PREAMPLIFICATORE    CCCCCCCCCCCCCCCCCCCCCCC */
    rf = (float)1e10;
/*       ! RESISTENZA DI REAZIONE IN OHM */
    cf = (float)0.92e-13;
/*       ! CAPACITA' DI REAZIONE IN pF */
    gm = (float)7e-4;
/*       ! TRANSCONDUTTANZA DEL MOSFET */
    rc = (float)4e8;
/*       ! RESISTENZA DI CARICO DEL MOSFET */
    cc = (float)1.5e-13;
/*       ! CAPACITA' DI CARICO DEL MOSFET */
    av = rc * gm;
/*       ! guadagno a loop aperto */
    tau = rc * cc;
/*       ! TEMPO CARATTERISTICO DEL CARICO */
    tf = rf * cf;
/*       ! TEMPO CARATTERISTICO DELLA REAZIONE */


    nu = av * ri * rf / (ri * (av + (float)1.) + rf);
    a1 = ri * rf * (ci + cf) * tau / (ri * (av + (float)1.) + rf);
    a2 = (ri * rf * (ci + (av + (float)1.) * cf) + (ri + rf) * tau) 
	    / (ri * (av + (float)1.) + rf);
}

ClusterNewton::~ClusterNewton()
{
  delete Vout;
  delete Iout;
}

void ClusterNewton::AddNoise()
{
  double deltatp = 1.e-9;
  double ratep = 2.e7;
  double yyip2 = 2.28e-28;
  double tmax = 10000*1.e-9; 
  // add parallel noise
  // xip = spike amplitudes
  // enc = 1550 e- for one ladder
  //
  double xip= 30.*(sqrt(yyip2)/sqrt(2.*ratep))/deltatp; //in Ampere
  double dt2;
  double tt=0.;
  while ( tt< tmax) {
    double ratep2=1./ratep;
    dt2=RandExponential::shoot(ratep2); //ratep2
    tt += dt2;
    if(tt>=tmax) break;
    HepDouble ran = RandFlat::shoot();
    double sign = -1.;
    if(ran >= 0.5) {sign=+1.;}
    int it = int(tt*1.e9);  // -1?
    if(it>=ndim) break;
    Iout[it] += sign*xip;  // add spike in timing slot 
  }
}

//########################################################################
// Preamp simulation
// Input Iout in microAmp
// Output Vout                                 
//-----------------------------------------------------AUTHOR N.Giglietto 
// modified in order to take into account the electronical
//         non-linear behaviour               M.Brigida
//########################################################################
void ClusterNewton::Preamp()
{
  double t2 = t*t;
  Vout[0] = 0.;
//  Vout[0] = (nu*Iout[0])/(a1/(t2)+a2/t+1.);
  Vout[1] = (Vout[0]*(2*a1/(t2)+a2/t)
             + nu * Iout[1])
             / (a1 / (t2) + a2 /t + 1.);
  for (int m = 2;m < ndim; m++)
    {
	av = rc * gm;
    /*       ! guadagno a loop aperto */
    tau = rc * cc;
	/*       ! TEMPO CARATTERISTICO DEL CARICO */
    tf = rf * cf;
	/*       ! TEMPO CARATTERISTICO DELLA REAZIONE */

    nu = av * ri * rf / (ri * (av + (float)1.) + rf);
    a1 = ri * rf * (ci + cf) * tau / (ri * (av + (float)1.) + rf);
    a2 = (ri * rf * (ci + (av + (float)1.) * cf) + (ri + rf) * tau) 
	    / (ri * (av + (float)1.) + rf);

      Vout[m] = (Vout[m-1]*(2*a1/(t2)+a2/t)
		       - Vout[m-2]*a1/(t2)
		       + nu*Iout[m])/
 	               (a1/(t2)+a2/t+1.);
	  if(Vout[m] > 0.325){gm = 5.2e-3;}
    }
}

//########################################################################
//  shaper simulation                                                     
//-----------------------------------------------------AUTHOR N.Giglietto 
//########################################################################
void ClusterNewton::Shaper(double Qtot)
{

/* CCCCCCCCCCCCCCC       PARAMETRI DELLO SHAPER   CCCCCCCCCCCCCCCCCCCCCCCCCCCCC */
    cis = (float)1.5e-12;
/* CAPACITÀ DI DISACC0PPIAMENTO */
    rfs = ((float)20.2e6) * Qtot;   /// calcolo della carica in input (femtocoulomb)
/*       ! RESISTENZA DI REAZIONE IN OHM */
    cfs = (float)0.9e-13;
/*       ! CAPACITA' DI REAZIONE IN pF */
    gms = (float)7e-4;
/*       ! TRANSCONDUTTANZA DEL MOSFET */
    rcs = (float)6e10;
/*       ! RESISTENZA DI CARICO DEL MOSFET */
    ccs = (float)8e-11;
/*       ! CAPACITA' DI CARICO DEL MOSFET */
    avs = rcs * gms;
/*       ! gudagno a loop aperto shaper */
    taus = rcs * ccs;
/*       ! TEMPO CARATTERISTICO DEL CARICO SHAPER */
    tfs = rfs * cfs;
/*       ! tempo caratteristico di reazione SHAPER */
    nus = cis * rfs * avs / (avs + (double)1.);
    a2s = (taus + rfs * (cis + cfs * (avs + (double)1.))) /
	     (avs + (double)1.);
    a1s = taus * rfs * (cfs + cis) / (avs + (double)1.);
/* CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC */
  double Vouts[10000]; // ndim
  double Vapp[10000];
  double t = 10.e-9; 
  double t2 = t*t;
//
  for(int k=0;k<10000;k++){Vapp[k] = Vout[k];}
  for(int nt = 0; nt < 1000; nt++)
  {
	  int nn = 10 * (nt -1);
      Vout[nt] = Vapp[nn];  
  }
  for(int ntt = 100; ntt < ndim; ntt++)
  { 
	Vout[ntt] = (Vout[99]) * exp(-float(ntt - 99)/(100*Qtot));
  }
//

  Vouts[0] = (nus * Vout[0]/t)/(a1s/(t2)+a2s/t+1.);
  Vouts[1]=(nus*(Vout[1]-Vout[0])/t
		  +(2*a1s/(t2)+a2s/t)*Vouts[0])
                  / (a1s / (t2) + a2s /t + 1.);
  int m;
  for (m = 2; m<ndim; m++)
    {
      Vouts[m]= (nus*(Vout[m]-Vout[m-1])/t
		  +(2*a1s/(t2)+a2s/t)*Vouts[m-1]
		       -Vouts[m-2]*a1s/(t2))
                        / (a1s / (t2) + a2s /t + 1.);
    }
/*  for(int pp =0; pp < ndim; pp++)
  {
	  if((fabs(Vouts[pp] * 1.e-3))  > 1100) {Vouts[pp] = 11.e5;} // saturazione
  }*/
  for (m=0; m<ndim; m++){
    Vout[m] = Vouts[m];
  }
}

void ClusterNewton::Clean()
{
  for (int i=0;i<ndim;i++)
    {
	  Vout[i] = 0.;
	  Iout[i] = 0;
    }
}

void ClusterNewton::Newton(const double* Icurr)
 {
// Icurr should be in Ampere
//Author Loparco,Mazziotta,Giglietto, Brigida
//  Gargano                       

    t = (double)1e-9; //this is in common to preamp and shaper

/*       T = tempo di campionamento della corrente (in s) */

    dts = (double)10.;
    tfin = dts * (double)1e3;

/*       dts = tempo di campionamento del segnale in tensione (in ns) */
/*       tfin = tempo per cui seguiamo il segnale di tensione (in ns) */

    Clean();
    // to have current in Ampere
    int mm;
    for (mm = 0; mm < ndim2; mm++) {
      Iout[mm] = Icurr[mm]*1.e-6;  
    }

    AddNoise(); // write on Iout
	qtot = 0.;
	for(int it = 0; it < ndim2; it++)
	{
		qtot += (Iout[it]) * (1.e-9) * (1.e15); // charge in fC 
	}
	qtot = fabs(qtot);

    for ( mm = 0;mm < ndim; mm++) {
	  Iout[mm]*=1.e+6;  // copio e scalo le unita'
    }

    t=1.e-9;
    Preamp();
    Shaper(qtot);
 }
  
