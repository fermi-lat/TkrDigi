// Author N.GIGLIETTO
//
//// Simulate the TOT logic
//
//
#ifndef ClusterTot
#define ClusterTot
class Tot{
private:
  int lcountmax;
  int llasttime;
  int lfirsttime;
public:
  Tot();
  ~Tot();
  void Put(double *);
  inline int Get(){return lcountmax;};
  inline int Gett1(){return lfirsttime;};
  inline int Gett2(){return llasttime;};
  inline int GetTotPos(int strip){return strip/64;};
};

#endif
