/**
 * @file SiLayerList.cxx
 *
 * @brief This class uses detModel to make a list of all Si Layers,
 * from the element names.
 *
 * @author ??? ... has written this class.
 * @author Michael Kuss ... has only made this header and some formatting.
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigiSandBox/src/SiLayerList.cxx,v 1.4 2004/02/24 13:57:32 kuss Exp $
 */

#include "SiLayerList.h"


/* Obviously, pushShape has some history, and several modifications were done.
   Effectively, only two input parameters are used.
*/
IGeometry::VisitorRet SiLayerList::pushShape(ShapeType /* s */,
                                             const UintVector& idvec, 
                                             std::string name,
                                             std::string /* material */, 
                                             const DoubleVector& /* params */, 
                                             VolumeType /* type */) {
    // concatenate the id for this node to current id.
    m_idcount.push_back(idvec.size());
    for ( UintVector::const_iterator u=idvec.begin(); u!=idvec.end(); ++u )
        m_idValues.push_back(static_cast<unsigned int>(*u));
    // is this what we want? add to the list if so and abort
    if ( name.find("SiLayerBox") != std::string::npos || 
         name.find("SiLadders") != std::string::npos ) {
        push_back(getId());
        return AbortSubtree;
    }
    // abort if we know it does not contain what we are looking for
    if ( name.substr(0,3) == "ACD" )
        return AbortSubtree;
    // otherwise continue
    return More;
}


void SiLayerList::popShape() {
    unsigned int count = m_idcount.back();
    m_idcount.pop_back();
    while ( count-- )
        m_idValues.pop_back();
}


idents::VolumeIdentifier SiLayerList::getId() const {
    idents::VolumeIdentifier id(m_prefix);
    for ( UintVector::const_iterator i=m_idValues.begin(); i!=m_idValues.end();
          ++i )
        id.append(*i);
    return id;
}
