#ifndef _GlastDigi_ReadDigi_H
#define _GlastDigi_ReadDigi_H 1

#include "GaudiKernel/Algorithm.h"

/*! \class ReadDigi.cpp
  */

class ReadDigi : public Algorithm {

public:
  //! Constructor of this form must be provided
  ReadDigi(const std::string& name, ISvcLocator* pSvcLocator); 

  //! mandatory
  StatusCode initialize();
  //! mandatory
  StatusCode execute();
  //! mandatory
  StatusCode finalize();

private:

};


#endif // _GlastDigi_ReadDigi_H
