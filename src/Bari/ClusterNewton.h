// Author: N.Giglietto

#ifndef ClusterNewton_h
#define ClusterNewton_h 1
class ClusterNewton    //class declaration
{
  private:
  int ndim;
  int ndim2;
  double *Iout;
  double *Vout;

    /* Local variables */
    int itim;
    double taus;
    double cc, cf, ci, gm, rc, av, rf, tf, ri;
    int it, jt;
    int jstrip, istrip;
    double ccs, cfs, gms, rcs, tau, rfs, avs, tfs;
    double nu, a1, a2, t;
    int mfir; 
    double nus, a2s, a1s, cis, dts, tfin;
	double qtot;
    double Vouts[10000]; // ndim
    double Vapp[10000];

  public:  
ClusterNewton();
~ClusterNewton();
// definizione dei metodi    
  void Clean();
  void Newton(const double* Icurr);  // out Vout[1000]
  void AddNoise();
  void Shaper(double);
  void Shaper();
  void Preamp();
  inline double* GetVout(){return Vout;};
  inline double* GetIout(){return Iout;};
};

#endif
