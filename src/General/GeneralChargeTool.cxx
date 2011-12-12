/*
* @file GeneralChargeTool.cxx
*
* @brief Adds electronic noise to strips and noisy strips to the SiStripList.
* Uses TkrSimpleDigiAlg::addCharge() of TkrDigi v1r11p2.
*
* @author Michael Kuss
*
* $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/TkrDigi/src/General/GeneralChargeTool.cxx,v 1.2.22.1 2011/01/13 21:21:46 jrb Exp $
*/

#include "GeneralChargeTool.h"

#include "../SiPlaneMapContainer.h"
#include "idents/VolumeIdentifier.h"

// Gaudi specific include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SmartDataPtr.h"

#include <string>


//static const ToolFactory<GeneralChargeTool>    s_factory;
//const IToolFactory& GeneralChargeToolFactory = s_factory;
DECLARE_TOOL_FACTORY(GeneralChargeTool);

GeneralChargeTool::GeneralChargeTool(const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent) :
AlgTool(type, name, parent) {
    // Declare the additional interface
    declareInterface<IChargeTool>(this);

}

StatusCode GeneralChargeTool::initialize() {
    // Purpose and Method: initializes the tool
    // Inputs: None
    // Outputs: a status code
    // Dependencies: several services
    // Restrictions and Caveats: None

    // last 3 numbers are made up!
    double chargeFrac[nFrac] = 
        {0.0,     .027,      .0091,     .0041,     .00081, 
        .00029,   .000118,   .000039,   .000013,   .0000043}; 
        //.0000014, .00000046, .00000015, .00000005, .000000017};

    int i;
    double totFrac = 1.0;
    for(i=0;i<nFrac;++i) {
        totFrac += 2*chargeFrac[i];
    }
    for(i=0;i<nFrac;++i) {
        m_chargeFrac[i] = chargeFrac[i]*totFrac;
    }



    StatusCode sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize " << endreq;

    // Get the Glast detector service 
    sc = service("GlastDetSvc", m_gdSvc);
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "Couldn't set up GlastDetSvc!" << endreq;
        return sc;
    }

    IService* iService = 0;
    sc = serviceLocator()->service("EventDataSvc", iService, true );
    if ( sc.isFailure() ) {
        log << MSG::ERROR << "could not find EventDataSvc!" << endreq;
        return sc;
    }
    m_edSvc = dynamic_cast<IDataProviderSvc*>(iService);

    // Get the tracker ToT service 
    //sc = service("TkrToTSvc", m_gdSvc);
    //if ( sc.isFailure() ) {
    //    log << MSG::ERROR << "Couldn't set up the ToT service" << endreq;
    //   return sc;
    //}

    return sc;
}


StatusCode GeneralChargeTool::execute() {
    // Purpose and Method:  
    // Inputs: class variables
    // Outputs: additional hits in the Si
    // TDS Inputs: /Event/tmp/siPlaneMapContainer
    // TDS Outputs: /Event/tmp/siPlaneMapContainer
    // Dependencies: None
    // Restrictions and Caveats: The TDS output is a hard-coded string
    StatusCode sc = StatusCode::SUCCESS;

    MsgStream log(msgSvc(), name());

    // retrieve the pointer to the SiPlaneMapContainer from TDS
    SmartDataPtr<SiPlaneMapContainer> pObject(m_edSvc,
        "/Event/tmp/siPlaneMapContainer");
    if ( !pObject ) {
        log << MSG::ERROR
            << "could not retrieve /Event/tmp/siPlaneMapContainer" << endreq;
        return sc;
    }

    const int nStripsW = 384;
    const int nStrips = nStripsW*4;
    std::vector<double> eStrip;
    std::vector<bool> iStrip;

    double minE = .01*.113;

    SiPlaneMapContainer::SiPlaneMap& siPlaneMap = pObject->getSiPlaneMap();

    int istr;
    Event::McPositionHit* nullHit = 0;

    SiPlaneMapContainer::SiPlaneMap::iterator itMap=siPlaneMap.begin();
    for ( ; itMap!=siPlaneMap.end(); ++itMap ) { 
        //idents::VolumeIdentifier id = itMap->first;
        SiStripList* sList = itMap->second;
        SiStripList::iterator itStrip=sList->begin();
        eStrip.assign(nStrips, 0.0);
        iStrip.assign(nStrips, false);
        //std::cout << "Setting strips: " ;
        for (itStrip=sList->begin(); itStrip!=sList->end(); ++itStrip ) {
            //int status = itStrip->stripStatus();
            double energy = itStrip->energy();
            int stripNum = itStrip->index();
            iStrip[stripNum] = true;
            //std::cout << stripNum << " " ;
            int minStrip = nStripsW*(stripNum/nStripsW);
            int maxStrip = std::min(minStrip + nStripsW -1, nStripsW*4);
            int minStr = std::max(minStrip, stripNum-(nFrac-1));
            int maxStr = std::min(maxStrip, stripNum+(nFrac-1));
            for (istr=minStr; istr<=maxStr; ++istr) {
                if (istr==stripNum) continue;
                int offset = abs(stripNum-istr);
                double fracEnergy = energy*m_chargeFrac[offset];
                eStrip[istr] += fracEnergy;
            } // loop over adjacent strips
        } // loop over strips
        //std::cout << std::endl;
        /*
        // check the list again!
        std::cout << "check list: " ;
        for(istr=0; istr<nStrips; ++istr) {
            if (isStart[istr]) {
            //if (startList[istr]!=sList->end()) {
                int idx = startList[istr]->index();
                std::cout << idx << " "  << istr << " ";
            }
        }
        std::cout << std::endl;
        */
        //std::cout << "and again: " ;
        // here we add energy to existing strips or create new ones
        for(istr=0; istr<nStrips; ++istr) {
            double addedE = eStrip[istr];
            if (addedE<minE) continue;
            /*
            if (isStart[istr]) {
                std::cout << istr << " " << startList[istr]->index() << " ";
            //if(startList[istr]!=sList->end()) {
                // add the energy
                startList[istr]->addEnergy(addedE);
            } else {
            */
                sList->addStrip(istr, addedE, nullHit);
            //}
        }
        //std::cout << std::endl;
    } // loop over stripLists

    return sc;
}
