/**
* @file SiStripList.cxx
*
* @brief Provides means of filling and retrieving a list of strips.
*
* @author Toby Burnett, Leon Rochester (original authors)
* @author Michael Kuss
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiStripList.cxx,v 1.25 2009/09/09 22:18:37 lsrea Exp $
*/

#include "SiStripList.h"
#include "General/GeneralNoiseTool.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandBinomial.h"
#include "CLHEP/Random/RandLandau.h"

#include <algorithm>
#include <float.h>
#include <limits.h>  // <limits> exist in gcc3.2, but not gcc2.95.3


// utility function declarations
#ifdef _MSC_VER
inline double copysign(double a, double b){return _copysign(a,b);}
#endif

namespace {
    bool debug = false;
    bool printEdep = true;
    bool doLandau = true;
}

// static variable implementations--now initialized with detsvc.

IGlastDetSvc* SiStripList::s_detSvc = 0;
int     SiStripList::s_n_si_dies     = 0;   // 4;
int     SiStripList::s_stripPerWafer = 0;   // 384;
double  SiStripList::s_die_width     = 0.0; // 89.500;
double  SiStripList::s_guard_ring    = 0.0; // 0.974;
double  SiStripList::s_ssd_gap       = 0.0; // 0.025;            
double  SiStripList::s_ladder_gap    = 0.0; // 0.200;


StatusCode SiStripList::initialize(IGlastDetSvc* ds/*, ITkrToTSvc* ts*/) 
{
    // Purpose and Method: initializes the static members from the detector svc
    // Inputs: pointer to the detector service
    // Outputs: a status code
    // Dependencies: the detector service has to be initialized first

    s_detSvc = ds;
    //s_totSvc = ts;
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
                           int t1, int t2) 
{
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
                           const int t1, const int t2) 
{
#else
export template<class T> void SiStripList::addStrip(const int strip,
                                                    const double dE,
                                                    const T* hit,
                                                    const int t1,
                                                    const int t2) 
{
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
            if (hit!=0) s->addHit(hit);
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
                        const Event::McPositionHit* hit, 
                        bool fluctuate = false, bool test = false) 
{
    // Purpose and Method: distribute energy among the various strips as the
    //                     particle passes through the detector.
    // Inputs: entry and exit point (in local coordinates), and a pointer to a
    //         McPositionHit. If "test" is true, a constant 0.155 MeV is deposited.
    // Outputs: none
    // Dependencies: none


    double eLoss = ( test ? 0.155 : hit->depositedEnergy());
    if( eLoss == 0 ) return;

    HepVector3D inVec  = o;
    HepVector3D outVec = p;

    //store the stuff to add energies at the end;
    // brute force, should be a little class
    std::vector<float> eVec(0);  // eDep for strip
    std::vector<int>   idVec(0); // iD of strip
    std::vector<const Event::McPositionHit*> hitVec(0); // Mc pos hit

    // the method can modify all the arguments
    bool trimmed;
    if(!isActiveHit(inVec, outVec, eLoss, trimmed)) return;

    HepVector3D dir = outVec - inVec;
    float dTot = dir.mag();

    float xDir = dir.x();

    float in = inVec.x();     // entry point -- x
    float ex = outVec.x();     // exit point -- x
    int ins = stripId(in), exs = stripId(ex); // enter/exit strips (if valid)
    float len = dTot;  // path length through a strip
    if ( fabs(xDir) > si_strip_pitch() )
        len *= si_strip_pitch() / fabs(xDir);
    // max. length in x in a strip
    float dx_max = fabs(xDir)<si_strip_pitch() ? fabs(xDir) : si_strip_pitch();
    float dE = (test ? 0.155 : eLoss*len/dTot);

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
            float frac = (test ? 1.0 : dx / dx_max);
            //addStrip(exs, dE*frac, hit);
            idVec.push_back(exs);
            eVec.push_back(dE*frac);
            hitVec.push_back(hit);
            if(eVec.size()!=idVec.size()){
                std::cout << eVec.size() << " " << idVec.size() << std::endl;
            }


            short sinc = (xDir>0) ? -1 : 1; // move backwards (exit strip)
            for ( int sid=exs+sinc;
                (sid>=0)
                && (sid<n_si_strips())
                &&
                (
                xDir>0 ? (in<calculateBin(sid)) : (in>calculateBin(sid))
                );
            sid+=sinc ) {
                //addStrip(sid, dE, hit);
                idVec.push_back(sid);
                eVec.push_back(dE);
                hitVec.push_back(hit);
                if(eVec.size()!=idVec.size()){
                    std::cout << eVec.size() << " " << idVec.size() << std::endl;
                }
            }

 
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
            float frac = ( test ? 1.0 : dx / dx_max);
            //addStrip(ins, dE*frac, hit);
            idVec.push_back(ins);
            eVec.push_back(dE*frac);
            hitVec.push_back(hit);
            if(eVec.size()!=idVec.size()){
                std::cout << eVec.size() << " " << idVec.size() << std::endl;
            }


            short sinc = (xDir>0) ? 1 : -1;   // move backwards (exit strip)
            for ( int sid=ins+sinc;
                (sid>=0)
                && (sid<n_si_strips())
                && 
                (
                xDir>0 ? (ex>calculateBin(sid)) : (ex<calculateBin(sid))
                );
            sid+=sinc ) {
                //addStrip(sid, dE, hit);
                idVec.push_back(sid);
                eVec.push_back(dE);
                hitVec.push_back(hit);
                if(eVec.size()!=idVec.size()){
                    std::cout << eVec.size() << " " << idVec.size() << std::endl;
                }
            }


            // scans for until it crosses the exited gap
        }
        else {                // entered + exited through strips
            if ( ins == exs ) {
                //addStrip(ins, eLoss, hit);
                idVec.push_back(ins);
                eVec.push_back(eLoss);
                hitVec.push_back(hit);
                if(eVec.size()!=idVec.size()){
                    std::cout << eVec.size() << " " << idVec.size() << std::endl;
                }
            } else {
                float sx = calculateBin(ins);
                float dx = si_strip_pitch() / 2. - (in-sx) * copysign(1., xDir);
                float frac = ( test ? 1.0 : dx / dx_max);
                //addStrip(ins, dE*frac, hit); // entry strip
                idVec.push_back(ins);
                eVec.push_back(dE*frac);
                hitVec.push_back(hit);
                if(eVec.size()!=idVec.size()){
                    std::cout << eVec.size() << " " << idVec.size() << std::endl;
                }

                sx = calculateBin(exs);
                dx = si_strip_pitch() / 2.0 + (ex-sx) * copysign(1.0, xDir);
                frac = ( test ? 1.0 : dx / dx_max);
                //addStrip(exs, dE*frac, hit); // exit strip
                idVec.push_back(exs);
                eVec.push_back(dE*frac);
                hitVec.push_back(hit);
                if(eVec.size()!=idVec.size()){
                    std::cout << eVec.size() << " " << idVec.size() << std::endl;
                }

                short sinc = (ins<exs) ? 1 : -1;
                for ( int sid=ins+sinc; sid!=exs; sid+=sinc ) {  
                    //addStrip(sid, dE, hit);
                    idVec.push_back(sid);
                    eVec.push_back(dE);
                    hitVec.push_back(hit);
                    if(eVec.size()!=idVec.size()){
                        std::cout << eVec.size() << " " << idVec.size() << std::endl;
                    }
                }

                // add energy to all strips between entry and exit
            }   // else (ins != ens)
        }   // else (exs != undef_strip())
    }   // else (ins != undef_strip())


    // now we can process all the strips
    // for normal operation, just add the strips with the standard energy
    // if this works, we'll add code here to do the fluctuations
    unsigned int i;
    unsigned int size = eVec.size();
    if (fluctuate && size > 0 ) {
        //unsigned int k = 0;     
        // get the landau numbers
        //const int nLandau = 100;
        //double landauArray[nLandau];

        //CLHEP::RandLandau::shootArray(size, landauArray);
        float e0 = 0.0;
        float e1 = 0.0;
        for(i=0;i<size; ++i) {
            e0 += eVec[i];
            double rand = CLHEP::RandLandau::shoot();
            eVec[i] *= (1.0 + 0.095*rand);
            e1 += eVec[i];
        }
        float norm = e0/e1;
        for(i=0;i<size;++i) {
            eVec[i] *= norm;
        }

    }
    for(i=0;i<size; ++i) {
        addStrip(idVec[i], eVec[i], hitVec[i]);
    }
}

int SiStripList::addNoise(const double sigma, const double occupancy,
                          const double threshold, const double trigThreshold)
{
    // Purpose and Method: this function call other functions to add noise
    // Inputs: noise rms in MeV, strip occupancy fraction, and energy threshold
    // Outputs: number of strips added
    // Dependencies: none

    addElectronicNoise(sigma);
    const int addedStrips   = addNoiseStrips(occupancy, threshold, trigThreshold);
    const int removedStrips = removeStripsBelowThreshold(threshold, trigThreshold);

    return addedStrips - removedStrips;
}


void SiStripList::addElectronicNoise(const double sigma) 
{
    // Purpose and Method: checks for the elec noise flag, and adds electronic
    //                     noise to already triggered strips
    // Inputs: noise rms in MeV
    // Outputs: none
    // Dependencies: none

    iterator iter = begin();
    while ( iter != end() ) {
        // check for the electronic noise flag
        //double eDep = iter->energy();
        
        //if(printEdep&&!iter->electronicNoise()) std::cout << eDep << std::endl;

        if ( !(iter->electronicNoise())) {
            iter->addEnergy(CLHEP::RandGauss::shoot(0.0, sigma));  // in MeV
            iter->electronicNoise(true);
        }
        ++iter;
        
    }
}

int SiStripList::addNoiseStrips(const double occupancy,
                                const double threshold,
                                const double /* trigThreshold */) 
{
    // Purpose and Method: adds noise hits to the strip list
    // Inputs: strip occupancy fraction, and energy threshold
    // Outputs: number of strips added
    // Dependencies: none

    int newStrips = 0;  // counter for added strips

    if(occupancy>0.0) {
        static const int N = s_stripPerWafer * s_n_si_dies;  // number of all strips
        // (random) number of strips to add
        const int n = static_cast<int>(CLHEP::RandBinomial::shoot(N, occupancy));

        for ( int i=0; i!=n; ++i ) {
            //int strip = stripId(RandFlat::shoot()*panel_width()
            //    - panel_width()/2.0);
            int strip = static_cast<int>(CLHEP::RandFlat::shoot()*N);
            const_iterator iter = begin();
            for ( iter=begin(); iter!=end() && iter->index()!=strip; iter++ );
            // discard if the strip id is already in the list
            if ( iter == end() ) {
                //TODO: use service
                //            addStrip<Event::McPositionHit>(strip, threshold*(1.0-log(RandFlat::shoot())));
                Event::McPositionHit* dummy;
                dummy = 0;

                double energy = threshold*(1.0-log(CLHEP::RandFlat::shoot()));
                addStrip(strip, energy, dummy);
                //if(printEdep) std::cout << energy << std::endl;
                ++newStrips;
            }
        }
    }

    return newStrips;
}


int SiStripList::removeStripsBelowThreshold(const double threshold, 
                                            const double trigThreshold) 
{
    // Purpose and Method: Flags strips which an energy deposit below
    //                     "threshold".  Strips can have a low energy if:
    //                     1) the strip energy falls below threshold when the
    //                        electronic noise is added
    //                     2) the strip is generated with an energy below
    //                        threshold.  As of Aug 19, 2003, this can happen.
    //                        Why (MWK)?
    // Inputs: energy threshold
    // Outputs: number of strips flagged as removed
    // Dependencies: none

    int removedStrips = 0;  // counter for removed strips

    iterator iter = begin();
    bool first = true;
    while ( iter != end() ) {
        double eDep = iter->energy();
        //if(printEdep&&!iter->electronicNoise()&&eDep>threshold) std::cout << eDep << std::endl;
        if ( eDep < trigThreshold) {
            iter->setStripStatus(BELOWTRIGTHRESH);
        }

        if ( eDep < threshold ) {

            /*
            std::cout << "removing strip with energy " << iter->energy()
            << " below threshold " << threshold << std::endl;
            */
            iter->setStripStatus(BELOWDATATHRESH);
            if(debug) {
                if (first) {
                    std::cout << "Strips flagged for removal ";
                    first = false;
                }
                std::cout << iter->index() << " " ;
            }
            ++removedStrips;
        }
        ++iter;
    }
    if (!first) std::cout << std::endl;

    return removedStrips;
}


// private member functions

void SiStripList::getToT(int* ToT, const int tower, const int layer, const int view,
                         const ITkrToTSvc* pToTSvc, const int sep) const 
{
    // Purpose and Method: Calculates the ToTs of a layer.
    //                     There are two methods, based on the information
    //                     stored with the strips: McToHitBariTool stores the
    //                     ToT start and stop time (and the energy, but this one
    //                     is inaccurate and is used only for GuiSvc),
    //                     McToHitSimpleTool stores the energy.  Usually, a
    //                     strip list should contain only "real" hits filled
    //                     with one method.
    //
    //                     If called with one argument, the method returns a pointer
    //                     to a single ToT, the max ToT for the layer
    //
    //                     The code determines, if set, the minimum start and
    //                     stop times and calculates the ToT.  Otherwise, it
    //                     uses an empirical parametrization to estimate the
    //                     ToT from the maximum energy deposited in a single
    //                     strip.  For the later, implicitely noise hits are
    //                     included, but not for the "times" method.
    // Inputs: pointer to the array of ToTs, strip id of the last C0 strip
    // Outputs: fills in the ToTs

    int t1[2]     = {INT_MAX, INT_MAX};
    int t2[2]     = {INT_MIN, INT_MIN};
    int simpleToT[2] = {INT_MIN, INT_MIN};

    int totMax    = pToTSvc->getMaxToT();

    int size = m_strips.size();
    int controller = 0;
    int i;
    for (i=0; i<size; ++i ) { // loop over strips
        const SiStripList::Strip strip = m_strips[i];
        // don't use certain kinds of bad strips
        // RC and CC overflows do contribute to the ToT
        //   but failed layers and dead strips don't (depends on how they fail??)
        //   neither do hits below the trigger threshold (tho for now, trigger and 
        //   data thresholds are the same)

        int status = strip.stripStatus();
        if ((status&NOTRIG)!=0) continue;
        int index = strip.index();
        if (index>sep) controller = 1;
        int time1 = strip.time1();
        int time2 = strip.time2();
        if( time1 != -1 && time2 != -1 ) { // strip with times ("Bari")
            if ( time1 < t1[controller] )
                t1[controller] = time1;
            if ( time2 > t2[controller] )
                t2[controller] = time2;
        }
        else { // strip without times ("Simple")
            float e = strip.energy();
            if ( e>0 ) { // "Simple" or noise
                int iToT = pToTSvc->getRawToT(e, tower, layer, view, index);
                simpleToT[controller] = std::max(simpleToT[controller], iToT);
            }
        }
    }

    // These values reproduce the previous results 
    //double rawGain   = 2.50267833;
    //double rawThresh = -2.92;

    // loop over controllers
    for(i=0; i<2; ++i) {
        ToT[i] = 0;
        if ( t1[i] != INT_MAX && t2[i] != INT_MIN ) { // "Bari"
            ToT[i] =  std::min( totMax, ( t2[i] - t1[i] ) / 20 );
        }
        if(simpleToT[i]>0) { 
            ToT[i] =  std::min( simpleToT[i], totMax );
        }
        if (sep==sepSentinel) break;
    }
}

bool SiStripList::isActiveHit(HepVector3D& inVec, HepVector3D& outVec, 
                              double& eLoss, bool& trimmed) 
{
    trimmed = false;
    bool active  = true;
    HepVector3D dir = outVec - inVec;
    // check if the *other* projection is in the wafer
    // if partway in, we want to trim the original hit.
    double activeYin  = s_detSvc->insideActiveLocalY(inVec);
    double activeYout = s_detSvc->insideActiveLocalY(outVec);

    // here's the easy one
    if (activeYin<=0 && activeYout<=0) {
        trimmed = true;
        return false;
    }

    double fractionIn = 1.0;
    double delta;
    if (activeYin<=0) {
        //this is more work, only one is out
        // get the fraction in
        delta = activeYout - activeYin; // delta is guaranteed non-zero
        fractionIn = activeYout/delta;
        inVec = outVec - fractionIn*dir;
    } else if (activeYout<=0) {
        delta = activeYin - activeYout;
        fractionIn = activeYin/delta;
        outVec = inVec + fractionIn*dir;
    }
    if (fractionIn<1.0) trimmed = true;
    if (fractionIn<=0.0) active = false;
    dir = outVec - inVec;
    eLoss *= fractionIn;

    // now for the measured projection
    double activeXin  = s_detSvc->insideActiveLocalX(inVec);
    double activeXout = s_detSvc->insideActiveLocalX(outVec);

    // here's the easy one
    if (activeXin<=0 && activeXout<=0) {
        trimmed = true;
        return false;
    }

    fractionIn = 1.0;
    // little fudge to make sure the point ends up inside
    double eps  = 0.005*si_strip_pitch(); 
    if (activeXin<=0) {
        //this is more work, only one is out
        // get the fraction in
        delta = activeXout - activeXin; // delta is guaranteed nonzero!
        fractionIn  = std::max(0.0, (activeXout - eps)/delta);
        inVec = outVec - fractionIn*dir; 
    } else if (activeXout<=0) {
        delta  = activeXin - activeXout;
        fractionIn   = std::max(0.0, (activeXin - eps)/delta);
        outVec = inVec + fractionIn*dir;
    }
    if (fractionIn<1.0) trimmed = true;
    if (fractionIn<=0.0) active = false;
    eLoss *= fractionIn;

    return active;
}
