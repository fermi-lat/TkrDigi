/**
 * @file SiStripList.cxx
 *
 * @brief Provides means of filling and retrieving a list of strips.
 *
 * @author Toby Burnett, Leon Rochester (original authors)
 * @author Michael Kuss
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/SiStripList.cxx,v 1.22 2004/02/24 13:57:32 kuss Exp $
 */

#include "SiStripList.h"
#include "General/GeneralNoiseTool.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandBinomial.h"

#include <algorithm>
#include <float.h>
#include <limits.h>  // <limits> exist in gcc3.2, but not gcc2.95.3


// utility function declarations
#ifdef _MSC_VER
inline double copysign(double a, double b){return _copysign(a,b);}
#endif


// static variable implementations--now initialized with detsvc.

IGlastDetSvc* SiStripList::s_detSvc = 0;
int     SiStripList::s_n_si_dies     = 0;   // 4;
int     SiStripList::s_stripPerWafer = 0;   // 384;
double  SiStripList::s_die_width     = 0.0; // 89.500;
double  SiStripList::s_guard_ring    = 0.0; // 0.974;
double  SiStripList::s_ssd_gap       = 0.0; // 0.025;            
double  SiStripList::s_ladder_gap    = 0.0; // 0.200;


StatusCode SiStripList::initialize(IGlastDetSvc* ds) {
    // Purpose and Method: initializes the static members from the detector svc
    // Inputs: pointer to the detector service
    // Outputs: a status code
    // Dependencies: the detector service has to be initialized first

    s_detSvc = ds;
    double temp;

    if ( s_detSvc->getNumericConstByName("nWaferAcross",
                                         &s_n_si_dies).isFailure()
        || s_detSvc->getNumericConstByName("stripPerWafer",
                                           &s_stripPerWafer).isFailure()
        || s_detSvc->getNumericConstByName("SiWaferSide",
                                           &s_die_width).isFailure()
        || s_detSvc->getNumericConstByName("SiWaferActiveSide",
                                           &temp).isFailure()
        || s_detSvc->getNumericConstByName("ssdGap", &s_ssd_gap).isFailure()
        || s_detSvc->getNumericConstByName("ladderGap",
                                           &s_ladder_gap).isFailure()
        )
        return StatusCode::FAILURE;
    s_guard_ring = 0.5 * (s_die_width - temp);

    return StatusCode::SUCCESS;
}


#ifndef TEMPLATE
void SiStripList::addStrip(const int strip, double dE,
                           const hitList* hits,
                           int t1, int t2) {
    // Purpose and Method: adds a strip to the list of strips.  To add all
    //                     McPositionHits, addStrip is called several times.  To
    //                     avoid that the energy is added each time, it is set
    //                     to 0 after the first call.
    // Inputs: strip id, energy deposit, vector of McPositionHits, ToT start and
    //         stop time
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: Icky coding.  Actually, it would be better if
    //                           addStrip would be a template for both a hit and
    //                           a list of hits.

    for ( hitList::const_iterator it=hits->begin(); it!=hits->end(); ++it ) {
        addStrip(strip, dE, *it, t1, t2);
        dE = 0;
        t1 = t2 = -1;
    }
}


void SiStripList::addStrip(const int strip, const double dE,
                           const Event::McPositionHit* hit,
                           const int t1, const int t2) {
#else
export template<class T> void SiStripList::addStrip(const int strip,
                                                    const double dE,
                                                    const T* hit,
                                                    const int t1,
                                                    const int t2) {
#endif
    // Purpose and Method: adds a strip with a single McPositionHit to the list
    //                     of strips
    // Inputs: strip id, energy deposit, pointer to McPositionHit, ToT start and
    //         stop time
    // Outputs: none
    // Dependencies: none
    // Restrictions and Caveats: addStrip should be a template for both a hit
    //                           and a list of hits

    if ( strip == Strip::undef_strip() )
        return;

    // Real hits should have McPositionHits associated.  If hit is empty,
    // addStrip labels the strip as noise.
    const bool noise = hit ? false : true;

    // Strips from the Bari code (McToHitBariTool) contain electronic noise
    // (and ToT start and stop times) already.
    // Also, we assume that the parametrization for noise hits already
    // contains electronic noise.
    const bool elecNoise = ( t1>=0 && t2>=0 ) || noise ? true : false;
    // If there are none (e.g. the strip is from McToHitSimpleTool),
    // electronic noise has to be added later.

    // search list for strip
    for ( iterator s=begin(); s!=end(); ++s ) {
        const int index = s->index();
        if ( index < strip )
            continue;
        if ( strip == index ) {
            s->addEnergy(dE);
            s->addHit(hit);
            s->addTime(t1, t2);
            return;
        }
        // else ... add before the next, to keep in order of the index
        insert(s, Strip(strip, dE, noise, hit, elecNoise, t1, t2));
        return;
    }
        
    // If the strip isn't in the list yet, and no others have higher ids, 
    // add it to the end of the list.
    push_back(Strip(strip, dE, noise, hit, elecNoise, t1, t2));
}


void SiStripList::score(const HepPoint3D& o, const HepPoint3D& p,
                        const Event::McPositionHit* hit) {
    // Purpose and Method: distribute energy among the various strips as the
    //                     particle passes through the detector.
    // Inputs: entry and exit point (in local coordinates), and a pointer to a
    //         McPositionHit.
    // Outputs: none
    // Dependencies: none

    double eLoss = hit->depositedEnergy();
    if( eLoss == 0 )
        return;
    
    HepVector3D dir = p - o;
    float dTot = dir.mag();
    
    float xDir = dir.x();
    
    float in = o.x();     // entry point -- x
    float ex = p.x();     // exit point -- x
    int ins = stripId(in), exs = stripId(ex); // enter/exit strips (if valid)
    float len = dTot;  // path length through a strip
    if ( fabs(xDir) > si_strip_pitch() )
        len *= si_strip_pitch() / fabs(xDir);
    // max. length in x in a strip
    float dx_max = fabs(xDir)<si_strip_pitch() ? fabs(xDir) : si_strip_pitch();
    
    // There are four likely cases to deal with here: 
    // 1) particle entered & exited entirely in a gap 
    // 2) particle entered through a gap, then passed through & exited strips
    // 3) particle entered a strip, then passed into a gap and exited there 
    // 4) particle entered and exited strips
    // Note that this algorithm handles the the case where a particle completely
    // crosses a gap between strips (though this is highly unlikely). This 
    // algorithm fails if the particle enters a gap passes through an entire 
    // die of silicon strips (or multiple dies) then exits through another
    // gap. This is highly unlikely.
    
    if ( ins == Strip::undef_strip() ) {        // entered in a gap
        
        if ( exs != Strip::undef_strip() ) {        // exited through a strip
            float sx = calculateBin(exs);
            // fraction of strip crossed
            float dx = si_strip_pitch() / 2.0 + (ex-sx) * copysign(1.0, xDir);
            float frac = dx / dx_max;
            addStrip(exs, eLoss*frac*len/dTot, hit);
            
            short sinc = (xDir>0) ? -1 : 1; // move backwards (exit strip)
            for ( int sid=exs+sinc;
                  (sid>=0)
                      && (sid<n_si_strips())
                      &&
                      (
                       xDir>0 ? (in<calculateBin(sid)) : (in>calculateBin(sid))
                       );
                  sid+=sinc )
                addStrip(sid, eLoss*len/dTot, hit);
            // scans for until it crosses the entered gap
        }
        else
            return;  // entered & exited through a gap (assume no strips hit)
    }
    else {
        if ( exs == Strip::undef_strip() ) {        // exited through a gap
            float sx = calculateBin(ins);
            // fraction of strip crossed
            float dx = si_strip_pitch() / 2.0 - (in-sx) * copysign(1.0, xDir);
            float frac = dx / dx_max;   
            addStrip(ins, eLoss*frac*len/dTot, hit);
            
            short sinc = (xDir>0) ? 1 : -1;   // move backwards (exit strip)
            for ( int sid=ins+sinc;
                  (sid>=0)
                      && (sid<n_si_strips())
                      && 
                      (
                       xDir>0 ? (ex>calculateBin(sid)) : (ex<calculateBin(sid))
                       );
                  sid+=sinc )
                addStrip(sid, eLoss*len/dTot, hit);
            // scans for until it crosses the exited gap
        }
        else {                // entered + exited through strips
            if ( ins == exs )
                addStrip(ins, eLoss, hit);
            else {
                float sx = calculateBin(ins);
                float dx = si_strip_pitch() / 2. - (in-sx) * copysign(1., xDir);
                float frac = dx / dx_max;
                addStrip(ins, eLoss*frac*len/dTot, hit); // entry strip
                sx = calculateBin(exs);
                dx = si_strip_pitch() / 2.0 + (ex-sx) * copysign(1.0, xDir);
                frac = dx / dx_max;
                addStrip(exs, eLoss*frac*len/dTot, hit); // exit strip
                
                short sinc = (ins<exs) ? 1 : -1;
                for ( int sid=ins+sinc; sid!=exs; sid+=sinc )   
                    addStrip(sid, eLoss*len/dTot, hit);
                // add energy to all strips between entry and exit
            }   // else (ins != ens)
        }   // else (exs != undef_strip())
    }   // else (ins != undef_strip())
}


int SiStripList::addNoise(const double sigma, const double occupancy,
                          const double threshold){
    // Purpose and Method: this function call other functions to add noise
    // Inputs: noise rms in MeV, strip occupancy fraction, and energy threshold
    // Outputs: number of strips added
    // Dependencies: none

    addElectronicNoise(sigma);
    const int addedStrips   = addNoiseStrips(occupancy, threshold);
    const int removedStrips = removeStripsBelowThreshold(threshold);

    return addedStrips - removedStrips;
}


void SiStripList::addElectronicNoise(const double sigma) {
    // Purpose and Method: checks for the elec noise flag, and adds electronic
    //                     noise to already triggered strips
    // Inputs: noise rms in MeV
    // Outputs: none
    // Dependencies: none

    iterator iter = begin();
    while ( iter != end() ) {
        // check for the electronic noise flag
        if ( !iter->electronicNoise() ) {
            iter->addEnergy(RandGauss::shoot(0.0, sigma));  // in MeV
            iter->electronicNoise(true);
        }
        ++iter;
    }
}


int SiStripList::addNoiseStrips(const double occupancy,
                                      const double threshold) {
    // Purpose and Method: adds noise hits to the strip list
    // Inputs: strip occupancy fraction, and energy threshold
    // Outputs: number of strips added
    // Dependencies: none

    int newStrips = 0;  // counter for added strips

    if(occupancy>0.0) {
        static const int N = s_stripPerWafer * s_n_si_dies;  // number of all strips
        // (random) number of strips to add
        const int n = static_cast<int>(RandBinomial::shoot(N, occupancy));

        for ( int i=0; i!=n; ++i ) {
            //int strip = stripId(RandFlat::shoot()*panel_width()
            //    - panel_width()/2.0);
            int strip = static_cast<int>(RandFlat::shoot()*N);
            const_iterator iter = begin();
            for ( iter=begin(); iter!=end() && iter->index()!=strip; iter++ );
            // discard if the strip id is already in the list
            if ( iter == end() ) {
                //TODO: use service
                //            addStrip<Event::McPositionHit>(strip, threshold*(1.0-log(RandFlat::shoot())));
                Event::McPositionHit* dummy;
                dummy = 0;
                addStrip(strip, threshold*(1.0-log(RandFlat::shoot())), dummy);
                ++newStrips;
            }
        }
    }

    return newStrips;
}


int SiStripList::removeStripsBelowThreshold(const double threshold) {
    // Purpose and Method: Removes strips which an energy deposit below
    //                     "threshold".  Strips can have a low energy as low as
    //                     if:
    //                     1) the strip energy falls below threshold when the
    //                        electronic noise is added
    //                     2) the strip is generated with an energy below
    //                        threshold.  As of Aug 19, 2003, this can happen.
    //                        Why (MWK)?
    // Inputs: energy threshold
    // Outputs: number of strips removed
    // Dependencies: none

    int removedStrips = 0;  // counter for removed strips

    iterator iter = begin();
    while ( iter != end() ) {
        if ( iter->energy() < threshold ) {
            /*
            std::cout << "removing strip with energy " << iter->energy()
                      << " below threshold " << threshold << std::endl;
            */
            iter = erase(iter);
            ++removedStrips;
        }
        else
            ++iter;
    }

    return removedStrips;
}


// private member functions

int SiStripList::getToT(const int sep, const int controller) const {
    // Purpose and Method: Calculates the ToT of a layer for given controller.
    //                     There are two methods, based on the information
    //                     stored with the strips: McToHitBariTool stores the
    //                     ToT start and stop time (and the energy, but this one
    //                     is inaccurate and is used only for GuiSvc),
    //                     McToHitSimpleTool stores the energy.  Usually, a
    //                     strip list should contain only "real" hits filled
    //                     with one method, but noise hits always don't contain
    //                     the times but an energy.
    //                     The code determines, if set, the minimum start and
    //                     stop times and calculates the ToT.  Otherwise, it
    //                     uses an empirical parametrization to estimate the
    //                     ToT from the maximum energy deposited in a single
    //                     strip.  For the later, implicitely noise hits are
    //                     included, but not for the "times" method.
    // Inputs: strip id of the separating strip (exclusive)
    // Outputs: a ToT
    // Dependencies: none
    // Caveats: Before, there were two separate routines for ToT0 and ToT1.
    //          Both were almost identical except for the "for" loop.  ToT0 used
    //          an iterator, ToT1 an reverse_iterator (to speed up searching),
    //          To ease maintainance, I wanted to merge both routines, but
    //          didn't find a better way than the one I have implemented now.

    int begin;
    int end;
    int inc;
    if ( controller == 0 ) {  // loop parameters for controller 0
        begin = 0;
        end = size();
        inc = 1;
    }
    else { // assuming controller 1
        begin = size() - 1;
        end = -1;
        inc = -1;
    }

    int t1 = INT_MAX;
    int t2 = INT_MIN;
    float emax = FLT_MIN;

    for ( int i=begin; i!=end; i+=inc ) { // loop over strips
        const SiStripList::Strip* it = &m_strips[i];
        // don't use strips which are "bad" or "noise"
        if ( it->badStrip() /* || it->noise()*/ )
            continue;
        if ( ( controller == 0 && it->index() >= sep ) ||
             ( controller == 1 && it->index() < sep ) )
            break; // break if the range of the other controller is reached
        const int time1 = it->time1();
        const int time2 = it->time2();
        if( time1 != -1 && time2 != -1 ) { // strip with times ("Bari")
            if ( time1 < t1 )
                t1 = time1;
            if ( time2 > t2 )
                t2 = time2;
        }
        else { // strip without times ("Simple")
            const float e = it->energy();
            if ( e > emax )
                emax = e;
        }
    }

    static const int totMax = GeneralHitToDigiTool::totMax();

    if ( t1 != INT_MAX && t2 != INT_MIN ) // "Bari"
        return std::min( totMax, ( t2 - t1 ) / 20 );

    static const double totAt1Mip = GeneralHitToDigiTool::totAt1Mip();
    static const double mevPerMip = GeneralHitToDigiTool::mevPerMip();
    static const double threshold = GeneralHitToDigiTool::totThreshold();
    static const double f         = totAt1Mip / ( 1.0 - threshold / mevPerMip );
   
    if ( emax/mevPerMip > threshold ) { // "Simple" or noise
        int iToT = static_cast<int> ( ( emax - threshold ) / mevPerMip * f );
        return  std::min( iToT, totMax );
    }

    // ToT is below threshold, but strip fired: different thresholds for each!
    // (or some other anomaly)
    return 0;
}
