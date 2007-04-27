//##################################################################
//#          Id$:  TimeOr.h                                        #
//#                                                                #
//# Class to maninpulate dinamic arrays used in digitization       #
//#  processes performing Map_OR on each plane                     #
//#                                                                #
//# Input: Plane ID, and strip ID and the strip Voltage signal     #
//# Output: Plane ID and Map_OR for each fired plane,              #
//#         nr of strips for each plane fired and the strip IDs    #
//#                                                                #
//# public functions:                                              #
//# 1.Clean() to reset all variables -                             #
//#           to be done at the beginning of the event             #
//# 2.  Print();                                                   #
//# 3. Out();                                                      #
//# 4. Add(int,int,int,int,int); //pln,or,strip,t1,t2              #
//# 5. Len(){return Nelem;}                                        #
//# 6. Plane(unsigned int l) {return List[l*list_elem+0];};        #
//#                  return the Plane                              #
//# 7. Type(unsigned int l) {return List[l*list_elem+1];};         #
//#                        return the PlaneType                    #
//# 8. Tim1(unsigned int l) {return List[l*list_elem+2];};         #
//#                        return the Tot1 in 200 ns step          #
//# 9. Tim2(unsigned int l) {return List[l*list_elem+3];};         #
//#                        return the Tot2 in 200 ns step          #
//# 10. Pos(int,int);                                              #
//#  return the entry number given a coordinate or -1 if not found #
//# 11. ListPos(int l){return l*list_elem;};                       #
//#                    the base address for List                   #
//# 12. vsize(int l) is the number of strips in the l-th plane     #
//# 13. vget(int l) return the vector list of the fired strips     #
//#                                                                #
//#                                   N. Giglietto                 #
//#                                                                #
//#  23-Aug-02 change to Tower, Layer, View   LSR                  #
////////////////////////////////////////////////////////////////////
// Revised April 2007 M.Brigida             ////////////////////////
//                                               ///////////////////
//##################################################################

#ifndef TimeOr_h
#define TimeOr_h 1

#include <vector>
#include "Digi.h"

class TimeOr{

private:
    int list_elem;     
    void AddNew(int,int,int,int,double,double);     // Add a new entry in the object
    unsigned int Nelem;               // number of entries       
    std::vector<Digi> mylist; 
    
public:
    TimeOr();
    ~TimeOr();

    void TimeOr::Add(TimeOr* );
    void Clean();
    void Out();
    void Add(int,int,int,int,double,double);    //lindex,pln,or,strip,t1,t2
    void AddStrip(int,int); // add a strip in the layer checking if it is new 
    int Pos(int,int, int);
    // return entry number given a coordinate or -1 if not found
    unsigned int size(){return Nelem;}
    int Tower(int l) {return mylist[l].Tower();};  
    // return the Plane 
    int Layer(int l) {return mylist[l].Layer();}; 
    // return the PlaneType
    int View(int l) {return mylist[l].View();}; 
    // Tot Difference (Time2 - Time1)
    double Tim3(int l) {return (mylist[l].Tim2() - mylist[l].Tim1());}; 
    double Tim1(int l) {return (mylist[l].Tim1());}; 
    double Tim2(int l) {return (mylist[l].Tim2());}; 

};
#endif
