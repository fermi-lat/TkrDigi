// $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiStripList.h,v 1.2 2002/06/20 22:23:38 burnett Exp $

#ifndef SiStripList_H
#define SiStripList_H

#include <vector>
#include "CLHEP/Geometry/Vector3D.h"
#include "GlastSvc/GlastDetSvc/IGlastDetSvc.h"

class SiStripList {
public:
    SiStripList(){}
    
    ~SiStripList();
    
    
    void score(const HepPoint3D& o, const HepPoint3D& p, float eLoss);
    
    void clear();
    
    /// add noise and remove hits below threshold
    /**
    *  @param noise_sigma noise rms in MeV
    *  @param occupancy  fraction of time a cell is occupied
    *  @param theshold   minimium energy deposit (MeV) that results in a latch
    */
    
    void addNoise(double noise_sigma, double occupancy, double threshold);
    
    /// Strip data declaration
    class Strip 
    {
    public:
        
        // constructor
        Strip (int index = -1, double energy = 0, bool noise=false)
            :m_index(index), m_energy(energy), m_noise(noise)
        {}
        
        // access, 
        void addEnergy (float e)    {  m_energy += e;   }   // add energy
        void energy (float e)       {  m_energy = e;    }   // set energy
        float energy () const       {  return m_energy; }   // get energy
        unsigned int index() const  {  return m_index;  }   // get index
        bool noise() const          {  return m_noise;  }   // get noise status
        
        // static parameters
        // undefined strip (non-existent)
        static unsigned int undef_strip () { return 65535; } 
        
    private:
        int     m_index;  // strip number, -1 if invalid
        float   m_energy; // charge deposited
        bool    m_noise;  // true if generated by noise
        
    };
    
    
    /// for access to the service
    static void initialize(IGlastDetSvc* detsvc);
    
    typedef std::vector < Strip > StripList;            // list of hit strips
    typedef StripList::const_iterator const_iterator;   // iterator
    typedef StripList::iterator iterator;   
    
    // wrap std::vector members
    // access to the list of strips
    const_iterator  begin () const { return m_strips.begin(); } 
    const_iterator  end () const { return m_strips.end(); }
    int size () const { return m_strips.size(); }
    bool empty() const { return size()==0; }
    
    
    static IGlastDetSvc* s_detsvc;
    
    // compute local coordinate from strip id
    static double calculateBin (unsigned int ix);   
    
    static unsigned int stripId(double x);
    void addStrip(unsigned int ix, float dE);
    
    
    // access
    iterator    begin () { return m_strips.begin(); }
    iterator    end () { return m_strips.end(); }
    iterator    insert (iterator it, const Strip& x) 
    { return m_strips.insert(it, x); }
    iterator    erase (iterator it) { return m_strips.erase(it); }
    void        push_back (const Strip& x) { m_strips.push_back(x); }
    
    StripList   m_strips;
    
    /// number of silicon dies across a single layer
    static unsigned int n_si_dies ();
    
    /// number of silicon strips across a single die
    static unsigned int strips_per_die ();
    
    /// width of a single si die
    static double die_width ();
    
    /// number of silicon strips in a single layer
    static unsigned int n_si_strips ();
    
    /// number of front-end chips on a single si die
    static unsigned int n_fe_chips ();
    
    /// silicon strip pitch (computed from n_fe_chips & active are of the die)
    static double si_strip_pitch ();
    
    
    /// gap between the edge of the silicon die and the tray closeout
    static double electronics_gap ();
    
    /// gap between Silicon dies
    static double ssd_gap ();
    
    /// gap between ladders
    static double ladder_gap ();
    
    /// width of dead area, aka guarg_ring
    static double guard_ring ();
    static double panel_width(); 
    
private:
        
    static double s_dice_width;
    static double s_ssd_gap;
    static double s_ladder_gap;
    static double s_guard_ring;
    static double s_panel_width;
    static unsigned int s_stripPerWafer;
    static unsigned int s_n_si_dies;
        
};




#endif
