/** @file gsRemapInterface.h

    @brief Provides a mapping from the patch side of geometry one to the corresponding patch side of geometry two

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Seiler, R. Schneckenleitner
    Created on: 2018-06-12
*/


#pragma once

#include <gsCore/gsAffineFunction.h>
#include <gsModeling/gsCurveFitting.h>
#include <gsModeling/gsFitting.h>

namespace gismo {

/// Provides a mapping from the patch side of geometry one to the corresponding patch side of geometry two
template <class T>
class gsRemapInterface : public gsFunction<T>
{
public:

    /// Shared pointer for gsRemapInterface
    typedef memory::shared_ptr< gsRemapInterface > Ptr;

    /// Unique pointer for gsRemapInterface
    typedef memory::unique_ptr< gsRemapInterface > uPtr;

    /// Unique pointer to domain iterator
    typedef memory::unique_ptr< gsDomainIterator<T> > domainIterUPtr;

public:

    /// Constructor which takes a multipatch and a boundary interface, useful if the interface is fully matching
    gsRemapInterface(const gsMultiPatch<T> & mp, const gsMultiBasis<T> & basis, const boundaryInterface & bi);

public:

    /// Returns the interface map
    const typename gsFunction<T>::Ptr & interfaceMap() const { return m_fittedInterface; }

    /// Returns parametric values on the boundary from patch1 to the corresponding boundary values on patch2
    virtual void eval_into(const gsMatrix<T>& u, gsMatrix<T>& result) const;

    /// Returns parameter dimension of the domain
    virtual short_t domainDim() const { return m_g1->geoDim(); }

    /// Returns a domain iterator
    domainIterUPtr makeDomainIterator() const;

    /// @brief Returns true iff the discretization is matching.
    ///
    /// In this case, the mapping is only affine-linear.
    bool isMatching() const { return m_isMatching; }

    /// Returns the break points
    const gsMatrix<T> & breakPoints() const { return m_breakpoints; }

    /// Prints the state of the object
    virtual std::ostream & print(std::ostream& os) const override;

private:

    // Computes \a m_parameterBounds1 and \a m_parameterBounds2 for the affine linear setting
    static gsMatrix<T> parameterBounds(const gsGeometry<T>& geo, boxSide s, index_t dim);

    // Helper to compute the closest point to lti on the other patch via Newton's method
    gsMatrix<T> closestPoint(const gsMatrix<T> b_null, const gsGeometry<T> & R, const gsMatrix<T> & lti);

    // rename: getPointsOnInterface() --> check eval_into, then evaluate both g1 and g2 and check equality
    // TODO: docs
    const std::vector<T> getPointsOnInterface() const;


    // Member to enrich a matrix of 1D points to a matrix of m_domain.geoDim() points
    void enrichToVector(boxSide boundarySide, const gsGeometry<T> & geo, const gsMatrix<T> & intervals, gsMatrix<T> & pts);

    // Find the interface between the two incoming patches
    void findInterface(const boundaryInterface& bi);

    // Check if the incoming patches are matching or not
    bool checkIfMatching();

    // Check if the incoming evaluation points are out of bounds because of rounding errors
    gsMatrix<T> checkIfInBound(const gsMatrix<T> & u) const;

    // Change dir direction of the parameterization of the patches
    void changeDir(const boundaryInterface & bi);

    // Constructs the reparametrization \a m_reparamInterfaceMap
    void constructReparam();

    // Constructs the breakpoints \a m_breakpoints
    void constructBreaks();

private:
    /// Geometry of first patch
    const gsGeometry<T> * m_g1;
    /// Geometry of second patch
    const gsGeometry<T> * m_g2;

    /// Basis on first patch
    const gsBasis<T> * m_b1;
    /// Basis on second patch
    const gsBasis<T> * m_b2;

    /// Side of first patch which constitutes interface
    patchSide m_side1;
    /// Side of second patch which constitutes interface
    patchSide m_side2;

    /// @brief True iff the interfaces are matching
    /// If true then an affine linear map is created, which is much faster
    bool m_isMatching;

    /// @brief True iff the orientation of the second side is fliped
    /// This is important for reparameterization, especially for constructing the breakpoints
    bool m_flipSide2;

    /// Union of breakpoints of both bases
    gsMatrix<T> m_breakpoints;

    /// The fitted interface itself
    typename gsFunction<T>::Ptr m_fittedInterface;

    /// @brief The bounds of the box that constitute the interface on the parameter domain for first patch
    /// The matrix has the structure [lower_1, ..., lower_d \\ upper_1, ..., upper_d ]
    gsMatrix<T> m_parameterBounds1;
    /// @brief The bounds of the box that constitute the interface on the parameter domain for first patch
    /// The matrix has the structure [lower_1, ..., lower_d \\ upper_1, ..., upper_d ]
    gsMatrix<T> m_parameterBounds2;


}; // End gsRemapInterface

/// Prints the state of the object
template <class T>
inline std::ostream & operator<<(std::ostream& os, const gsRemapInterface<T>& remapIf)
{ return remapIf.print(os); }

} // End namespace gismo

#ifndef GISMO_BUILD_LIB
#include GISMO_HPP_HEADER(gsRemapInterface.hpp)
#endif
