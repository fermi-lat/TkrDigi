//$Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiLayerList.cxx,v 1.3 2003/03/01 02:16:06 lsrea Exp $
#include "SiLayerList.h"


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IGeometry::VisitorRet 
  SiLayerList::pushShape(ShapeType /* s */, const UintVector& idvec, 
                           std::string name, std::string /* material */, 
                           const DoubleVector& /* params */, 
                           VolumeType /* type */)
{
    // concatenate the id for this node to current id.
    m_idcount.push_back(idvec.size());
    for( UintVector::const_iterator u=idvec.begin(); u!=idvec.end(); ++u){
        m_idValues.push_back(static_cast<unsigned int>(*u));
    }
    // is this what we want? add to the list if so and abort
    if(name.find("SiLayerBox")!=std::string::npos || 
        name.find("SiLadders")!=std::string::npos ) {
        this->push_back(getId());
        return AbortSubtree;
    }
    // abort if we know it does not contain what we are looking for
    if( name.substr(0,3)=="ACD") return AbortSubtree;

    // otherwise continure
    return More;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SiLayerList::popShape()
{
    unsigned int count = m_idcount.back(); m_idcount.pop_back();
    while(count--){
        m_idValues.pop_back();
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
idents::VolumeIdentifier SiLayerList::getId()const
{
    idents::VolumeIdentifier id(m_prefix);
    for(UintVector::const_iterator i=m_idValues.begin(); i!=m_idValues.end(); ++i) id.append(*i);
    return id;

}
