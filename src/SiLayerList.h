/**
 * @class SiLayerList
 *
 * @brief This class uses detModel to make a list of all Si Layers,
 * from the element names.
 *
 * @author ??? ... has written this class.
 * @author Michael Kuss ... has only made this header and some formatting.
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/TkrDigi/src/SiLayerList.h,v 1.3 2004/02/27 10:14:13 kuss Exp $
 */

#ifndef SiLayerList_h
#define SiLayerList_h

#include <vector>

#include "GlastSvc/GlastDetSvc/IGeometry.h"
#include "idents/VolumeIdentifier.h"


class SiLayerList : public IGeometry,
            public std::vector<idents::VolumeIdentifier> {

 public:

    /**
     * Create the object.
     * Expect the function members shape, push, pop, and id to be called in
     * proper sequence.
     */
    SiLayerList(){};
    ~SiLayerList(){};

    /**
     * @param s type of the shape
     * @param id vector of unsigned ints (maybe null)
     * @param name
     * @param material
     * @param params vector with the six transformation parameters, followed by
     * 3 or so dimensions
     * @return tell caller whether to skip subvolumes or not.
     */
    virtual VisitorRet pushShape(ShapeType s, const UintVector& id,
                                 std::string name, std::string material,
                                 const DoubleVector& params, VolumeType type,
                                 SenseType sense);

    /// called to signal end of nesting
    virtual void popShape();

    /// current ID
    idents::VolumeIdentifier getId() const;

    void setPrefix(const idents::VolumeIdentifier prefix) { m_prefix = prefix; }

 private:

    typedef std::vector<unsigned int> UintVector;
    typedef std::vector<std::string> StringVector;

    /// vector of ids to describe current object
    UintVector m_idValues;

    /// should be equivalent
    idents::VolumeIdentifier m_volId;

    /// vector of the number of ids for this geometry level: 0, 1, or even 2
    UintVector m_idcount;

    std::vector<idents::VolumeIdentifier> m_idlist;

    /// the prefix to prepend
    idents::VolumeIdentifier m_prefix;

};

#endif
