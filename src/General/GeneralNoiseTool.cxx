/*
 * @file GeneralNoiseTool.cxx
 *
 * @brief Adds electronic noise to strips and noisy strips to the SiStripList.
 * Uses TkrSimpleDigiAlg::addNoise() of TkrDigi v1r11p2.
 *
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/src/General/GeneralNoiseTool.cxx,v 1.4.142.1 2011/01/13 21:21:46 jrb Exp $
 */

#include "GeneralNoiseTool.h"

#include "../SiPlaneMapContainer.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include <string>


//static const ToolFactory<GeneralNoiseTool>    s_factory;
//const IToolFactory& GeneralNoiseToolFactory = s_factory;
DECLARE_TOOL_FACTORY(GeneralNoiseTool);


// 1/4 mip, was .03
//double GeneralNoiseTool::s_noiseThreshold = 0.03875;
// to match pdr number, was .01
//double GeneralNoiseTool::s_noiseSigma     = 0.00698;
// to match pdr number, was 1.e-5
//double GeneralNoiseTool::s_noiseOccupancy = 5.e-5;


GeneralNoiseTool::GeneralNoiseTool(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent) :
    AlgTool(type, name, parent) {
    // Declare the additional interface
    declareInterface<INoiseTool>(this);

    // Declaring the properties.  Attention: the variables are static!
    declareProperty("dataThreshold",    m_noiseThreshold = 0.03875);
    declareProperty("triggerThreshold", m_trigThreshold = 0.03875);
    declareProperty("sigma",            m_noiseSigma = 0.00698);
    // set occupancy to a more realistic level
    // but the real fix is overlays
    declareProperty("occupancy",        m_noiseOccupancy = 5.e-6);
    declareProperty("fullThreshold",    m_fullThreshold = false);
}


StatusCode GeneralNoiseTool::initialize() {
    // Purpose and Method: initializes the tool
    // Inputs: None
    // Outputs: a status code
    // Dependencies: several services
    // Restrictions and Caveats: None

    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize " << endreq;

    // Get the Glast detector service 
    sc = service("GlastDetSvc", m_gdSvc);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up GlastDetSvc!" << endreq;
        return sc;
    }

    // Get the tracker ToT service 
    //sc = service("TkrToTSvc", m_gdSvc);
    //if ( sc.isFailure() ) {
    //    log << MSG::ERROR << "Couldn't set up the ToT service" << endreq;
    //   return sc;
    //}

    // get the list of layers, to be used to add noise to otherwise empty layers
    m_layers.setPrefix(m_gdSvc->getIDPrefix());
    m_gdSvc->accept(m_layers);
    log << MSG::INFO << "will add noise to "<< m_layers.size()
        << " Si layers, ids from " << m_layers.front().name() << " to "
        << m_layers.back().name() << endreq;

    IService* iService = 0;
    sc = serviceLocator()->service("EventDataSvc", iService, true );
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc!" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);

    return sc;
}


StatusCode GeneralNoiseTool::execute() {
    // Purpose and Method:  add noise hits to the SiPlaneMap
    // Inputs: class variables, especially m_layers, the list of si plane ids.
    // Outputs: additional hits in the Si
    // TDS Inputs: /Event/tmp/siPlaneMapContainer
    // TDS Outputs: /Event/tmp/siPlaneMapContainer
    // Dependencies: None
    // Restrictions and Caveats: The TDS output is a hard-coded string
    
    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "execute " << endreq;

    // retrieve the pointer to the SiPlaneMapContainer from TDS
    SmartDataPtr<SiPlaneMapContainer> pObject(m_edSvc,
                                              "/Event/tmp/siPlaneMapContainer");
    if ( !pObject ) {
        log << MSG::ERROR
            << "could not retrieve /Event/tmp/siPlaneMapContainer" << endreq;
        return sc;
    }

    SiPlaneMapContainer::SiPlaneMap& siPlaneMap = pObject->getSiPlaneMap();

    int noiseCount = 0;

    // loop over list of possible layer ids
    for ( SiLayerList::const_iterator it=m_layers.begin(); it!=m_layers.end();
          ++it ) {
        idents::VolumeIdentifier id = *it;
        if ( siPlaneMap.find(id) == siPlaneMap.end() ) {
            SiStripList* siPlane = new SiStripList;
            noiseCount += siPlane->addNoise(m_noiseSigma, m_noiseOccupancy,
                                            m_noiseThreshold, m_trigThreshold);
            if ( siPlane->size() > 0 )
                siPlaneMap[id] = siPlane;
            else
                delete siPlane;
        }
        else
            noiseCount += siPlaneMap[id]->addNoise(m_noiseSigma,
                                                   m_noiseOccupancy,
                                                   m_noiseThreshold,
                                                   m_trigThreshold);
    }

    log << MSG::DEBUG << "added " << noiseCount <<" noise hits" << endreq;

    return sc;
}
