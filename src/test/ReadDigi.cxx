#define GlastDigi_ReadDigi_CPP 

// Include files
#include "ReadDigi.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "GaudiKernel/ObjectVector.h"


static const AlgFactory<ReadDigi>  Factory;
const IAlgFactory& ReadDigiFactory = Factory;

/// Algorithm parameters which can be set at run time must be declared.
/// This should be done in the constructor.
ReadDigi::ReadDigi(const std::string& name, ISvcLocator* pSvcLocator) :
Algorithm(name, pSvcLocator) {

}

/*! The initialize method will be called once at the very beginning of the run,
while everything is being initialized before the event loop begins.
Any setup necessary will be done here.
*/
StatusCode ReadDigi::initialize() {
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize" << endreq;
    
    // Use the Job options service to set the Algorithm's parameters
    setProperties();

    return StatusCode::SUCCESS;
}


/*! The execute method will be called once per event.
*/
StatusCode ReadDigi::execute() {
    using namespace Event;
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream   log( msgSvc(), name() );
    log << MSG::INFO << "execute" << endreq;


    return sc;
}


/*
Finalize is called once at the end of event processing.
Any cleanup to occur - will occur here.
*/
StatusCode ReadDigi::finalize() {
    
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "finalize" << endreq;
    
    return StatusCode::SUCCESS;
}






