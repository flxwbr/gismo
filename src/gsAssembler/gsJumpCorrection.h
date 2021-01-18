/** @file gsJumpCorrection.h

    @brief Provides assembler for a jump correction term.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): P. Weinmueller & R.
*/


#pragma once

#include <gsAssembler/gsAssembler.h>
#include <gsAssembler/gsVisitorJumpCorrection.h>

namespace gismo
{

template <class T, class bhVisitor = gsVisitorJumpCorrection<T> >
class gsJumpCorrection : public gsAssembler<T>
{
public:
    typedef gsAssembler<T> Base;

public:

    gsJumpCorrection( gsMultiPatch<T>         & patches,
                      gsMultiBasis<T>         & bases,
                      const boundaryInterface & item):
                      m_mp(patches), m_mb(bases), m_iface(item)
    {
        refresh();

        assemble();
    }

    void refresh();

    void assemble();

    void apply(bhVisitor visitor);

protected:

    gsMultiPatch<> & m_mp;
    gsMultiBasis<> & m_mb;

    const boundaryInterface & m_iface;

    std::vector<gsMatrix<T> > m_ddof;

    // Members from gsAssembler
    using Base::m_system;
    using Base::m_options;
};

template <class T, class bhVisitor>
void gsJumpCorrection<T,bhVisitor>::refresh()
{
    gsDofMapper map(m_mb.basis(m_iface.first().patch)); // Only one patch of the interface is considered

    index_t dir = m_iface.first().m_index < 3 ? 1 : 0; // If interface == West/East, then dir==1 (v)

    gsMatrix<index_t> act;
    // only the first u/v-columns are Dofs
    for (index_t i = 1; i < m_mb.basis(m_iface.first().patch).component(1-dir).size(); i++) // only for 2D !!!
    {
        act = m_mb.basis(m_iface.first().patch).boundaryOffset(m_iface.first().m_index, i);
        map.markBoundary(0, act); // Always Patch 0
    }

    map.finalize();
    //map.print();

    // 2. Create the sparse system
    m_system = gsSparseSystem<T>(map);
}

template <class T, class bhVisitor>
void gsJumpCorrection<T,bhVisitor>::assemble()
{
    GISMO_ASSERT(m_system.initialized(), "Sparse system is not initialized, call refresh()");

    // Reserve sparse system
    const index_t nz = gsAssemblerOptions::numColNz(m_mb[0],2,1,0.333333);
    m_system.reserve(nz, 1);

    // Compute the Dirichlet Degrees of freedom (if needed by m_options)
    m_ddof.resize(1);
    m_ddof[0].setZero(m_system.rowMapper(0).boundarySize(), m_system.unkSize(0));

    // Assemble volume integrals
    bhVisitor visitor;
    apply(visitor);

    // Assembly is done, compress the matrix
    Base::finalize();
}

template <class T, class bhVisitor>
void gsJumpCorrection<T,bhVisitor>::apply(bhVisitor visitor)
{
    std::vector<gsMultiBasis<T> > m_bases;
    m_bases.push_back(m_mb);
    const gsBasisRefs<T> bases(m_bases, m_iface.first().patch);

#pragma omp parallel
{
    gsQuadRule<T> quRule ; // Quadrature rule
    gsMatrix<T> quNodes  ; // Temp variable for mapped nodes
    gsVector<T> quWeights; // Temp variable for mapped weights

    bhVisitor
#ifdef _OPENMP
    // Create thread-private visitor
    visitor_(visitor);
    const int tid = omp_get_thread_num();
    const int nt  = omp_get_num_threads();
#else
    &visitor_ = visitor;
#endif

    // Initialize reference quadrature rule and visitor data
    visitor_.initialize(bases, m_iface.first().patch, m_options, quRule);

    gsGeometry<T> & patch = m_mp.patch(m_iface.first().patch);

    // Initialize domain element iterator -- using unknown 0
    typename gsBasis<T>::domainIter domIt = bases[0].makeDomainIterator(boundary::none);

    // Start iteration over elements
#ifdef _OPENMP
    for ( domIt->next(tid); domIt->good(); domIt->next(nt) )
#else
    for (; domIt->good(); domIt->next() )
#endif
    {
        // Map the Quadrature rule to the element
        quRule.mapTo( domIt->lowerCorner(), domIt->upperCorner(), quNodes, quWeights );

        // Perform required evaluations on the quadrature nodes
        visitor_.evaluate(bases, patch, quNodes);

        // Assemble on element
        visitor_.assemble(*domIt, quWeights);

        // Push to global matrix and right-hand side vector
#pragma omp critical(localToGlobal)
        visitor_.localToGlobal(0, m_ddof, m_system); // omp_locks inside
    }
}//omp parallel
}

} // namespace gismo



