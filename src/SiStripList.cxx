// $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiStripList.cxx,v 1.4 2002/08/12 23:07:26 lsrea Exp $

#include "SiStripList.h"
#include <algorithm>
#include <float.h>
#include <iomanip>

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"



//  utility function declarations
#ifdef _MSC_VER
inline double copysign(double a, double b){return _copysign(a,b);}
#endif

IGlastDetSvc* SiStripList::s_detsvc=0;

void SiStripList::initialize(IGlastDetSvc * ds)
{
    s_detsvc=ds;
	StatusCode sc;
	double temp;

	sc = s_detsvc->getNumericConstByName("ssdGap", &s_ssd_gap);
	sc = s_detsvc->getNumericConstByName("ladderGap", &s_ladder_gap);
	sc = s_detsvc->getNumericConstByName("stripPerWafer", &temp);
	s_stripPerWafer = temp + 0.5;
	sc = s_detsvc->getNumericConstByName("nWaferAcross", &temp);
	s_n_si_dies = temp + 0.5;
	sc = s_detsvc->getNumericConstByName("SiWaferSide", &s_dice_width);
	sc = s_detsvc->getNumericConstByName("SiWaferActiveSide", &temp);
	s_guard_ring = 0.5*(s_dice_width - temp);

}

SiStripList::~SiStripList(){ clear(); }

// static variable implementations--now initialized with detsvc.
unsigned int SiStripList::s_n_si_dies      =  0; // 4; // # of Si dies across single SSD plane
unsigned int SiStripList::s_stripPerWafer  =  0; // 384;
double       SiStripList::s_dice_width     = 0.; // 89.500; // width of a single silicon die 
double       SiStripList::s_guard_ring     = 0.; // 0.974;
double       SiStripList::s_ssd_gap        = 0.; // 0.025;            
double       SiStripList::s_ladder_gap     = 0.; // 0.200;


/// number of silicon dies
double   SiStripList::die_width () {     return s_dice_width;}
double   SiStripList::guard_ring () {    return s_guard_ring;}
double   SiStripList::ssd_gap () {       return s_ssd_gap;}
double   SiStripList::ladder_gap () {    return s_ladder_gap;}
unsigned int SiStripList::strips_per_die () {return s_stripPerWafer;}
unsigned int SiStripList::n_si_dies() {return s_n_si_dies;}

/// si_strip_pitch - return the width of a single silicon strip
double SiStripList::si_strip_pitch () {
    return ((die_width() - 2. * guard_ring()) / strips_per_die()); 
}

/// panel_width - width of all connected silicon in a single layer
double SiStripList::panel_width () {
    return (n_si_dies() * die_width() + (n_si_dies() - 1) * ladder_gap());
}
/// n_si_strips - return the number of Si strips in a single layer
unsigned int SiStripList::n_si_strips ()  {
    return (n_si_dies() * strips_per_die());
}

/// stripId - calculate the strip ID from the plane coordinate
unsigned int SiStripList::stripId(double x){
    return s_detsvc->stripId(x);
    
}
/// calculateBin - calculates the center of a given strip
double   SiStripList::calculateBin (unsigned int ix) {
	return s_detsvc->stripLocalX(ix);
}

/// addStrip - adds a strip to the list of strips...
void SiStripList::addStrip(unsigned int ix, float dE)
{
    if (ix != Strip::undef_strip()) {
        // search list for
        iterator s = begin();
        for(; s != end(); ++s) {
            if (ix > (*s).index()) continue;
            if (ix == (*s).index()) (*s).addEnergy(dE);    
            else  // add before the next, to keep in order of the index
                insert(s, Strip(ix, dE));
            return;
        }
        
        ///  If the strip isn't in the list yet, and no others have higher ids, 
        /// add it to the end of the list.
        push_back(Strip(ix, dE));
    }
}


// score - distribute energies among the various strips as the particle
// passes through the detector
// p and o must be in local coordinates...
void SiStripList::score(const HepPoint3D& o, const HepPoint3D& p, float eLoss)
{
    if( eLoss == 0 )
        return;
    
    HepVector3D dir = (p - o);
    float dTot = dir.mag();
    
    float xDir = dir.x();
    
    float    in = o.x();     // entry point -- x
    float    ex = p.x();     // exit point -- x
    unsigned ins = stripId(in), exs = stripId(ex); // enter/exit strips (if valid)
    float len = dTot;  // path length through a strip
    if(fabs(xDir) > si_strip_pitch()) len *= si_strip_pitch()/fabs(xDir);
    // max. length in x in a strip
    float dx_max = 
        (fabs(xDir) < si_strip_pitch()) ? fabs(xDir) : si_strip_pitch();
    
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
            
            float   sx = calculateBin( exs );
            // fraction of strip crossed
            float dx = si_strip_pitch()/2. + (ex-sx)*copysign(1.,xDir);
            float frac = dx/dx_max;
            addStrip( exs, eLoss*frac*len/dTot );
            
            short   sinc = ( xDir > 0 ) ? -1 : 1;   // move backwards (exit strip)
            for (unsigned int sid = exs + sinc;
            (sid >= 0) && (sid < n_si_strips()) 
                && ((xDir > 0) ? 
                (in < calculateBin( sid )) : (in > calculateBin( sid )));
            sid += sinc )
                addStrip( sid, eLoss*len/dTot );
            // scans for until it crosses the entered gap
            
        } else return;   // entered & exited through a gap (assume no strips hit)
        
    } else {
        
        if ( exs == Strip::undef_strip() ) {        // exited through a gap
            
            float   sx = calculateBin( ins );
            // fraction of strip crossed
            float dx = si_strip_pitch()/2. - (in-sx)*copysign(1.,xDir);
            float frac = dx/dx_max;   
            addStrip( ins, eLoss*frac*len/dTot );
            
            short   sinc = ( xDir > 0 ) ? 1 : -1;   // move backwards (exit strip)
            for (unsigned int  sid = ins + sinc;
            (sid >= 0) && 
                (sid < n_si_strips()) && 
                ((xDir > 0) ? 
                (ex > calculateBin( sid )):(ex < calculateBin( sid )));
            sid += sinc )
                addStrip( sid, eLoss*len/dTot );
            // scans for until it crosses the exited gap
            
        } else {                // entered + exited through strips
            
            if ( ins == exs )   addStrip ( ins, eLoss );
            else {
                float sx = calculateBin ( ins );
                float dx = si_strip_pitch()/2. - (in-sx)*copysign(1.,xDir);
                float frac = dx/dx_max;
                addStrip( ins, eLoss*frac*len/dTot ); // entry strip
                sx = calculateBin ( exs );
                dx = si_strip_pitch()/2. + (ex-sx)*copysign(1.,xDir);
                frac = dx/dx_max;
                addStrip( exs, eLoss*frac*len/dTot ); // exit strip
                
                short sinc = ( ins < exs ) ? 1 : -1;
                for( unsigned sid = ins + sinc; sid != exs; sid += sinc )   
                    addStrip( sid, eLoss*len/dTot );
                // add energy to all strips between entry and exit
            }   // else (ins != ens)
            
        }   // else (exs != undef_strip())
        
    }   // else (ins != undef_strip())
}

void SiStripList::addNoise(double noise_sigma, double noise_occupancy, double threshold)
{
    // add noise to already triggered strips
    iterator iter = begin();
    while ( iter != end() ) {
        float enoise = RandGauss::shoot(0.,noise_sigma);  // in MeV
        (*iter).addEnergy(enoise);
        ++iter;
    }
    
    // add noise 'hits' to the strip listing
    double  r1 = RandFlat::shoot(); //TODO: use service

    static int   N = s_stripPerWafer*s_n_si_dies;
    static double q0 = 1. - pow(1. - noise_occupancy, N);

    // insert random triggered strips
    for (double n = 1. , q = q0; 
    r1 < q; n += 1. , q *=(N-n)*noise_occupancy/((n+1)*(1-noise_occupancy)))  {
        unsigned  strip = 
            stripId((RandFlat::shoot())*panel_width() - panel_width()/2.);
        for (iter = begin(); (iter != end())&&((*iter).index() != strip); iter++);
        
        if (iter == end())  {
            addStrip(strip, threshold*(1. - log(RandFlat::shoot())) ); //TODO: use service
        }
    }
    
    
    // remove strips below threshold
    
    iter = begin();
    while ( iter != end() ) {
        
        float e = (*iter).energy();
        
        if( e < threshold ) {
            iter = erase(iter);
        }else {
            ++iter;
        }
    }
}

void SiStripList::clear()
{
    m_strips.clear();
}