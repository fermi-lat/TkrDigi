// File and Version Information:
//      $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/TkrSimpleDigiAlg.cxx,v 1.2 2002/06/20 22:23:38 burnett Exp $
//
// Description:
//      TkrSimpleDigiAlg provides an example of a Gaudi algorithm.  
//      These comments will not be processed by Doxygen - but are here for 
//      developers to identify this source file.
//      Note the format for the comment header for all major methods:
//          Purpose and Method:
//          Inputs:
//          Outputs:
//          TDS Inputs:
//          TDS Outputs:
//          Dependencies:
//          Restrictions and Caveats:
//
//      Note that those sections of the method comment headers that are unused,
//      may be omitted
//
// Author(s):
//      T. Burnett		
//     	

// [Gaudi system includes]
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Algorithm.h"

#include "idents/VolumeIdentifier.h"
#include "idents/TowerId.h"
#include <map>
/// Glast specific includes
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/Digi/TkrDigi.h"

#include "SiStripList.h"

#include "Event/MonteCarlo/McPositionHit.h"

#include <cassert>
/** 
* @class TkrSimpleDigiAlg
*
* @brief A  Gaudi algorithm that creates digis for the tracker.  
*
*
* @author T. Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/TkrSimpleDigiAlg.cxx,v 1.2 2002/06/20 22:23:38 burnett Exp $  
*/

class TkrSimpleDigiAlg : public Algorithm {
public:
    // [required Gaudi Algorithm constructor]
    TkrSimpleDigiAlg(const std::string& name, ISvcLocator* pSvcLocator);
    
    /** 
    * Describe anything special about this algorithm's initialization here.
    */
    StatusCode initialize();
    
    /** 
    * Describe the specifics of the execute routine for this algorithm
    */
    StatusCode execute();
    
    /**
    * Describe anything special about the finalize routine for this algorithm 
    * here.
    */
    StatusCode finalize();
    
    typedef std::map<idents::VolumeIdentifier, SiStripList*> SiPlaneMap;
    
private: 
    SiPlaneMap m_SiMap;
    void createSiHits(const Event::McPositionHitVector& hits);
    void clear();
    IGlastDetSvc * m_gsv;

	/// energy deposit above which hit is recorded (MeV)
	double m_threshold;
	/// amount to fluctuate hit strips (MeV)
	double m_noiseSigma;
	/// frequency of noise hits
	double m_noiseOccupancy;    
};

static const AlgFactory<TkrSimpleDigiAlg>  Factory;
const IAlgFactory& TkrSimpleDigiAlgFactory = Factory;

TkrSimpleDigiAlg::TkrSimpleDigiAlg(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator)
{
    declareProperty("threshold",      m_threshold     = 0.030 );  
	declareProperty("noiseSigma",     m_noiseSigma    = 0.010 );
	declareProperty("noiseOccupancy", m_noiseOccupancy= 1.e-4 );
}

StatusCode TkrSimpleDigiAlg::initialize(){
    // Purpose and Method:  This routine is an example initialize routine for a
    //     Gaudi algorithm.  It is called once before event processing begins.
    // Inputs:  None
    // Outputs:  A StatusCode which denotes success or failure.
    // TDS Inputs:  None
    // TDS Outputs: TrkDigi
    // Dependencies: None
    // Restrictions and Caveats:  None
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "initialize TkrSimpleDigiAlg" << endreq;
    
    // [Use the job options service to set the algorithm's parameters]
    setProperties();
    
    // Get the Glast detector service 
    m_gsv=0;
    if( service( "GlastDetSvc", m_gsv).isFailure() ) {
        log << MSG::ERROR << "Couldn't set up GlastDetSvc!" << endreq;
        return StatusCode::FAILURE;
    }
    
    // pass the GlastDetSvc pointer to the SiStripList static functions
    SiStripList::initialize(m_gsv);
    
    return sc;
}

StatusCode TkrSimpleDigiAlg::execute()
{
    // Purpose and Method:  do it!
    // Inputs:  None
    // Outputs:  A StatusCode which denotes success or failure.
    // TDS Inputs:  /Event/MC/PositionHitsCol
    // TDS Outputs:  /Event/TkrRecon/TkrDigi
    // Dependencies: None
    // Restrictions and Caveats:  None
    
    using namespace Event;
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream   log( msgSvc(), name() );
    //Look to see if the McPositionHitVector object is in the TDS
    SmartDataPtr<Event::McPositionHitVector> mcHits(eventSvc(), EventModel::MC::McPositionHitCol);
    
    //If the McPositionHitVector doesn't exist
    if( 0 == mcHits)  { 
        log << MSG::DEBUG << "could not find \""<< "EventModel::MC::McPositionHitCol" <<"\"" << endreq;
        return  sc;
    }
    
    // now create the Si hits
    createSiHits(mcHits);
    
    
    
    //Take care of insuring that data area has been created
    DataObject* pNode = 0;
    sc = eventSvc()->retrieveObject( EventModel::Digi::Event, pNode);
    
    if (sc.isFailure()) {
        sc = eventSvc()->registerObject(EventModel::Digi::Event,new DataObject);
        if( sc.isFailure() ) {
            log << MSG::ERROR << "could not register "<< EventModel::Digi::Event << endreq;
            return sc;
        }
    }
    
    //Create the collection of digi objects - will be empty at this point
    TkrDigiCol*  pTkrDigi   = new TkrDigiCol;
    
    /// Add this new collection to the TDS - indicating where it belongs in the tree via the 
    /// first parameter.
    sc = eventSvc()->registerObject( EventModel::Digi::TkrDigiCol, pTkrDigi);
    
    if (sc != StatusCode::SUCCESS){
        log << MSG::INFO << "Failed to register TKR digi vector" << endreq;
        return sc;
    }
    
    // finally make digis from the hits
    for(SiPlaneMap::iterator si = m_SiMap.begin(); si != m_SiMap.end(); ++si){
        SiStripList& sidet = *si->second;
        idents::VolumeIdentifier id = si->first;  

        sidet.addNoise(m_noiseSigma, m_noiseOccupancy, m_threshold); 
        
        // unpack the id: the order is correct!
#if 0 //  old way
        unsigned int towery=id[1], towerx= id[2],  tray=id[4], view=id[5],
            botTop=id[6], layer = tray+botTop-1;
#else // do it from the bottom
        int s = id.size()-2;   // last two fields are ladder, wafer
        assert(s>3);
        unsigned int 
            botTop=id[--s],    // 6 with full LAT
            view  =id[--s],    // 5
            tray = id[--s],    // 4
            layer = tray+botTop-1;
        --s; // skip
        unsigned int 
            towerx=s>0? id[--s] : 0, //2 with full LAT
            towery=s>0? id[--s] : 0; //1
#endif
        
        int ToT[2]={0,0};
        
        idents::TowerId tower = idents::TowerId(towerx, towery).id();
        idents::GlastAxis::axis iview = (view==0 ? idents::GlastAxis::X : idents::GlastAxis::Y);
        
        TkrDigi* pDigi  = new TkrDigi(layer, iview, tower, ToT);
        
        pTkrDigi->push_back(pDigi);
        
        // now loop over contained list of strips
        for(SiStripList::const_iterator i=sidet.begin(); i!=sidet.end(); ++i) {
            const SiStripList::Strip & strip = *i;
            int stripId = strip.index();
            float e = strip.energy();
            
            //TODO: apply threshold, add strip to TkrDigi
            pDigi->addC0Hit(stripId);
            
        }
    }
    
    return sc;
}

StatusCode TkrSimpleDigiAlg::finalize(){
    // Purpose and Method:  This routine is an example finalize routine for a
    //     Gaudi algorithm.  Finalize will be called when event processing is
    //     completed.
    // Inputs:  None
    // Outputs:  A StatusCode which denotes success or failure.
    // TDS Inputs:  None
    // TDS Outputs:  None
    // Dependencies: None
    // Restrictions and Caveats:  None
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream log(msgSvc(), name());
    
    clear();
    
    return sc;
}

void TkrSimpleDigiAlg::createSiHits(const Event::McPositionHitVector& hits)
{
    // Purpose and Method: generate a list of hit strips
    // Inputs:  
    // Outputs:  the map of SiStripList objects
    // Dependencies: None
    // Restrictions and Caveats:  None
    MsgStream   log( msgSvc(), name() );
    
    log << MSG::DEBUG << "Number of position hits found = " << hits.size() << endreq;
    
    clear();
    
    for(Event::McPositionHitVector::const_iterator ihit=hits.begin(); ihit != hits.end(); ++ihit) {
        const Event::McPositionHit & hit = **ihit;
        
        idents::VolumeIdentifier id = hit.volumeID();
        // siPlaneCoord expects size to be 9
        int s = id.size(); 
        if( s < 9 ) {
            idents::VolumeIdentifier tid;
            for( int j = 0; j<9; ++j) {
                if( j< 9-s) 
                    tid.append(0); else tid.append( id[j-9+s]);
            }
            id = tid;
        }
        
#if 1
    static double ladder_pitch = SiStripList::die_width()+SiStripList::ladder_gap();
    static double ssd_pitch    = SiStripList::die_width()+SiStripList::ssd_gap();

        int ladder = id[7], wafer=id[8];
        Hep3Vector 
            offset( (ladder-1.5)*ladder_pitch, (wafer-1.5)*ssd_pitch, 0),
            entry(hit.entryPoint()+offset),
            exit( hit.exitPoint()+offset);
#else
        
        HepPoint3D entry = m_gsv->siPlaneCoord( hit.entryPoint(), id);
        HepPoint3D exit  = m_gsv->siPlaneCoord( hit.exitPoint(),  id);
#endif        
        
        if( m_SiMap.find(id) == m_SiMap.end()) m_SiMap[id]= new SiStripList;
        m_SiMap[id]->score(entry, exit, hit.depositedEnergy());
    }
}

void TkrSimpleDigiAlg::clear()
{
    // clear last event
    for(SiPlaneMap::iterator si = m_SiMap.begin(); si != m_SiMap.end(); ++si){
        delete (*si).second;
    }
    m_SiMap.clear();
    
}
