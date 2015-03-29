/** @file gsBasis.h

    @brief Provides declaration of tensor-product B-spline basis class.

    This file is part of the G+Smo library. 

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
    Author(s): A. Mantzaflaris
*/

#include <gsCore/gsTemplateTools.h>

#include <gsNurbs/gsTensorBSplineBasis.h>
#include <gsNurbs/gsKnotVector.h>


#include <gsIO/gsXml.h>
#include <gsIO/gsXmlGenericUtils.hpp>


namespace gismo
{


template<unsigned d, class T, class KnotVectorType>
void gsTensorBSplineBasis<d,T,KnotVectorType>::
active_cwise(const gsMatrix<T> & u, 
             gsVector<unsigned,d>& low, 
             gsVector<unsigned,d>& upp ) const
{
    for (index_t j = 0; j < u.cols(); ++j)
    {
        for (unsigned i = 0; i < d; ++i)
        {
            low[i] = component(i).firstActive( u(i,j) );
            upp[i] = low[i] + component(i).degree();
        }
    }
}


/*
 * NB:
 * This implementation assumes that the component bases implement the firstActive() / numActive() protocol.
 * In particular, their active basis functions must always be continuous intervals.
 * This is the case for all current component bases, so we only keep this version for now.
 * Above, commented out, is the generic version which is quite a bit slower.
 */
template<unsigned d, class T, class KnotVectorType>
void gsTensorBSplineBasis<d,T,KnotVectorType>::
active_into(const gsMatrix<T> & u, gsMatrix<unsigned>& result) const
{
    unsigned firstAct[d];
    gsVector<unsigned, d> v, size;

    // count active functions in each tensor direction
    unsigned numAct = 1;
    for (unsigned i = 0; i < d; ++i)
    {
        size[i] = component(i).numActive();
        numAct *= size[i];
    }

    result.resize( numAct, u.cols() );  
  
    // Fill with active bases indices
    for (index_t j = 0; j < u.cols(); ++j)
    {
        // get the active basis indices for the component bases at u(:,j)
        for (unsigned i = 0; i < d; ++i)
        {
            firstAct[i] = component(i).firstActive( u(i,j) );
        }

        // iterate over all tensor product active functions
        unsigned r = 0;
        v.setZero();
        do
        {
            int gidx = firstAct[d-1] + v(d-1);    //compute global index in the tensor product
            for ( int i=d-2; i>=0; --i )
                gidx = gidx * this->size(i) + firstAct[i] + v(i);

            result(r, j) = gidx;
            ++r ;
        } while (nextLexicographic(v, size));
    }
}


namespace internal
{

/// @brief Get a TensorBSplineBasis from XML data
template<unsigned d, class T, class KnotVectorType>
class gsXml< gsTensorBSplineBasis<d,T,KnotVectorType> >
{
private:
    gsXml() { }
public:
    GSXML_COMMON_FUNCTIONS(gsTensorBSplineBasis<TMPLA3(d,T,KnotVectorType)>);
    static std::string tag () { return "Basis"; }
    static std::string type () { return "TensorBSplineBasis"+to_string(d); }

    static gsTensorBSplineBasis<d,T,KnotVectorType> * get (gsXmlNode * node)
    {
        return getTensorBasisFromXml<gsTensorBSplineBasis<d,T,KnotVectorType> >( node );
    }
    
    static gsXmlNode * put (const gsTensorBSplineBasis<d,T,KnotVectorType> & obj, 
                            gsXmlTree & data )
    {
        return putTensorBasisToXml<gsTensorBSplineBasis<d,T,KnotVectorType> >(obj,data);
    }
};

} // internal

} // gismo

