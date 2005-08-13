/**
* @class SiStripList
*
* @brief Provides means of filling and retrieving a list of strips.
*
* SiStripList is used in SiPlaneMap, where it is mapped with an unique volume
* identifier of a silicon layer.  It contains a list of silicon strip which
* have fired.  Functions are provided to fill this list, and retrieve from it,
* as well as methods to manipulate the strip information.
*
* @author Toby Burnett, Leon Rochester (original authors)
* @author Michael Kuss
*
* $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiStripList.h,v 1.10 2004/12/14 03:07:33 lsrea Exp $
*/

#ifndef SISTRIPLIST_H
#define SISTRIPLIST_H

#include "General/GeneralHitToDigiTool.h"

#include "CLHEP/Geometry/Vector3D.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"
#include "Event/MonteCarlo/McPositionHit.h"

#include <algorithm>
#include <vector>

class SiStripList {

public:

    SiStripList(){}

    ~SiStripList() { clear(); }

    typedef std::vector<Event::McPositionHit*> hitList;

    // declaration of class Strip
    class Strip {

    public:

        /**
        * Constructor.  The strip list should contain only valid strips.  Thus,
        * the first parameter (index) should never get a default value
        * assigned.
        * @param index      strip id
        * @param energy     deposited energy
        * @param noise      flag to label if this strip was created by noise
        * @param hits       either a pointer to a McPositionHit or a hitList
        * @param elecNoise  flag to label if electronic noise was added
        * @param t1         ToT start time
        * @param t2         ToT stop time
        */
        template<class T> Strip(const int index, const double energy=0,
            const bool noise=false,
            const T* hits=0, const bool elecNoise=false,
            const int t1=-1, const int t2=-1)
            : m_index(index), m_energy(energy), m_noise(noise),
            m_elecNoise(elecNoise), m_time1(t1), m_time2(t2) {
                m_badStrip = false;
                addHit(hits);
            }

            void energy(const double e)        { m_energy = e; }
            void electronicNoise(const bool b) { m_elecNoise = b; }
            void setBadStrip()                 { m_badStrip = true; }
            double energy()          const { return m_energy; }
            int index()              const { return m_index; }
            bool noise()             const { return m_noise; }
            bool badStrip()          const { return m_badStrip; }
            bool electronicNoise()   const { return m_elecNoise; }
            int time1()              const { return m_time1; }
            int time2()              const { return m_time2; }
            const hitList& getHits() const { return m_hits; }

            /// add energy
            void addEnergy(const double e) { m_energy += e; }

            /**
            * modifies the ToT start and stop times, but only if both t1 and t2 are
            * != -1 (undefined).
            * @param t1 ToT start time
            * @param t2 ToT stop time
            */
            void addTime(const int t1, const int t2) {
                if( ( t1 != -1 ) && ( t2 != -1 ) ) {
                    if ( t1 < m_time1 || m_time1 < 0 )
                        m_time1 = t1;
                    if ( t2 > m_time2 )
                        m_time2 = t2;
                }
            }

            /// adding a hit to the list of hits
            void addHit(const Event::McPositionHit* hit) {
                hitList::iterator it=std::find(m_hits.begin(),m_hits.end(),hit);
                if ( it == m_hits.end() )
                    m_hits.push_back(const_cast<Event::McPositionHit*>(hit));
            }

            /// adding a list of hits to the list of hits
            void addHit(const hitList* hits) {
                for ( hitList::const_iterator it=hits->begin(); it!=hits->end(); ++it )
                    addHit(*it);
            }

            hitList::const_iterator begin() const { return m_hits.begin(); } 
            hitList::const_iterator end()   const { return m_hits.end(); }
            int size() const { return m_hits.size(); }

            // static functions

            /// unphysical strip id for an undefined (non-existent) strip
            static const int undef_strip() { return 65535; } 

    private:

        /// strip id, or undef_strip() if not set
        int     m_index;
        /// charge deposited
        float   m_energy;
        /// true if strip was generated by noise
        bool    m_noise;
        /// list of mc hits contributing to this strip
        hitList m_hits;
        /// true if energy contains contribution from electronic noise
        bool    m_elecNoise;
        /// true if strip is marked bad
        bool    m_badStrip;
        /// start ToT
        int     m_time1;
        /// end ToT
        int     m_time2;
    };
    // end definition of class Strip

    /**
    * Distribute energy among the various strips as the particle passes through
    * the detector.
    * @param 1   entrance point in local coordinates
    * @param 2   exit point in local coordinates
    * @param 3   pointer to a McPositionHit
    */
    void score(const HepPoint3D&,const HepPoint3D&,const Event::McPositionHit*, bool test);

    //#define TEMPLATE
#ifdef TEMPLATE
    /**
    * Adds a strip to the list of strips.
    * @param 1  strip id
    * @param 2  energy deposit in MeV
    * @param 3  T* is either a pointer to a McPositionHit or a hitList
    * @param 4  ToT start time
    * @param 5  ToT stop time
    */
    template<class T> void addStrip(const int, const double, const T*,
        const int =-1, const int =-1);
#else
    void addStrip(const int, const double, const hitList*, const int =-1,
        const int =-1);

    /**
    * Adds a strip to the list of strips.
    * @param 1   strip id
    * @param 2   energy deposit in MeV
    * @param 3   pointer to a McPositionHit
    * @param 4   ToT start time
    * @param 5   ToT stop time
    */
    void addStrip(const int, const double, const Event::McPositionHit*,
        const int =-1, const int =-1);
#endif

    void clear() { m_strips.clear(); }

    /**
    * ToT functions.  For all functions:
    * @param sep         strip id of separation (sep belongs to controller 1)
    * @param controller  controller
    * @return            ToT in units of 200ns
    */
public:
    ///
    static const int sepSentinel=100000;
    void getToT(int* ToT, const int tower, const int layer, const int view,
        const ITkrToTSvc* pToTSvc, const int sep=sepSentinel) const;

    /**
    * noise member functions.  The parameters denote:
    *  @param s  noise rms in MeV
    *  @param o  fraction of time a cell is occupied
    *  @param t  minimium energy deposit (MeV) that results in a latch
    *  @return   number of strips added/removed
    */
    /// uses the following functions to manipulate the strip list
    int  addNoise(const double s, const double o, const double t);
    /// add electronic noise to already triggered strips
    void addElectronicNoise(const double s);
    /// add noisy strips
    int  addNoiseStrips(const double o, const double t);
    /// remove strips with energy deposit below threshold
    int  removeStripsBelowThreshold(const double t);

    // typedefs to shorten typing

    typedef std::vector<Strip> StripList;
    typedef StripList::const_iterator const_iterator;
    typedef StripList::iterator iterator;   
    typedef StripList::const_reverse_iterator const_reverse_iterator;
    typedef StripList::reverse_iterator reverse_iterator;

    // helpers to manipulate the StripList

    int size()   const { return m_strips.size(); }
    /// true if the StripList is empty
    bool empty() const { return size() == 0; }
    iterator               begin()        { return m_strips.begin(); }
    iterator               end()          { return m_strips.end(); }
    const_iterator         begin()  const { return m_strips.begin(); } 
    const_iterator         end()    const { return m_strips.end(); }
    reverse_iterator       rbegin()       { return m_strips.rbegin(); }
    reverse_iterator       rend()         { return m_strips.rend(); }
    const_reverse_iterator rbegin() const { return m_strips.rbegin(); }
    const_reverse_iterator rend()   const { return m_strips.rend(); }
    iterator insert(iterator it, const Strip& x){return m_strips.insert(it, x);}
    iterator erase(iterator it) { return m_strips.erase(it); }
    void push_back(const Strip& x) { m_strips.push_back(x); }

    // static functions

    static const int    n_si_dies()      { return s_n_si_dies; }
    static const int    strips_per_die() { return s_stripPerWafer; }
    static const double die_width()      { return s_die_width; }
    static const double guard_ring()     { return s_guard_ring; }
    static const double ssd_gap()        { return s_ssd_gap; }
    static const double ladder_gap()     { return s_ladder_gap; }

    /**
    * access to the detector service, and initialization of the static
    * variables.
    */
    static StatusCode initialize(IGlastDetSvc*);

    /// number of Si strips in a single layer
    static const int n_si_strips() { return n_si_dies() * strips_per_die(); }

    /// width of a single silicon strip
    static const double si_strip_pitch() {
        return ( die_width() - 2.0 * guard_ring() ) / strips_per_die(); 
    }

    /// width of all connected silicon in a single layer (entire panel)
    static const double panel_width() {
        return n_si_dies() * die_width() + ( n_si_dies() - 1 ) * ladder_gap();
    }

    /// compute local coordinate from strip id
    static const double calculateBin(int x) { return s_detSvc->stripLocalX(x); }

    /// calculate the strip ID from the plane coordinate
    static const int stripId(double x) { return s_detSvc->stripId(x); }

private:
    /// method to confine hit to active area
    bool isActiveHit(HepVector3D& inVec, HepVector3D& outVec, double& eLoss, bool& trimmed);
    /// pointer to the detector service
    static IGlastDetSvc* s_detSvc;
    /// vector of strips
    StripList m_strips;        
    /// number of silicon dies across a single layer
    static int    s_n_si_dies;       
    /// number of silicon strips across a single die
    static int    s_stripPerWafer;
    /// width of a single die
    static double s_die_width;
    /// width of dead area, aka guard ring
    static double s_guard_ring;
    /// gap between dies
    static double s_ssd_gap;
    /// gap between ladders
    static double s_ladder_gap;

};

#endif
