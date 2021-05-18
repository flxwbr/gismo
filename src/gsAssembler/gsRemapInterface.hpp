/** @file gsRemapInterface.h

    @brief Provides a mapping between the corresponding sides of two patches sharing an interface

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Seiler, R. Schneckenleitner, S. Takacs
    Created on: 2018-06-12
*/

#pragma once

#include <gsCore/gsMultiPatch.h>
#include <gsUtils/gsSortedVector.h>
#include <gsNurbs/gsTensorNurbsBasis.h>
#include <gsModeling/gsCurveFitting.h>

namespace gismo {

template <class T>
gsRemapInterface<T>::gsRemapInterface(const gsMultiPatch<T>   & mp,
                                      const gsMultiBasis<T>   & basis,
                                      const boundaryInterface & bi,
                                      index_t checkAffine)
    : m_g1(&(mp[bi.first().patch])), m_g2(&(mp[bi.second().patch])),
      m_b1(&(basis[bi.first().patch])), m_b2(&(basis[bi.second().patch])),
      m_bi(bi),
      m_isMatching(true), m_isAffine(true)
{
    GISMO_ASSERT( m_g1->geoDim()==m_g2->geoDim(), "gsRemapInterface: Dimensions do not agree." );

    // First we construct the affine mapping
    computeBoundingBox();

    // Setup the affine mapping
    m_intfMap = gsAffineFunction<T>::make(bi.dirMap(m_bi.first()), bi.dirOrientation(m_bi.first()), m_parameterBounds1, m_parameterBounds2);

    // Next, we check (if so desired by called) if the affine mapping coincides with the real mapping
    GISMO_ASSERT( checkAffine > 0 || checkAffine == neverAffine || checkAffine == alwaysAffine,
        "gsRemapInterface: Parameter checkAffine has invalid value:" << checkAffine );
    if (checkAffine==neverAffine)
        m_isAffine = false;
    else if (checkAffine > 0)
        m_isAffine = checkIfAffine(checkAffine);

    if (m_isAffine)
        constructBreaksAffine();
    else
    {
        GISMO_ENSURE( m_isAffine || domainDim() <= 2,
            "gsRemapInterface: Can handle non-affine interfaces only for 2 dimensions." );

        constructReparam();
        constructBreaksNotAffine();
    }

}

// Used for the affine case

namespace {
template <class T>
gsMatrix<T> parameterBounds(const gsGeometry<T> & geo, boxSide s)
{
    gsMatrix<T> pr = geo.parameterRange();
    const index_t dim = pr.rows();
    gsMatrix<T> result(dim, 2);
    for (index_t i = 0; i<dim; ++i)
    {
        if (s.direction()==i)
            result(i,0) = result(i,1) = pr( i, s.parameter() == false ? 0 : 1 );
        else
            result.row(i) = pr.row(i);
    }
    return result;
}

template <class T>
gsMatrix<T> transferParameterBounds(const gsGeometry<T> & g1, const gsGeometry<T> & g2,
                const gsMatrix<T> & parameterBounds1, const gsMatrix<T> & parameterBounds2, T tolerance)
{
    gsVector<T> transfered[2];
    for (index_t j=0; j<2; ++j)
    {
        transfered[j] = parameterBounds1.col(j); // initial guess
        g1.newtonRaphson( g2.eval(parameterBounds2).col(j), transfered[j], true, tolerance, 100 );
    }

    gsMatrix<T> result(transfered[0].rows(), 2);
    for (index_t i=0; i<transfered[0].rows(); ++i)
    {
        result(i,0) = std::min( transfered[0][i], transfered[1][i] );
        result(i,1) = std::max( transfered[0][i], transfered[1][i] );
    }
    return result;

}

}

template <class T>
void gsRemapInterface<T>::computeBoundingBox()
{
    const T equalityTolerance = 0;
    const T newtonTolerance = 1e-8;

    m_parameterBounds1 = parameterBounds(*m_g1,m_bi.first());
    m_parameterBounds2 = parameterBounds(*m_g2,m_bi.second());

    gsMatrix<T> parameterBounds1trafsferedTo2
        = transferParameterBounds(*m_g2,*m_g1,m_parameterBounds2,m_parameterBounds1,newtonTolerance);
    gsMatrix<T> parameterBounds2trafsferedTo1
        = transferParameterBounds(*m_g1,*m_g2,m_parameterBounds1,m_parameterBounds2,newtonTolerance);

    for (index_t i=0; i<domainDim(); ++i)
    {
        if (parameterBounds2trafsferedTo1(i,0)>m_parameterBounds1(i,0)+equalityTolerance)
        {
            m_parameterBounds1(i,0) = parameterBounds2trafsferedTo1(i,0);
            m_isMatching = false;
        }
        if (parameterBounds2trafsferedTo1(i,1)<m_parameterBounds1(i,1)-equalityTolerance)
        {
            m_parameterBounds1(i,1) = parameterBounds2trafsferedTo1(i,1);
            m_isMatching = false;
        }

        if (parameterBounds1trafsferedTo2(i,0)>m_parameterBounds2(i,0)+equalityTolerance)
        {
            m_parameterBounds2(i,0) = parameterBounds1trafsferedTo2(i,0);
            m_isMatching = false;
        }
        if (parameterBounds1trafsferedTo2(i,1)<m_parameterBounds2(i,1)-equalityTolerance)
        {
            m_parameterBounds2(i,1) = parameterBounds1trafsferedTo2(i,1);
            m_isMatching = false;
        }
    }
}

template <class T>
bool gsRemapInterface<T>::checkIfAffine(index_t steps)
{
    gsVector<T> lower = m_parameterBounds1.col(0);
    gsVector<T> upper = m_parameterBounds1.col(1);
    gsVector<unsigned> numberGridPoints = gsVector<unsigned>::Constant(domainDim(),2+steps);
    numberGridPoints[m_bi.first().direction()] = 1;
    gsMatrix<T> points = gsPointGrid(lower,upper,numberGridPoints);

    return  (
                m_g1->eval(points)
                -
                m_g2->eval(m_intfMap->eval(points))
            ).norm() < (T)(1.e-6);
}


namespace {
template <class T, class Vector>
inline void addBreaks(std::vector< std::vector<T> >& breaks, const gsMatrix<T>& parameterBounds, const Vector& point)
{
    const T tolerance = 1.e-5;
    const index_t dim = point.rows();
    for (index_t d=0; d<dim; ++d)
    {
        const T t = point(d,0);
        if ( parameterBounds(d,0) <= t && t <= parameterBounds(d,1) )
        {
            // As in gsSortedVector::push_sorted_unique
            typename std::vector<T>::iterator pos = std::lower_bound(breaks[d].begin(), breaks[d].end(), t-tolerance );
            if ( pos == breaks[d].end() || *pos > t+tolerance ) // If not found
                breaks[d].insert(pos, t);
        }
    }
}
}

template <class T>
void gsRemapInterface<T>::constructBreaksAffine()
{

    gsAffineFunction<T> intfMap_inverse(m_bi.dirMap(m_bi.second()), m_bi.dirOrientation(m_bi.second()),
        m_parameterBounds2, m_parameterBounds1);
    m_breakpoints.resize(domainDim());

    const typename gsBasis<T>::domainIter domIt1 = m_b1->makeDomainIterator(m_bi.first());
    addBreaks(m_breakpoints, m_parameterBounds1, m_parameterBounds1.col(0));
    for (; domIt1->good(); domIt1->next())
        addBreaks(m_breakpoints, m_parameterBounds1, domIt1->upperCorner());
    addBreaks(m_breakpoints, m_parameterBounds1, m_parameterBounds1.col(1));

    const typename gsBasis<T>::domainIter domIt2 = m_b2->makeDomainIterator(m_bi.second());
    for (; domIt2->good(); domIt2->next())
        addBreaks(m_breakpoints, m_parameterBounds1, intfMap_inverse.eval(domIt2->upperCorner()));

}


// Used for the non-affine case


template <class T>
void gsRemapInterface<T>::constructBreaksNotAffine() {
    // assert domainDim()==2 taken care by constructor

    // computes break points per element

    const typename gsBasis<T>::domainIter domIt1 = m_b1->makeDomainIterator(static_cast<boxSide>(m_bi.first()));
    const typename gsBasis<T>::domainIter domIt2 = m_b2->makeDomainIterator(static_cast<boxSide>(m_bi.second()));

    const gsMatrix<T> startPatch1 = m_parameterBounds1.col(0);
    const gsMatrix<T> startPatch2 = m_parameterBounds2.col(0);

    // Compute interface knots in physical domain by evaluating left and right geometry maps at the knot values
    const size_t numelP1 = domIt1->numElements();
    const size_t numelP2 = domIt2->numElements();
    gsMatrix<T> physicalKnotsP1(m_g1->geoDim(), numelP1 + 1);
    gsMatrix<T> physicalKnotsP2(m_g2->geoDim(), numelP2 + 1);
    gsMatrix<T> dummy;

    domIt1->reset();
    domIt2->reset();
    index_t numBreaksPatch1 = 1, numBreaksPatch2 = 1; // vars to count the entries in the physical breakpoints

    // evaluate the first point of the interface
    m_g1->eval_into(startPatch1, dummy);
    physicalKnotsP1.col(0) = dummy;

    // loop over all elements of the boundary with interface part, but evaluate only element corners on the real interface
    for (; domIt1->good(); domIt1->next())
    {
        if (m_bi.first().direction() == 1) // v is fix
        {
            if (domIt1->lowerCorner()(0,0) > startPatch1(0,0) && domIt1->lowerCorner()(0,0) <= m_parameterBounds1(0,1))
            {
                m_g1->eval_into(domIt1->lowerCorner(), dummy);
                physicalKnotsP1.col(numBreaksPatch1) = dummy;
                numBreaksPatch1++;
            }
        }
        else if (m_bi.first().direction() == 0) // u is fix
        {
            if (domIt1->lowerCorner()(1,0) > startPatch1(1,0) && domIt1->lowerCorner()(1,0) <= m_parameterBounds1(1,1))
            {
                m_g1->eval_into(domIt1->lowerCorner(), dummy);
                physicalKnotsP1.col(numBreaksPatch1) = dummy;
                numBreaksPatch1++;
            }
        }
        //domIt1->next();
    }

    // evaluate the last point of the interface, i.e., this last point must also be within the parameter bound
    if (m_bi.first().direction() == 1)
    {
        if (domIt1->upperCorner()(0,0) <= m_parameterBounds1(0,1))
        {
            m_g1->eval_into(domIt1->upperCorner(), dummy);
            physicalKnotsP1.col(numBreaksPatch1) = dummy;
            numBreaksPatch1++;
        }
    }
    else if (m_bi.first().direction() == 0)
    {
        if (domIt1->upperCorner()(1,0) <= m_parameterBounds1(1,1))
        {
            m_g1->eval_into(domIt1->upperCorner(), dummy);
            physicalKnotsP1.col(numBreaksPatch1) = dummy;
            numBreaksPatch1++;
        }
    }
    //gsInfo << "physical knots 1: \n" << physicalKnotsP1 << "\n";



    // do the same for patch 2 as above
    m_g2->eval_into(startPatch2, dummy);
    physicalKnotsP2.col(0) = dummy;

    for (; domIt2->good(); domIt2->next()) // for (index_t i = 0; i < numelP2; i++)
    {
        if (m_bi.second().direction() == 1)
        {
            if (domIt2->lowerCorner()(0,0) > startPatch2(0,0) && domIt2->lowerCorner()(0,0) < m_parameterBounds2(0,1))
            {
                m_g2->eval_into(domIt2->lowerCorner(), dummy);
                physicalKnotsP2.col(numBreaksPatch2) = dummy;
                numBreaksPatch2++;
            }
        }
        else if(m_bi.second().direction() == 0)
        {
            if (domIt2->lowerCorner()(1,0) > startPatch2(1,0) && domIt2->lowerCorner()(1,0) < m_parameterBounds2(1,1))
            {
                m_g2->eval_into(domIt2->lowerCorner(), dummy);
                physicalKnotsP2.col(numBreaksPatch2) = dummy;
                numBreaksPatch2++;
            }
        }
        //domIt2->next();
    }

    // add only the breakpoints within the parameter bounds
    if (m_bi.second().direction() == 1)
    {
        if (domIt2->upperCorner()(0,0) <= m_parameterBounds2(0,1))
        {
            m_g2->eval_into(domIt2->upperCorner(), dummy);
            physicalKnotsP2.col(numBreaksPatch2) = dummy;
            numBreaksPatch2++;// to get the number of entries
        }
    }
    else if (m_bi.second().direction() == 0)
    {
        if(domIt2->upperCorner()(1,0) <= m_parameterBounds2(1,1))
        {
            m_g2->eval_into(domIt2->upperCorner(), dummy);
            physicalKnotsP2.col(numBreaksPatch2) = dummy;
            numBreaksPatch2++;
        }
    }
    //gsInfo << "physical knots 2: \n" << physicalKnotsP2 << "\n";

    // store all the physical points in one vector
    gsMatrix<T> physicalBreaks(domainDim(), numBreaksPatch1+numBreaksPatch2); // Assume m_g1->geoDim() == m_g2->geoDim()

    for (index_t c = 0; c < numBreaksPatch1; c++)
        physicalBreaks.col(c) = physicalKnotsP1.col(c);

    for (index_t c = 0; c < numBreaksPatch2; c++)
        physicalBreaks.col(numBreaksPatch1+c) = physicalKnotsP2.col(c);

    // compute the corresponding parameter values in one patch, here of patch1
    gsSortedVector<T> parameterBreaks;

    // Determine fixed coordinate of patch2 -> Use here patch2 because we compute the Interfacemap of patch1!!!
    // fixedDir ==  0 corresponds to fixed u and 1 corresponds to a fixed v
    index_t fixedDir = m_bi.first().direction();

    gsMatrix<T> G2_parametric_LC;
    for (index_t i = 0; i < physicalBreaks.cols(); i++) {
        // computes the preimages of the breakpoints for each of the two patches
        m_g1->invertPoints(physicalBreaks.col(i), G2_parametric_LC); // not exact, we have rounding errors
        // taking care of the rounding errors by iterating over the vector and checking the absolute value between the current
        // preimage and the already available ones
        if (fixedDir == 1)
        {
            if (parameterBreaks.size() == 0)
                parameterBreaks.push_sorted_unique(G2_parametric_LC(0, 0)); // sort w.r.t. u direction
            else
            {
                index_t j = 0;
                gsVector<bool> roundingError = gsVector<bool>::Constant(parameterBreaks.size(), true);

                for (typename gsSortedVector<T>::iterator it = parameterBreaks.begin();
                     it != parameterBreaks.end(); it++)
                {
                    if (math::abs(G2_parametric_LC(0, 0) - *it) > 1.e-4) {
                        roundingError(j) = false;
                    }
                    j++;
                }
                if (( false == roundingError.array() ).all())
                    parameterBreaks.push_sorted_unique(G2_parametric_LC(0, 0)); // sort w.r.t. u direction


            }
        }
        else
        {
            if (parameterBreaks.size() == 0)
                parameterBreaks.push_sorted_unique(G2_parametric_LC(1, 0)); // sort w.r.t. v direction
            else
            {
                index_t j = 0;
                gsVector<bool> roundingError = gsVector<bool>::Constant(parameterBreaks.size(), true);

                for (typename gsSortedVector<T>::iterator it = parameterBreaks.begin();
                     it != parameterBreaks.end(); it++)
                {
                    if (math::abs(G2_parametric_LC(1, 0) - *it) > 1.e-4)
                        roundingError(j) = false;

                    j++;
                }
                if (( false == roundingError.array() ).all())
                    parameterBreaks.push_sorted_unique(G2_parametric_LC(1, 0)); // sort w.r.t. v direction
            }
        }

    }

    m_breakpoints.resize(2);
    m_breakpoints[1-fixedDir] = parameterBreaks;

}


template <class T>
void gsRemapInterface<T>::constructReparam()
{
    // assert domainDim()==2 taken care by constructor

    const index_t numIntervals = 11; // ?
    const index_t numGeometries = 2;

    // Check if side 2 is to be flipped
    bool flipSide2 = false;

    if(m_bi.first().direction() == 1)
    {
        if(m_bi.dirOrientation()(0) == 0)
        {
            if(m_bi.second().direction() == 0) //change v parameters
                { flipSide2=true; }
            else if(m_bi.second().direction() == 1) // change u parameters
                { flipSide2=true; }
        }
    }

    if(m_bi.first().direction() == 0)
    {
        if(m_bi.dirOrientation()(1) == 0)
        {
            if(m_bi.second().direction() == 0) //change v parameters
                { flipSide2=true; }
            else if(m_bi.second().direction() == 1) // change u parameters
                { flipSide2=true; }
        }
    }

    // Assume tensor structure
    // now create samples for both patches
    // the knot intervals can be different, e.g.,
    //----------------------------
    //-                          -
    //----------------------------
    //    --------
    //    -      -
    //    --------
    gsMatrix<T> t_vals = gsMatrix<T>::Zero(numGeometries, numIntervals);
    T firstKnot, lastKnot;
    gsVector<T> upper(1), lower(1);
    gsVector<unsigned> numPoints(1);
    numPoints << numIntervals;

    //gsInfo << "parameterbounds: \n" << m_parameterBounds2 << "\n";
    //gsInfo << "patch: \n" << m_g2->id() << "\n";
    for (index_t np = 0; np < numGeometries; np++) {
        if (np == 0)
        {
            if (m_bi.first().direction() == 1) // v is fixed
            {
                firstKnot = m_parameterBounds1(0, 0);
                lastKnot = m_parameterBounds1(0, 1);
            }
            else // u is fixed
            {
                firstKnot = m_parameterBounds1(1, 0);
                lastKnot = m_parameterBounds1(1, 1);
            }
        } else {
            if (m_bi.second().direction() == 1) // v is fixed
            {
                firstKnot = m_parameterBounds2(0, flipSide2 ? 1 : 0);
                lastKnot = m_parameterBounds2(0, flipSide2 ? 0 : 1);
            }
            else // u is fixed
            {
                firstKnot = m_parameterBounds2(1, flipSide2 ? 1 : 0);
                lastKnot = m_parameterBounds2(1, flipSide2 ? 0 : 1);
            }
        }

        lower(0) = firstKnot;
        upper(0) = lastKnot;

        //gsInfo << "lower:\n" << firstKnot << "\n upper:\n" << lastKnot << std::endl;

        //t_vals.row(np) = uniformPointGrid(lower, upper, numIntervals); // uniformly distributed samples between the overlapping part of the interface
        t_vals.row(np) = gsPointGrid(lower, upper, numPoints);

    }

    gsMatrix<T> samples_left, samples_right;
    gsMatrix<T> find_start_value;

    // Get the corresponding edges
    //Edge 1, {(u,v) : u = 0}
    //Edge 2, {(u,v) : u = 1}
    //Edge 3, {(u,v) : v = 0}
    //Edge 4, {(u,v) : v = 1}

    //gsInfo << "left boundary: " << m_interfacePatch1 << "\n";
    //gsInfo << "right boundary: " << m_interfacePatch2 << "\n";

    //gsMatrix<T> vals2dPatch1(t_vals.rows()+1, t_vals.cols()), vals2dPatch2(t_vals.rows()+1, t_vals.cols());
    // TODO: use already available information
    gsMatrix<T> vals2dPatch1, vals2dPatch2;
    enrichToVector(m_bi.first(), *m_g1, t_vals.row(0), vals2dPatch1);
    enrichToVector(m_bi.second(), *m_g2, t_vals.row(1), vals2dPatch2);

    m_g1->eval_into(vals2dPatch1, samples_left);
    m_g2->eval_into(vals2dPatch2, samples_right);

    //gsInfo << "vals2dPatch1:\n" << GEO_L_ref.coefs() << "\n vals2dPatch2:\n" << GEO_R_ref.coefs() << std::endl;
    //gsInfo << "vals2dPatch1:\n" << vals2dPatch1 << "\n vals2dPatch2:\n" << vals2dPatch2 << std::endl;
    //std::cout << "samples left:\n" << samples_left << "\n samples right:\n" << samples_right << std::endl;

    gsMatrix<T> B(numIntervals, m_g1->geoDim());

    for (index_t i = 0; i < t_vals.cols(); i++)
    {
        // find a suitable start value for the Newton iteration
        find_start_value = (samples_right.colwise()) - samples_left.col(i);

        size_t row, col;

        find_start_value.colwise().squaredNorm().minCoeff(&row, &col);

        //gsVector<T> b_null = samples_right.col(col);
        gsVector<T> b_null = vals2dPatch2.col(col);

        // Pass on g2 if one wants to find a mapping from interface1 to interface2
        //gsMatrix<T> b = closestPoint(b_null, g2, samples_left.col(i));

        // this gives the same result as above
        m_g2->newtonRaphson(samples_left.col(i), b_null, true, 10e-6, 100);
        //gsInfo << "newton: " << b_null << "\n";

        // TODO: Check if the order of the coefficients has an impact on the mapping regarding assembling aso.
        B.row(i) = b_null.transpose(); // to be in the correct order

    }

    // the coefficients to fit
    //std::cout << "B:\n" << B << std::endl;

    // check the error
    // assume that the right map is the identity
    gsMatrix<T> eval_orig, eval_fit, B2, id;

    gsKnotVector<T> KV(t_vals(0, 0), t_vals(0, numIntervals - 1), 5, 4);

    gsCurveFitting<T> fit(t_vals.row(0).transpose(), B, KV);

    fit.compute();
    m_intfMap = fit.curve().clone();
    std::cout << "Hi, I'm the resulting curve: \n" << *m_intfMap << std::endl;

    unsigned errorInterval = 10;
    gsVector<unsigned > errorSamples(1);
    errorSamples << errorInterval;

    gsMatrix<T> eval_points;// = gsMatrix<T>::Zero(numGeometries, errorInterval);

    for (index_t np = 0; np < numGeometries; np++)
    {
        gsVector<T> lowerVal(1), upperVal(1);
        lowerVal << t_vals(np, 0);
        upperVal << t_vals(np, numIntervals - 1);
        //gsMatrix<T> grid = uniformPointGrid(lowerVal, upperVal, errorInterval);
        gsMatrix<T> grid = gsPointGrid(lowerVal, upperVal, errorSamples);
        eval_points.conservativeResize(np + 1, grid.cols()); // to check the error
        eval_points.row(np) = grid;
    }

    m_intfMap->eval_into(eval_points.row(0), eval_fit);
    //eval_fit(0,0) -= 0.0001; // do a nasty slight correction since the first entry is out of the domain of definition due to rounding errors

    // TODO: also here use already available information
    enrichToVector(m_bi.second(), *m_g2, eval_points.row(1), id);

    m_g2->eval_into(eval_fit, eval_orig);
    m_g2->eval_into(id, B2);
    //gsInfo << "b2: \n" << id.transpose() << " and eval_orig: \n" << eval_fit.transpose() << "\n";

    // do test
    /*
    for(index_t c = 0; c < eval_fit.cols(); c++)
    {
        if(std::isnan(eval_orig.col(c).squaredNorm()))
        {
            switch (m_bi.second().index())
            {
                case 1 :
                    // u = value of the first knot in u direction
                    eval_fit(0, c) += 10e-4;
                    eval_fit(1, c) += 10e-4;
                    break;
                case 2 :
                    //u = value of the last knot in u direction
                    eval_fit(0, c) -= 10e-4;
                    eval_fit(1, c) += 10e-4;
                    break;
                case 3 :
                    //v = value of the first knot in v direction;
                    eval_fit(0, c) += 10e-4;
                    eval_fit(1, c) += 10e-4;
                    break;
                case 4 :
                    //v = value of the last knot in v direction
                    eval_fit(0, c) += 10e-4;
                    eval_fit(1, c) -= 10e-4;
                    break;
            }
            m_g2->eval_into(eval_fit, eval_orig);
        }
    }
*/
    //end test

    T error = 0;

    for (index_t i = 0; i < eval_points.cols(); i++)
        error += (id.col(i) - eval_fit.col(i)).squaredNorm();
        //error += (eval_orig.col(i) - B2.col(i)).squaredNorm();

    error = math::sqrt(error);

    //if(error > 0.5)
    //    gsInfo << "patch 1: \n" << eval_orig << " and patch 2: \n" << B2 << "\n";

    std::cout << "Error: " << error << std::endl;

}

template <class T>
void gsRemapInterface<T>::eval_into(const gsMatrix<T>& u, gsMatrix<T>& result) const
{
    GISMO_ASSERT( u.rows() == domainDim(), "gsRemapInterface<T>::eval_into: "
        "The rows of the evaluation points must be equal to the dimension of the domain." );

    if (m_isAffine)
    {
        m_intfMap->eval_into(u, result);
    }
    else
    {
        // TODO: What is going on here?

        const index_t fixedDir = m_bi.first().direction();
        // v is fixed => loop over u values

        m_intfMap->eval_into(checkIfInBound(u.row(!fixedDir)), result); // ????

        // need here the second basis since result store points in the second geometry
        if (const gsTensorBSplineBasis<2, T> *tb = dynamic_cast<const gsTensorBSplineBasis<2, T> * >(&m_g2->basis()))
        {
            const short_t direction = m_bi.second().direction();
            result.row(direction).setConstant(m_bi.second().parameter() ?
                                               tb->knots(direction).last() :
                                               tb->knots(direction).first());

            return;
        }

        if (const gsTensorNurbsBasis<2, T> * ntb = dynamic_cast<const gsTensorNurbsBasis<2, T> * >(&(m_g2->basis())))
        {
            const short_t direction = m_bi.second().direction();
            result.row(direction).setConstant(m_bi.second().parameter() ?
                                               ntb->source().knots(direction).last() :
                                               ntb->source().knots(direction).first());
            return;
        }

        GISMO_ERROR("Unfitted interface not supported");
    }

}

template <class T>
typename gsDomainIterator<T>::uPtr gsRemapInterface<T>::makeDomainIterator() const
{
    gsTensorDomainBoundaryIterator<T> * tdi = new gsTensorDomainBoundaryIterator<T> (*m_b1, m_bi.first());
    for (index_t i=0; i<domainDim(); ++i)
    {
        if (i!=m_bi.first().direction())
            tdi->setBreaks(m_breakpoints[i],i);
    }
    return typename gsDomainIterator<T>::uPtr(tdi);
}

// Function to enhance a sequence of 1D points in an interval to 2D points in the parameter domain
// A matrix pts with the already correct dimensions is expected
// boundarySide is the boundary side according to gismo's boundary::side
// intervals is the number of samples to enrich to more dimensions
// pts is the matrix populated with the corresponding points for more dimensions
// only works for 2d at the moment!!
// TODO: Generalize for arbitrary dimensions
template <class T>
void gsRemapInterface<T>::enrichToVector(const boxSide         boundarySide,
                                         const gsGeometry<T> & geo,
                                         const gsMatrix<T>   & intervals,
                                               gsMatrix <T>  & pts)
{
    pts.resize(geo.geoDim(), intervals.cols());

    //const gsTensorBSplineBasis<2, T> *tb = dynamic_cast<const gsTensorBSplineBasis<2, T> * >(&geo.basis());

    //if(tb == NULL)
    //const gsTensorNurbsBasis<2, T> * tb = dynamic_cast<const gsTensorNurbsBasis<2, T> * >(&(geo.basis()));

    const index_t dim = 2;

    if(const gsTensorBSplineBasis<2, T> *tb = dynamic_cast<const gsTensorBSplineBasis<2, T> * >(&geo.basis()))
    {
        for (index_t i=0; i<dim; ++i)
        {
            if (boundarySide.direction()==i)
                pts.row(i) = gsMatrix<T>::Constant(1, intervals.cols(),
                    boundarySide.parameter()==0 ? tb->knots(0).first() : tb->knots(i).last()
                );
            else
                pts.row(i) = intervals;
        }
    }
    else
    {
        const gsTensorNurbsBasis<2, T> * ntb = dynamic_cast<const gsTensorNurbsBasis<2, T> * >(&(geo.basis()));

        for (index_t i=0; i<dim; ++i)
        {
            if (boundarySide.direction()==i)
                pts.row(i) = gsMatrix<T>::Constant(1, intervals.cols(),
                    boundarySide.parameter()==0 ? ntb->source().knots(0).first() : ntb->source().knots(i).last()
                );
            else
                pts.row(i) = intervals;
        }
    }
}

template <class T>
gsMatrix<T> gsRemapInterface<T>::checkIfInBound(const gsMatrix<T> & u) const
{
    // Here u contains only the coordinates in one direction
    gsMatrix<T> evalpts = u;

    const T begin = m_parameterBounds1(!m_bi.first().direction(), 0);
    const T end = m_parameterBounds1(!m_bi.first().direction(), 1);

    for(index_t c = 0; c < u.cols(); c++)
        if(u(0,c) - begin < 0)
            evalpts(0,c) += (begin - u(0,c));
        else
            break;


    for(index_t c = u.cols()-1; c > -1; c--)
        if(u(0, c) - end > 0)
            evalpts(0, c) -= (u(0, c) - end);
        else
            break;

    /*
    if(u(0,0) - begin < 0)
        evalpts(0,0) += (begin - u(0,0));


    if(u(0, u.cols()-1) - end > 0)
        evalpts(0, u.cols()-1) -= (u(0, u.cols()-1) - end);
    */

    return evalpts;
}

template <class T>
std::ostream& gsRemapInterface<T>::print(std::ostream& os) const
{
    os << "gsRemapInterface:"
       << "\n    First side:         " << m_bi.first()
       << "\n    Second side:        " << m_bi.second()
       << "\n    Is Affine:          " << ( m_isAffine   ? "yes" : "no")
       << "\n    Matching:           " << ( m_isMatching ? "yes" : "no")
       << "\n    Bounding box 1 min: " << m_parameterBounds1.transpose().row(0)
       << "\n                   max: " << m_parameterBounds1.transpose().row(1)
       << "\n    Bounding box 2 min: " << m_parameterBounds2.transpose().row(0)
       << "\n                   max: " << m_parameterBounds2.transpose().row(1);

    for (size_t i=0; i<m_breakpoints.size(); ++i)
    {
        os << "\n    Beakpoints " << (i<3?char('x'+i):' ') << ":     ";
        if ( m_breakpoints[i].size() <= 10 )
        {
            for (size_t j=0; j<m_breakpoints[i].size(); ++j)
                os << "  " << m_breakpoints[i][j];
        }
        else
        {
            for (size_t j=0; j<5; ++j)
                os << "  " << m_breakpoints[i][j];
            os << "  ...";
            for (size_t j=m_breakpoints[i].size()-5; j<m_breakpoints[i].size(); ++j)
                os << "  " << m_breakpoints[i][j];
        }
    }
    os << "\n";
    return os;
}


} // End namespace gismo
