// File and Version Information:
//      $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/TkrSimpleDigiAlg.cxx,v 1.24 2003/02/07 23:36:26 lsrea Exp $
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

#include "TkrUtil/ITkrFailureModeSvc.h"
#include "TkrUtil/ITkrBadStripsSvc.h"
#include "TkrUtil/ITkrAlignmentSvc.h"
#include "TkrUtil/ITkrGeometrySvc.h"

#include "idents/VolumeIdentifier.h"
#include "idents/TowerId.h"
#include <map>
/// Glast specific includes
#include "Event/TopLevel/EventModel.h"
#include "Event/TopLevel/Event.h"
#include "Event/Digi/TkrDigi.h"
#include "Event/MonteCarlo/McTkrStrip.h"
#include "Event/RelTable/RelTable.h"
#include "Event/RelTable/Relation.h"

#include "facilities/Util.h"


#include "SiStripList.h"
#include "SiLayerList.h"

#include "Event/MonteCarlo/McPositionHit.h"

#include <cassert>
#include <algorithm>

/** 
* @class TkrSimpleDigiAlg
*
* @brief A  Gaudi algorithm that creates digis for the tracker.  
*
*
* @author T. Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/TkrSimpleDigiAlg.cxx,v 1.24 2003/02/07 23:36:26 lsrea Exp $  
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
    * Creates the output
    */
    StatusCode execute();
    
    StatusCode finalize();
    
private: 
/**
*  This map assoicates the SiPlane volume Id with a list of hit strips.
    */ 
    typedef std::map<idents::VolumeIdentifier, SiStripList*> SiPlaneMap;
    
    SiPlaneMap m_SiMap;
    
    void createSiHits(const Event::McPositionHitVector& hits);
    void addNoise();
    
    void clear();
    IGlastDetSvc *       m_gsv;
    ITkrGeometrySvc *    m_tgsv;

    ITkrFailureModeSvc * m_fsv;
    ITkrBadStripsSvc *   m_bsv;
    ITkrAlignmentSvc *   m_asv;
    
    SiLayerList m_layers;
    
    /// energy deposit above which hit is recorded (MeV)
    double m_threshold;
    /// amount to fluctuate hit strips (MeV)
    double m_noiseSigma;
    /// frequency of noise hits
    double m_noiseOccupancy;  
    /// average tot response to a MIP
    double m_totAt1Mip;
    /// average eloss in silicon per MIP (to calculate ToT from eloss
    double m_mevPerMip;
    /// threshold for ToT (in Mips)
    double m_totThreshold;
    /// if true, kill bad strips in digi
    bool   m_killBadStrips;
    /// if true, kill failed layers and towers in digi
    bool   m_killFailed;
};

static const AlgFactory<TkrSimpleDigiAlg>  Factory;
const IAlgFactory& TkrSimpleDigiAlgFactory = Factory;

TkrSimpleDigiAlg::TkrSimpleDigiAlg(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator)
{
    declareProperty("threshold",      m_threshold     = 0.030 );  
    declareProperty("noiseSigma",     m_noiseSigma    = 0.010 );
    declareProperty("noiseOccupancy", m_noiseOccupancy= 1.e-5 );
    declareProperty("totAt1Mip"     , m_totAt1Mip     = 52.5  );
    declareProperty("mevPerMip"     , m_mevPerMip     = 0.155 );
    declareProperty("totThreshold"  , m_totThreshold  = 0.1   );
    declareProperty("killBadStrips" , m_killBadStrips = false );
    declareProperty("killFailed"    , m_killFailed    = true  );
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

    // Get the Glast detector service 
    m_tgsv=0;
    if( service( "TkrGeometrySvc", m_tgsv).isFailure() ) {
        log << MSG::ERROR << "Couldn't set up TkrGeometrySvc!" << endreq;
        return StatusCode::FAILURE;
    }

   // Get the failure mode service 
    m_fsv=0;
    if( service( "TkrFailureModeSvc", m_fsv).isFailure() ) {
        log << MSG::INFO << "Couldn't set up TkrFailureModeSvc" << endreq;
        log << MSG::INFO << "Will assume it is not required"    << endreq;
    }

    // Get the alignment service 
    m_asv=0;
    if( service( "TkrAlignmentSvc", m_asv).isFailure() ) {
        log << MSG::INFO << "Couldn't set up TkrAlignmentSvc" << endreq;
        log << MSG::INFO << "Will assume it is not required"    << endreq;
    }

    m_bsv=0;
    if( service( "TkrBadStripsSvc", m_bsv).isFailure() ) {
        log << MSG::INFO << "Couldn't set up TkrBadStripsSvc" << endreq;
        log << MSG::INFO << "Will assume it is not required"    << endreq;
    }

    // pass the GlastDetSvc pointer to the SiStripList static functions
    if (SiStripList::initialize(m_gsv).isFailure() ) {
        log << MSG::ERROR << "Couldn't initialize SIStripList" << endreq;
        return StatusCode::FAILURE;
    }
    
    log << MSG::INFO << "ssdgap " << SiStripList::ssd_gap() 
        << " laddergap " << SiStripList::ladder_gap()
        << " strips/wafer " << SiStripList::strips_per_die() 
        << " #ladders " << SiStripList::n_si_dies() 
        << " waferside " << SiStripList::die_width() 
        << " deadgap " << SiStripList::guard_ring() 
        << endreq;
    
    // get the list of layers, to be used to add noise to otherwise empty layers
    m_layers.setPrefix(m_gsv->getIDPrefix());
    
    m_gsv->accept(m_layers);
    log << MSG::INFO << "will add noise to "<< m_layers.size() << " Si layers, ids from "
        << m_layers.front().name() << " to " << m_layers.back().name() << endreq;
#if 0
    for( SiLayerList::const_iterator it = m_layers.begin(); it!=m_layers.end(); ++it){
        std::cout << it->name() << std::endl;
    }
#endif
    
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

    double totFactor = m_totAt1Mip/(1. - m_totThreshold);
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream   log( msgSvc(), name() );
    //Look to see if the McPositionHitVector object is in the TDS
    SmartDataPtr<Event::McPositionHitVector> mcHits(eventSvc(), EventModel::MC::McPositionHitCol);
    
    //If the McPositionHitVector doesn't exist
    if( 0 == mcHits)  { 
        log << MSG::DEBUG;
        if (log.isActive() ) {
            log << "could not find \""<< "EventModel::MC::McPositionHitCol" 
                <<"\"";
        }
        log << endreq;
        return  sc;
    }
    
    // create full map
    
    // now create the Si hits
    createSiHits(mcHits);
    
    addNoise();
    
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
    // Create the collection of hit strip objects
    McTkrStripCol* strips = new McTkrStripCol;
    sc = eventSvc()->registerObject( EventModel::MC::McTkrStripCol, strips);
    
    if (sc != StatusCode::SUCCESS){
        log << MSG::INFO << "Failed to register TkrStrip vector" << endreq;
        return sc;
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
    
    // Create the relational table
    typedef Event::Relation<Event::TkrDigi,Event::McPositionHit> relType;
    typedef ObjectList<relType> tabType;
    
    Event::RelTable<Event::TkrDigi, Event::McPositionHit> digiHit;
    digiHit.init();
    tabType* pRelTab = digiHit.getAllRelations();
    
    sc = eventSvc()->registerObject(EventModel::Digi::TkrDigiHitTab, pRelTab);
    if (sc.isFailure()) {
        log << MSG::ERROR << "Registration of digiHit Failed!" << endreq; 
        return sc;
    }
    
    // finally make digis from the hits
    for(SiPlaneMap::iterator si = m_SiMap.begin(); si != m_SiMap.end(); ++si){
        SiStripList& sidet = *si->second;
        idents::VolumeIdentifier id = si->first;  
        
        
        // unpack the id: the order is correct!
        
        unsigned int towery=id[1], towerx= id[2],  tray=id[4], view=id[5],
            botTop=id[6], layer = tray+botTop-1;
        int ToT[2]={0,0};
        
        idents::TowerId tower = idents::TowerId(towerx, towery);
        idents::GlastAxis::axis iview = (view==0 ? idents::GlastAxis::X : idents::GlastAxis::Y);
        int theTower = tower.id();

        if (m_killFailed && m_fsv && !m_fsv->empty() 
            && m_fsv->isFailed(theTower, layer, view)) continue;
        
        TkrDigi* pDigi  = new TkrDigi(layer, iview, tower, ToT);
        
        //pTkrDigi->push_back(pDigi); // don't add to TDS until we know there are good hits

        int nStrips = 0;
        
        // now loop over contained list of strips
        for(SiStripList::const_iterator i=sidet.begin(); i!=sidet.end(); ++i) {
            const SiStripList::Strip & strip = *i;
            int stripId = strip.index();
            // kill bad strips
            if(m_killBadStrips && m_bsv && !m_bsv->empty() 
                && m_bsv->isBadStrip(theTower, layer, iview, stripId)) continue;
            nStrips++;
            float e = strip.energy();
            bool noise  = strip.noise();
            const SiStripList::hitList& hits = strip.getHits();
            
            // add the strip with the correct controller number
            // and do the ToT
            int thisToT = (e/m_mevPerMip - m_totThreshold)*totFactor;
            if (stripId<SiStripList::n_si_strips()/2) {
                pDigi->addC0Hit(stripId, thisToT);
            } else {
                pDigi->addC1Hit(stripId, thisToT);
            }
           
            // get the alignment offset of the strip
            HepPoint3D point(0);
            double deltaX = 0;
            double deltaY = 0;
            if( m_asv && m_asv->alignSim()) {
                HepPoint3D entry(0., 0., 0.);
                HepPoint3D  exit(0., 0., 1.);
                m_asv->moveMCHit(id, entry, exit);
                deltaX = -entry.x();
                deltaY = -entry.y();
            }

            // save the hit here
            Event::McTkrStrip* pStrip = 
                new Event::McTkrStrip(id, stripId, e, noise, hits, deltaX, deltaY);
            strips->push_back(pStrip);
            
            // and add the relation
            std::string stripString;
            facilities::Util::itoa(stripId, stripString);
            // right adjust... is there a better way?
            while (stripString.length()<4) {stripString = " "+stripString;}

            Event::McTkrStrip::hitList::const_iterator itH;
                
            for (itH = pStrip->begin(); itH!=pStrip->end(); itH++) {
                Event::McPositionHit* pHit = *itH;
                relType *rel = new relType(pDigi, pHit);
                rel->addInfo(stripString);
                //addRelation now does the right thing with duplicates
                // namely, appends the info to the existing info
                digiHit.addRelation(rel);
            }
        }
        if (nStrips>0) {
            pTkrDigi->push_back(pDigi);
        } else {
            delete pDigi;
        }
    }
    // sort by tower, layer, view
    std::sort(pTkrDigi->begin(), pTkrDigi->end(), Event::TkrDigi::digiLess());
        
    return sc;
}

void TkrSimpleDigiAlg::addNoise(){
    // Purpose and Method:  add noise hits
    // Inputs:  class variables, especially m_layers,the list of si plane ids.
    // Outputs:  additional hits in the Si
    // TDS Inputs:  None 
    // TDS Outputs:  None
    // Dependencies: None
    // Restrictions and Caveats:  None
    
    int noiseCount=0;
    
    // loop over list of possible layer ids
    for(SiLayerList::const_iterator it=m_layers.begin(); it!=m_layers.end(); ++it){
        idents::VolumeIdentifier id = *it;
        if( m_SiMap.find(id) == m_SiMap.end()){
            
            SiStripList* sidet = new SiStripList;
            sidet->addNoise(m_noiseSigma, m_noiseOccupancy, m_threshold);
            if(sidet->size()>0){
                m_SiMap[id]= sidet;
                noiseCount += sidet->size();
            }
            else delete sidet;
            
        }else{
            noiseCount -= m_SiMap[id]->size();
            m_SiMap[id]->addNoise(m_noiseSigma, m_noiseOccupancy, m_threshold);
            noiseCount += m_SiMap[id]->size();
        }       
    }
    
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG;
    if (log.isActive() ) {
        log << "added " << noiseCount <<" noise hits";
    }
    log << endreq;
    
    
}
StatusCode TkrSimpleDigiAlg::finalize(){
    // Purpose and Method:  clear accumulated data at the end of the run
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
    
    log << MSG::DEBUG;
    if (log.isActive() ) {
        log << "Number of position hits found = " << hits.size();
    }
    log << endreq;
    
    clear();
    
    for(Event::McPositionHitVector::const_iterator ihit=hits.begin(); ihit != hits.end(); ++ihit) {
        const Event::McPositionHit & hit = **ihit;
        
        idents::VolumeIdentifier id = hit.volumeID();
        
        // check for correct length
        if (id.size()!=9) continue;
        // check that it's really a TKR hit (probably overkill)
        if (!(id[0]==0 && id[3]==1)) continue; // !(LAT && TKR)
        
        // this assumes that the number of ladders equals the number of wafers/ladder
        // not true for the BFEM/BTEM!
        static double ladder_pitch = SiStripList::die_width()+SiStripList::ladder_gap();
        static double ssd_pitch    = SiStripList::die_width()+SiStripList::ssd_gap();
        static double waferOffset = 0.5*(SiStripList::n_si_dies() - 1);
        
        int ladder = id[7], wafer=id[8];
        HepVector3D 
            offset( (ladder-waferOffset)*ladder_pitch, (wafer-waferOffset)*ssd_pitch, 0);
        HepPoint3D
            entry(hit.entryPoint()+offset),
            exit( hit.exitPoint()+offset);
        // move hit by alignment constants
        if( m_asv && m_asv->alignSim()) m_asv->moveMCHit(id, entry, exit);
        
        //now truncate the id to the plane.
        idents::VolumeIdentifier planeId;
        for( int j=0; j<7; ++j) planeId.append(id[j]);
        if( m_SiMap.find(planeId) == m_SiMap.end()) m_SiMap[planeId]= new SiStripList;
        m_SiMap[planeId]->score(entry, exit, *ihit);
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


























    
