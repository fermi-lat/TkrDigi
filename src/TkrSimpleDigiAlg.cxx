// File and Version Information:
//      $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/TkrSimpleDigiAlg.cxx,v 1.12 2002/10/08 22:25:01 lsrea Exp $
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
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/TkrSimpleDigiAlg.cxx,v 1.12 2002/10/08 22:25:01 lsrea Exp $  
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
    IGlastDetSvc * m_gsv;
    
    SiLayerList m_layers;
    
    /// energy deposit above which hit is recorded (MeV)
    double m_threshold;
    /// amount to fluctuate hit strips (MeV)
    double m_noiseSigma;
    /// frequency of noise hits
    double m_noiseOccupancy;  
    /// average tot response to a MIP
    double m_totPerMip;
    /// average eloss in silicon per MIP (to calculate ToT from eloss
    double m_mevPerMip;
};

static const AlgFactory<TkrSimpleDigiAlg>  Factory;
const IAlgFactory& TkrSimpleDigiAlgFactory = Factory;

TkrSimpleDigiAlg::TkrSimpleDigiAlg(const std::string& name, ISvcLocator* pSvcLocator)
:Algorithm(name, pSvcLocator)
{
    declareProperty("threshold",      m_threshold     = 0.030 );  
    declareProperty("noiseSigma",     m_noiseSigma    = 0.010 );
    declareProperty("noiseOccupancy", m_noiseOccupancy= 1.e-5 );
    declareProperty("totPerMip"     , m_totPerMip     = 300.  );
    declareProperty("mevPerMip"     , m_mevPerMip     = 0.155 );
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
    
    StatusCode  sc = StatusCode::SUCCESS;
    MsgStream   log( msgSvc(), name() );
    //Look to see if the McPositionHitVector object is in the TDS
    SmartDataPtr<Event::McPositionHitVector> mcHits(eventSvc(), EventModel::MC::McPositionHitCol);
    
    //If the McPositionHitVector doesn't exist
    if( 0 == mcHits)  { 
        log << MSG::DEBUG << "could not find \""<< "EventModel::MC::McPositionHitCol" <<"\"" << endreq;
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
    
    //std::cout << "pointer to table: " << pRelTab << std::endl;
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
        
        idents::TowerId tower = idents::TowerId(towerx, towery).id();
        idents::GlastAxis::axis iview = (view==0 ? idents::GlastAxis::X : idents::GlastAxis::Y);
        
        TkrDigi* pDigi  = new TkrDigi(layer, iview, tower, ToT);
        
        pTkrDigi->push_back(pDigi);
        
        // now loop over contained list of strips
        for(SiStripList::const_iterator i=sidet.begin(); i!=sidet.end(); ++i) {
            const SiStripList::Strip & strip = *i;
            int stripId = strip.index();
            float e = strip.energy();
            bool noise  = strip.noise();
            const SiStripList::hitList& hits = strip.getHits();
            
            // add the strip with the correct controller number'
            // and do the ToT
            int thisToT = e/m_mevPerMip*m_totPerMip;
            if (stripId<SiStripList::n_si_strips()/2) {
                pDigi->addC0Hit(stripId, thisToT);
            } else {
                pDigi->addC1Hit(stripId, thisToT);
            }
            
            // save the hit here
            Event::McTkrStrip* pStrip = 
                new Event::McTkrStrip(id, stripId, e, noise, hits);
            strips->push_back(pStrip);
            
            // and add the relation
            std::string stripString;
            facilities::Util::itoa(stripId, stripString);
            // right adjust... is there a better way?
            while (stripString.length()<4) {
                stripString = " "+stripString;
            }
            
            Event::McTkrStrip::hitList::const_iterator ihit;
            Event::McPositionHit* pHit= 0;
            int nHits = pStrip->getHits().size();
            
            bool found1 = false;
            if (nHits>0) {
                for (ihit = pStrip->begin(); ihit!=pStrip->end(); ihit++) {
                    Event::McPositionHit* pHit = *ihit;

                    // now check to see if we have it already
                    if(digiHit.size()>0){
                        std::vector<relType*> byFirst;
                        byFirst = digiHit.getRelByFirst(pDigi);
                        int vSize = byFirst.size();
                        for (int ir = 0; ir<vSize; ir++) {
                            relType* rel = byFirst[ir];
                            if ( rel->getSecond()==pHit) {
                                found1 = true;
                                rel->addInfo(stripString);
                                //std::cout << "Info added to existing relation: " 
                                //    << pDigi << " " << pHit << std::endl;
                                break;
                            }
                        }
                    }
                    // if not, add it
                    if (!found1) {                          
                        relType *rel = new relType(pDigi, pHit);
                        rel->addInfo(stripString);
                        digiHit.addRelation(rel);
                        
                        //std::cout << "Regular relation added: " 
                        //    << pDigi << " " << pHit << std::endl;
                    }
                }
            }
        }

        // sort by tower, layer, view
        std::sort(pTkrDigi->begin(), pTkrDigi->end(), Event::TkrDigi::digiLess());
              
        
    }
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
    log << MSG::DEBUG << "added " << noiseCount <<" noise hits" << endreq;
    
    
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
    
    log << MSG::DEBUG << "Number of position hits found = " << hits.size() << endreq;
    
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
        Hep3Vector 
            offset( (ladder-waferOffset)*ladder_pitch, (wafer-waferOffset)*ssd_pitch, 0),
            entry(hit.entryPoint()+offset),
            exit( hit.exitPoint()+offset);       
        
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
