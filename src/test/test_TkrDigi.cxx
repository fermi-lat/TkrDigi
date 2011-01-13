// $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/test/test_TkrDigi.cxx,v 1.4.654.1 2010/09/18 03:53:47 heather Exp $

// Include files
// Gaudi system includes
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"

// TDS class declarations: input data, and McParticle tree

#include "Event/TopLevel/EventModel.h"

#include "Event/Digi/TkrDigi.h"

// Define the class here instead of in a header file: not needed anywhere but here!
//------------------------------------------------------------------------------
/** 
A simple algorithm.

  
*/
class test_TkrDigi : public Algorithm {
public:
    test_TkrDigi(const std::string& name, ISvcLocator* pSvcLocator);
    StatusCode initialize();
    StatusCode execute();
    StatusCode finalize();
    
private: 
    //! number of times called
    int m_count; 
    //! the GlastDetSvc used for access to detector info
};
//------------------------------------------------------------------------

// necessary to define a Factory for this algorithm
// expect that the xxx_load.cxx file contains a call     
//     DLL_DECL_ALGORITHM( test_TkrDigi );

//static const AlgFactory<test_TkrDigi>  Factory;
//const IAlgFactory& test_TkrDigiFactory = Factory;
DECLARE_ALGORITHM_FACTORY(test_TkrDigi);

//------------------------------------------------------------------------
//! ctor
test_TkrDigi::test_TkrDigi(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator)
,m_count(0)
{
}

//------------------------------------------------------------------------
//! set parameters and attach to various perhaps useful services.
StatusCode test_TkrDigi::initialize(){
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;
    
    return sc;
}

//------------------------------------------------------------------------
//! process an event
StatusCode test_TkrDigi::execute()
{
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream   log( msgSvc(), name() );
    log << MSG::INFO << "Has been called " << ++m_count << " time(s): ";
    
    
    // First, the collection of TkrDigis is retrieved from the TDS
    SmartDataPtr<Event::TkrDigiCol> digiCol(eventSvc(),EventModel::Digi::TkrDigiCol );
    
    if (digiCol == 0) {
        log << "no TkrDigiCol found" << endreq;
        sc = StatusCode::FAILURE;
        return sc;
    } else {
        log << digiCol->size() << " TKR digis found " << endreq;
        if(m_count==1) {
            log << MSG::INFO << endreq << "Detailed dump of 1st event: " << endreq << endreq;
            int ndigi = 0;
            Event::TkrDigiCol::const_iterator pTkrDigi = digiCol->begin();
            for (; pTkrDigi!= digiCol->end(); pTkrDigi++) {
                log <<MSG::INFO << "Digi " << ndigi++ << " ";
                // output the digi proper:  " << digi " no longer works because
                // "digi" now returns an integer sort order
                (**pTkrDigi).fillStream(log.stream());
                log << endreq;
            }
        }
    }
    
    
    return sc;
}

//------------------------------------------------------------------------
//! clean up, summarize
StatusCode test_TkrDigi::finalize(){
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize after " << m_count << " calls." << endreq;
    
    return sc;
}



