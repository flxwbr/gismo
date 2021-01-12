/** @file gsG1MultiBasis.h
    @brief Provides G1 Basis.

    This file is part of the G+Smo library.
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): P. Weinmüller
*/
#pragma once

namespace gismo
{
    template <class T>
    class gsG1MultiBasis
    {

    public:
        gsG1MultiBasis(gsMultiPatch<T> const         & patches,
                       const gsMultiBasis<T>               & bases)
                       : m_mp(patches), m_mb(bases)
        {
            // Create plus minus basis
            basis_pm.clear();
            for (size_t numInt = 0; numInt < m_mp.interfaces().size(); numInt++) // No loop for single patch!!!
            {
                const boundaryInterface &item = m_mp.interfaces()[numInt];

                index_t dir = item.first().m_index < 3 ? 1 : 0; // dir of interface

                // TODO assume that basis 2 is the same space
                gsBSplineBasis<> basis_int = dynamic_cast<gsBSplineBasis<>&>(m_mb.basis(item.first().patch).component(dir));

                //gsInfo << "Basis int : " << basis_int.knots().asMatrix() << "\n";

                index_t p = basis_int.degree();
                index_t r = p - basis_int.knots().multiplicityIndex(p+1); // r = p - m

                //gsInfo << "p: " << p << " r: " << r << "\n";

                // first,last,interior,mult_ends,mult_interior
                gsKnotVector<real_t> kv_plus(0, 1, 0, p + 1, p - 1 - r); // p,r+1 //-1 bc r+1
                gsBSplineBasis<> basis_plus(kv_plus);

                for (size_t i = p + 1; i < basis_int.knots().size() - (p + 1); i += basis_int.knots().multiplicityIndex(i))
                    basis_plus.insertKnot(basis_int.knot(i), p - 1 - r);

                basis_pm.push_back(basis_plus);
                //gsInfo << "Basis plus : " << basis_plus.knots().asMatrix() << "\n";

                gsKnotVector<real_t> kv_minus(0, 1, 0, p + 1 - 1, p - 1 - r); // p-1,r //-1 bc p-1
                gsBSplineBasis<> basis_minus(kv_minus);

                for (size_t i = p + 1; i < basis_int.knots().size() - (p + 1); i += basis_int.knots().multiplicityIndex(i))
                        basis_minus.insertKnot(basis_int.knot(i), p - 1 - r);

                basis_pm.push_back(basis_minus);
                //gsInfo << "Basis minus : " << basis_minus.knots().asMatrix() << "\n";

                // TODO assume that basis 2 is the same space
                basis_geo = dynamic_cast<gsBSplineBasis<>&>(m_mb.basis(item.first().patch).component(1-dir));
            }
        }

        index_t nPatches() { return m_mp.nPatches(); };
        index_t nBasisFunctions() { return basis_pm.size() > 0 ? basis_pm[0].size() + basis_pm[1].size() : 0; };

        void active_into(const gsMatrix<T> &, gsMatrix<index_t> &, index_t);

        void evalAllDers_into(const gsMatrix<T> & u, int n,
                              std::vector<gsMatrix<T> >& result, index_t patchIdx);

        void eval_into(const gsMatrix<T> & u, gsMatrix<T>& result, index_t patchIdx);
        //void deriv_into(const gsMatrix<T> & u, gsMatrix<T>& result, index_t patchIdx);
        void eval_deriv_into(const gsMatrix<T> & u, std::vector<gsMatrix<T>>& result, index_t patchIdx);
        //void deriv2_into(const gsMatrix<T> & u, gsMatrix<T>& result, index_t patchIdx);
        void eval_deriv_deriv2_into(const gsMatrix<T> & u, std::vector<gsMatrix<T>>& result, index_t patchIdx);


        void eval_alpha_S_into(const gsMatrix<T> & u, gsMatrix<T>& result, index_t patchIdx);
        void eval_beta_S_into(const gsMatrix<T> & u, gsMatrix<T>& result, index_t patchIdx);
        void eval_beta_into(const gsMatrix<T> & u, gsMatrix<T>& result);

    protected:
        gsMultiPatch<T> const m_mp;
        gsMultiBasis<T> m_mb;

        std::vector<gsBSplineBasis<>> basis_pm;

        gsBSplineBasis<> basis_geo;

        std::vector<gsMatrix<index_t>> g1active;
    }; // class gsG1MultiBasis

template <class T>
void gsG1MultiBasis<T>::eval_alpha_S_into(const gsMatrix <T> & points, gsMatrix <T> & result, int patchIdx)
{
    gsMatrix<> uv, ev;

    const boundaryInterface &item = m_mp.interfaces()[0];

    index_t dir = -1, idx_geo = -1;
    if (item.first().patch == patchIdx)
    {
        dir = item.first().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.first().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }
    else if (item.second().patch == patchIdx)
    {
        dir = item.second().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.second().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }

    if (idx_geo==1)
    {
        uv.setZero(2,points.cols());
        uv.row(dir) = points;
    }
    else if (idx_geo==basis_geo.size() - 2)
    {
        uv.setOnes(2,points.cols());
        uv.row(dir) = points;
    }

    // ======== Determine bar{alpha^(S)} ========
    const gsGeometry<> & P0 = m_mp.patch(patchIdx);

    result.setZero(1, points.cols());
    for (index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i), ev);
        result(0, i) = 1.0 * ev.determinant();
    }
}  // eval_alpha_S_into

template <class T>
void gsG1MultiBasis<T>::eval_beta_S_into(const gsMatrix <T> & points, gsMatrix <T> & result, int patchIdx)
{
    gsMatrix<> uv, ev;

    const boundaryInterface &item = m_mp.interfaces()[0];

    index_t dir = -1, idx_geo = -1;
    if (item.first().patch == patchIdx)
    {
        dir = item.first().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.first().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }
    else if (item.second().patch == patchIdx)
    {
        dir = item.second().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.second().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }

    if (idx_geo==1)
    {
        uv.setZero(2,points.cols());
        uv.row(dir) = points;
    }
    else if (idx_geo==basis_geo.size() - 2)
    {
        uv.setOnes(2,points.cols());
        uv.row(dir) = points;
    }

    // ======== Determine bar{alpha^(S)} ========
    const gsGeometry<> & P0 = m_mp.patch(patchIdx);


    const index_t d = m_mp.parDim();
    gsVector<> D0(d);

    // ======== Determine bar{beta}^L ========
    result.setZero(1, points.cols());
    for(index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i),ev);
        D0 = ev.col(dir);
        real_t D1 = 1/ D0.norm();
        result(0,i) = (patchIdx == 0 ? -1 : 1) * 1.0 * D1 * D1 * ev.col(1).transpose() * ev.col(0);
    }
}  // eval_beta_S_into

template <class T>
void gsG1MultiBasis<T>::eval_beta_into(const gsMatrix <T> & points, gsMatrix <T> & result)
{
    // TODO just work for 2 patches

    gsMatrix<T> uv0, uv1, ev0, ev1;

    uv0.setOnes(2, points.cols());
    uv0.row(1) = points; // v

    uv1.setZero(2, points.cols());
    uv1.row(1) = points; // v

    const gsGeometry<> &P0 = m_mp.patch(0); // iFace.first().patch = 1
    const gsGeometry<> &P1 = m_mp.patch(1); // iFace.second().patch = 0
    // ======================================

    const index_t d = 2;
    gsMatrix<> D0(d,d);

    // ======== Determine bar{beta} ========
    result.setZero(1, points.cols());
    for (index_t i = 0; i < uv1.cols(); i++) {
        P0.jacobian_into(uv0.col(i), ev0);
        P1.jacobian_into(uv1.col(i), ev1);

        D0.col(0) = ev0.col(0); // (DuFL, *)
        D0.col(1) = ev1.col(0); // (*,DuFR)

        result(0, i) = D0.determinant();
    }
} // eval_beta_into

template <class T>
void gsG1MultiBasis<T>::active_into(const gsMatrix <T> & points, gsMatrix<index_t> & active, index_t patchIdx)
{
    for (size_t numInt = 0; numInt < m_mp.interfaces().size(); numInt++) // No loop for single patch!!!
    {
        const boundaryInterface &item = m_mp.interfaces()[numInt];

        index_t dir = -1, idx_geo = -1;
        if (item.first().patch == patchIdx)
        {
            dir = item.first().m_index < 3 ? 1 : 0; // dir of interface
            idx_geo = item.first().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
        }
        else if (item.second().patch == patchIdx)
        {
            dir = item.second().m_index < 3 ? 1 : 0; // dir of interface
            idx_geo = item.second().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
        }

        gsMatrix<index_t> act_temp, act_plus, act_minus;
        basis_geo.active_into(points.row(1-dir), act_temp);

        for (index_t act_i = 0; act_i < act_temp.rows(); act_i++)
            if (act_temp(act_i,0) == idx_geo) // points is at the interface
            {
                basis_pm[0].active_into(points.row(dir), act_plus);
                basis_pm[1].active_into(points.row(dir), act_minus);
            }

        // For computing the basis function
        g1active.clear();
        g1active.push_back(act_plus);
        g1active.push_back(act_minus);

        // for getting the local dofs
        gsMatrix<index_t>ones(act_plus.rows(), act_plus.cols());
        ones.setOnes();
        active = act_plus + m_mb.basis(patchIdx).size() * ones;
        active.conservativeResize(active.rows() + act_minus.rows(), active.cols());

        ones.setOnes(act_minus.rows(), act_minus.cols());
        active.bottomRows(act_minus.rows()) = act_minus + (basis_pm[0].size() + m_mb.basis(patchIdx).size()) * ones;

    }
} // active_into

template<class T>
void gsG1MultiBasis<T>::evalAllDers_into(const gsMatrix<T> & u, int n,
                                    std::vector<gsMatrix<T> >& result, index_t patchIdx)
{
    result.resize(n+1);

    switch(n)
    {
        case 0:
            gsInfo << "I WAS HERE \n";
            eval_into(u, result[0], patchIdx);
            break;
        case 1:
            gsInfo << "I WAS HERE 2 \n";
            //eval_into (u, result[0], patchIdx);
            eval_deriv_into(u, result, patchIdx);
            break;
        case 2:
            //eval_into  (u, result[0], patchIdx);
            //eval_deriv_into (u, result, patchIdx);
            eval_deriv_deriv2_into(u, result, patchIdx);
            break;
        default:
            gsInfo << "Implementation for higher degree missing! \n";
            break;
    }
} // evalAllDers_into

template<class T>
void gsG1MultiBasis<T>::eval_into(const gsMatrix<T> & points, gsMatrix<T> & result, index_t patchIdx)
{
    if (g1active.size() == 0)
    {
        gsMatrix<index_t> act_plus, act_minus;
        gsVector<index_t> vec;
        vec.setLinSpaced(basis_pm[0].size(),0,basis_pm[0].size()-1);
        act_plus = vec;
        vec.setLinSpaced(basis_pm[1].size(), 0, basis_pm[1].size()-1);
        act_minus = vec;

        g1active.clear();
        g1active.push_back(act_plus);
        g1active.push_back(act_minus);
    }

    result.setZero(g1active[0].rows()+g1active[1].rows(), points.cols());

    const boundaryInterface &item = m_mp.interfaces()[0];

    index_t dir = -1, idx_geo = -1;
    if (item.first().patch == patchIdx)
    {
        dir = item.first().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.first().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }
    else if (item.second().patch == patchIdx)
    {
        dir = item.second().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.second().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }

    // tau/p
    real_t p = basis_geo.degree();
    real_t tau_1 = basis_geo.knots().at(p + 1); // p + 2 TODO Assume that mesh size is the same!!!

    gsMatrix<T> alpha, beta,
            N_0, N_1,
            N_j_minus, N_i_plus,
            der_N_i_plus;

    // Compute gluing data
    gsMatrix<> uv, ev;

    if (idx_geo==1)
    {
        uv.setZero(2,points.cols());
        uv.row(dir) = points.row(dir);
    }
    else if (idx_geo==basis_geo.size() - 2)
    {
        uv.setOnes(2,points.cols());
        uv.row(dir) = points.row(dir);
    }

    // ======== Determine bar{alpha^(S)} ========
    const gsGeometry<> & P0 = m_mp.patch(patchIdx);

    alpha.setZero(1, points.cols());
    for (index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i), ev);
        alpha(0, i) = (patchIdx == 0 ? -1 : 1) *  ev.determinant();
    }

    const index_t d = m_mp.parDim();
    gsVector<> D0(d);

    // ======== Determine bar{beta}^L ========
    beta.setZero(1, points.cols());
    for(index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i),ev);
        D0 = ev.col(dir);
        real_t D1 = 1/ D0.norm();
        beta(0,i) = (patchIdx == 0 ? -1 : 1) *  D1 * D1 * ev.col(1).transpose() * ev.col(0);

    }
    // End compute gluing data

    // Modify beta
    beta.setZero(); // TODO

    basis_geo.evalSingle_into(idx_geo == 1 ? 0 : idx_geo + 1, points.row(1-dir),N_0); // u
    basis_geo.evalSingle_into(idx_geo,points.row(1-dir),N_1); // u

    // TODO Make matrix multiplication instead of a for loop
    // Compute plus basis functions
    for (index_t i_plus = 0; i_plus < g1active[0].rows(); i_plus++)
    {
        index_t bfID = g1active[0](i_plus,0);

        basis_pm[0].evalSingle_into(bfID, points.row(dir), N_i_plus);
        basis_pm[0].derivSingle_into(bfID, points.row(dir), der_N_i_plus);

        gsMatrix <T> temp = beta.cwiseProduct(der_N_i_plus);

        result.row(i_plus) = N_i_plus.cwiseProduct(N_0 + N_1) - temp.cwiseProduct(N_1) * tau_1 / p;
    }
    //  End plus basis

    // Compute minus basis
    for (index_t i_minus = 0; i_minus < g1active[1].rows(); i_minus++)
    {
        index_t bfID = g1active[1](i_minus,0);

        basis_pm[1].evalSingle_into(bfID, points.row(dir), N_j_minus); // v

        result.row(g1active[0].rows() + i_minus) = alpha.cwiseProduct(N_j_minus.cwiseProduct(N_1)) * tau_1 / p;
    }
    // End minus basis

} // eval_into

template<class T>
void gsG1MultiBasis<T>::eval_deriv_into(const gsMatrix<T> & points, std::vector<gsMatrix<T>> & result, index_t patchIdx)
{
    if (g1active.size() == 0)
    {
        gsMatrix<index_t> act_plus, act_minus;
        gsVector<index_t> vec;
        vec.setLinSpaced(basis_pm[0].size(),0,basis_pm[0].size()-1);
        act_plus = vec;
        vec.setLinSpaced(basis_pm[1].size(), 0, basis_pm[1].size()-1);
        act_minus = vec;

        g1active.clear();
        g1active.push_back(act_plus);
        g1active.push_back(act_minus);
    }

    result.resize(2);
    result[0].setZero(g1active[0].rows()+g1active[1].rows(), points.cols());
    result[1].setZero(2*(g1active[0].rows()+g1active[1].rows()), points.cols());

    const boundaryInterface &item = m_mp.interfaces()[0];

    index_t dir = -1, idx_geo = -1;
    if (item.first().patch == patchIdx)
    {
        dir = item.first().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.first().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }
    else if (item.second().patch == patchIdx)
    {
        dir = item.second().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.second().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }

    // tau/p
    real_t p = basis_geo.degree();
    real_t tau_1 = basis_geo.knots().at(p + 1); // p + 2 TODO Assume that mesh size is the same!!!

    gsMatrix<T> alpha, beta,
            N_0, N_1,
            N_j_minus, N_i_plus,
            der_N_i_plus;

    // For the first derivative
    gsMatrix<T> der_alpha, der_beta,
            der_N_0, der_N_1,
            der_N_j_minus,
            der2_N_i_plus;

    // Compute gluing data
    gsMatrix<> uv, ev, ev2;

    if (idx_geo==1)
    {
        uv.setZero(2,points.cols());
        uv.row(dir) = points.row(dir);
    }
    else if (idx_geo==basis_geo.size() - 2)
    {
        uv.setOnes(2,points.cols());
        uv.row(dir) = points.row(dir);
    }

    // ======== Determine bar{alpha^(S)} ========
    const gsGeometry<> & P0 = m_mp.patch(patchIdx);

    alpha.setZero(1, points.cols());
    der_alpha.setZero(1, points.cols());
    for (index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i), ev);
        alpha(0, i) = (patchIdx == 0 ? -1 : 1) * ev.determinant();

        P0.deriv2_into(uv.col(i), ev2);
        if (dir == 1)
            der_alpha(0, i) = (patchIdx == 0 ? -1 : 1) * (ev2(2,0)*ev(1,1) + ev2(4,0)*ev(0,0) -
                                        ev2(1,0)*ev(1,0) - ev2(5,0)*ev(0,1));
        else if (dir == 0)
            der_alpha(0, i) = (patchIdx == 0 ? -1 : 1) * (ev2(0,0)*ev(1,1) + ev2(5,0)*ev(0,0) -
                                        ev2(2,0)*ev(1,0) - ev2(3,0)*ev(0,1));
    }

    const index_t d = m_mp.parDim();
    gsVector<> D0(d);

    // ======== Determine bar{beta}^L ========
    beta.setZero(1, points.cols());
    der_beta.setZero(1, points.cols());
    for(index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i),ev);
        D0 = ev.col(dir);
        real_t D1 = 1/ D0.norm();
        beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * ev.col(1).transpose() * ev.col(0);

        P0.deriv2_into(uv.col(i), ev2);
        D1 = 1/ D0.squaredNorm();
        real_t D2 = D0.squaredNorm();
        if (dir == 1)
            der_beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * (D2*(ev2(2,0)*ev(0,1) + ev2(1,0)*ev(0,0)+
                                                       ev2(5,0)*ev(1,1) + ev2(4,0)*ev(1,0)) -
                                                   (ev.col(1).transpose() * ev.col(0))(0,0) * 2.0 * (ev2(1,0)*ev(0,1) + ev2(4,0)*ev(1,1)));
        else if (dir == 0)
            der_beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * (D2*(ev2(0,0)*ev(0,1) + ev2(2,0)*ev(0,0)+
                                                       ev2(3,0)*ev(1,1) + ev2(5,0)*ev(1,0)) -
                                                   (ev.col(1).transpose() * ev.col(0))(0,0) * 2.0 * (ev2(0,0)*ev(0,0) + ev2(3,0)*ev(1,0)));
    }
    // End compute gluing data


    // ======== For modifying beta ========
    gsMatrix<T> zeroOne(2,2);
    zeroOne.setIdentity();

    // For modifying beta
    gsMatrix<T> alpha2, beta2;
    alpha2.setZero(1, zeroOne.cols());
    beta2.setZero(1, zeroOne.cols());

    const gsGeometry<> & PL = m_mp.patch(0); // Only in two Patch case TODO
    const gsGeometry<> & PR = m_mp.patch(1); // Only in two Patch case TODO

    gsMatrix<T> evZeroOne;
    PL.jacobian_into(zeroOne.col(0), evZeroOne);
    alpha2(0,0) = -1 * evZeroOne.determinant(); // alpha^L (0)

    D0 = evZeroOne.col(1); // dir of interface == 1
    real_t D1 = 1/ D0.norm();
    beta2(0,0) = -1 * D1 * D1 * evZeroOne.col(1).transpose() * evZeroOne.col(0); // beta^L (0)


    PR.jacobian_into(zeroOne.col(1), evZeroOne);
    alpha2(0,1) = evZeroOne.determinant(); // alpha^R (1)

    D0 = evZeroOne.col(1); // dir of interface == 1
    D1 = 1/ D0.norm();
    beta2(0,1) = D1 * D1 * evZeroOne.col(1).transpose() * evZeroOne.col(0); // beta^R (1)

    real_t lambdaL = beta2(0,0)/alpha2(0,0);
    real_t lambdaR = beta2(0,1)/alpha2(0,1);

    // Modify beta part 2
    gsMatrix<> ones;
    ones.setOnes(beta.rows(), beta.cols());
    beta = beta - lambdaL*(ones - points.row(dir)).cwiseProduct(alpha) - lambdaR*(points.row(dir)).cwiseProduct(alpha);

    der_beta = der_beta - lambdaL*(ones - points.row(dir)).cwiseProduct(der_alpha) - lambdaR*(points.row(dir)).cwiseProduct(der_alpha)
               + lambdaL*alpha - lambdaR*alpha;

    basis_geo.evalSingle_into(idx_geo == 1 ? 0 : idx_geo + 1, points.row(1-dir),N_0);
    basis_geo.evalSingle_into(idx_geo,points.row(1-dir),N_1);

    basis_geo.derivSingle_into(idx_geo == 1 ? 0 : idx_geo + 1, points.row(1-dir),der_N_0); // u
    basis_geo.derivSingle_into(idx_geo, points.row(1-dir),der_N_1); // u

    // TODO Make matrix multiplication instead of a for loop
    // Compute plus basis functions
    for (index_t i_plus = 0; i_plus < g1active[0].rows(); i_plus++)
    {
        index_t bfID = g1active[0](i_plus,0);

        basis_pm[0].evalSingle_into(bfID, points.row(dir), N_i_plus);
        basis_pm[0].derivSingle_into(bfID, points.row(dir), der_N_i_plus);
        basis_pm[0].deriv2Single_into(bfID, points.row(dir),der2_N_i_plus);

        gsMatrix <T> temp = beta.cwiseProduct(der_N_i_plus);
        gsMatrix<> der_temp = der_beta.cwiseProduct(der_N_i_plus) + beta.cwiseProduct(der2_N_i_plus);

        result[0].row(i_plus) = N_i_plus.cwiseProduct(N_0 + N_1) - temp.cwiseProduct(N_1) * tau_1 / p;

        result[1].row(2*i_plus + 1-dir) = N_i_plus.cwiseProduct(der_N_0 + der_N_1) - temp.cwiseProduct(der_N_1) * tau_1 / p;
        result[1].row(2*i_plus + dir) = der_N_i_plus.cwiseProduct(N_0 + N_1) - der_temp.cwiseProduct(N_1) * tau_1 / p;
    }
    //  End plus basis

    // Compute minus basis
    for (index_t i_minus = 0; i_minus < g1active[1].rows(); i_minus++)
    {
        index_t bfID = g1active[1](i_minus,0);

        basis_pm[1].evalSingle_into(bfID, points.row(dir), N_j_minus);
        basis_pm[1].derivSingle_into(bfID, points.row(dir),der_N_j_minus);

        result[0].row(g1active[0].rows() + i_minus) =  alpha.cwiseProduct(N_j_minus.cwiseProduct(N_1)) * tau_1 / p;

        result[1].row(2*g1active[0].rows() + 2*i_minus + 1-dir) = alpha.cwiseProduct(N_j_minus.cwiseProduct(der_N_1)) * tau_1 / p;
        result[1].row(2*g1active[0].rows() + 2*i_minus + dir) = (der_alpha.cwiseProduct(N_j_minus)+alpha.cwiseProduct(der_N_j_minus)).cwiseProduct(N_1) * tau_1 / p;
    }
    // End minus basis

} // deriv_into

template<class T>
void gsG1MultiBasis<T>::eval_deriv_deriv2_into(const gsMatrix<T> & points, std::vector<gsMatrix<T>> & result, index_t patchIdx)
{

    if (g1active.size() == 0 )
    {
        gsMatrix<index_t> act_plus, act_minus;
        gsVector<index_t> vec;
        vec.setLinSpaced(basis_pm[0].size(),0,basis_pm[0].size()-1);
        act_plus = vec;
        vec.setLinSpaced(basis_pm[1].size(), 0, basis_pm[1].size()-1);
        act_minus = vec;

        g1active.clear();
        g1active.push_back(act_plus);
        g1active.push_back(act_minus);
    }

    result.resize(3);
    result[0].setZero(g1active[0].rows()+g1active[1].rows(), points.cols());
    result[1].setZero(2*(g1active[0].rows()+g1active[1].rows()), points.cols());
    result[2].setZero(3*(g1active[0].rows()+g1active[1].rows()), points.cols());

    const boundaryInterface &item = m_mp.interfaces()[0];

    index_t dir = -1, idx_geo = -1;
    if (item.first().patch == patchIdx)
    {
        dir = item.first().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.first().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }
    else if (item.second().patch == patchIdx)
    {
        dir = item.second().m_index < 3 ? 1 : 0; // dir of interface
        idx_geo = item.second().m_index % 2 > 0 ? 1 : basis_geo.size() - 2; // 1 or n-1
    }

    // tau/p
    real_t p = basis_geo.degree();
    real_t tau_1 = basis_geo.knots().at(p + 1); // p + 2 TODO Assume that mesh size is the same!!!

    gsMatrix<T> alpha, beta,
            N_0, N_1,
            N_j_minus, N_i_plus,
            der_N_i_plus;

    // For the first derivative
    gsMatrix<T> der_alpha, der_beta,
            der_N_0, der_N_1,
            der_N_j_minus,
            der2_N_i_plus;

    // For the second derivative
    gsMatrix<T> der2_alpha, der2_beta,
            der2_N_0, der2_N_1,
            der2_N_j_minus,
            der3_N_i_plus;

    // Compute gluing data
    gsMatrix<> uv, ev, ev2, ev3;

    if (idx_geo==1)
    {
        uv.setZero(2,points.cols());
        uv.row(dir) = points.row(dir);
    }
    else if (idx_geo==basis_geo.size() - 2)
    {
        uv.setOnes(2,points.cols());
        uv.row(dir) = points.row(dir);
    }

    // ======== Determine bar{alpha^(S)} ========
    const gsGeometry<> & P0 = m_mp.patch(patchIdx);

    alpha.setZero(1, points.cols());
    der_alpha.setZero(1, points.cols());
    der2_alpha.setZero(1, points.cols());
    for (index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i), ev);
        alpha(0, i) = (patchIdx == 0 ? -1 : 1) * ev.determinant();

        P0.deriv2_into(uv.col(i), ev2);
        if (dir == 1)
            der_alpha(0, i) = (patchIdx == 0 ? -1 : 1) * (ev2(2,0)*ev(1,1) + ev2(4,0)*ev(0,0) -
                ev2(1,0)*ev(1,0) - ev2(5,0)*ev(0,1));
        else if (dir == 0)
            der_alpha(0, i) = (patchIdx == 0 ? -1 : 1) * (ev2(0,0)*ev(1,1) + ev2(5,0)*ev(0,0) -
                ev2(2,0)*ev(1,0) - ev2(3,0)*ev(0,1));

        std::vector<gsMatrix<>> ders;
        m_mp.patch(patchIdx).basis().evalAllDersFunc_into(uv.col(i),m_mp.patch(patchIdx).coefs(),4,ders); // TODO before
        ev3 = ders[3];

        if (dir == 1)
            der2_alpha(0,i) = (patchIdx == 0 ? -1 : 1) * (-2 * ev2(5,0)*ev2(1,0) + 2 * ev2(2,0)*ev2(4,0) +
                    ev(1,1)*ev3(3,0) - ev(0,1)*ev3(7,0) - ev(1,0)*ev3(1,0) +
                    ev(0,0)*ev3(5,0));

    }

    // ======== For modifying beta ========
    gsMatrix<T> zeroOne(2,2);
    zeroOne.setIdentity();

    // For modifying beta
    gsMatrix<T> alpha2, beta2;
    alpha2.setZero(1, zeroOne.cols());
    beta2.setZero(1, zeroOne.cols());

    const gsGeometry<> & PL = m_mp.patch(0); // Only in two Patch case TODO
    const gsGeometry<> & PR = m_mp.patch(1); // Only in two Patch case TODO

    gsMatrix<T> evZeroOne;
    PL.jacobian_into(zeroOne.col(0), evZeroOne);
    alpha2(0,0) = -1 * evZeroOne.determinant(); // alpha^L (0)

    const index_t d = m_mp.parDim();
    gsVector<> D0(d);
    D0 = evZeroOne.col(1); // dir of interface == 1
    real_t D1 = 1/ D0.norm();
    beta2(0,0) = -1 * D1 * D1 * evZeroOne.col(1).transpose() * evZeroOne.col(0); // beta^L (0)


    PR.jacobian_into(zeroOne.col(1), evZeroOne);
    alpha2(0,1) = evZeroOne.determinant(); // alpha^R (1)

    D0 = evZeroOne.col(1); // dir of interface == 1
    D1 = 1/ D0.norm();
    beta2(0,1) = D1 * D1 * evZeroOne.col(1).transpose() * evZeroOne.col(0); // beta^R (1)

    real_t lambdaL = beta2(0,0)/alpha2(0,0);
    real_t lambdaR = beta2(0,1)/alpha2(0,1);

    // ======== Determine bar{beta}^L ========
    beta.setZero(1, points.cols());
    der_beta.setZero(1, points.cols());
    der2_beta.setZero(1, points.cols());
    for(index_t i = 0; i < uv.cols(); i++)
    {
        P0.jacobian_into(uv.col(i),ev);
        D0 = ev.col(dir);
        real_t D1 = 1/ D0.norm();
        beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * ev.col(1).transpose() * ev.col(0);

        P0.deriv2_into(uv.col(i), ev2);
        D1 = 1/ D0.squaredNorm();
        real_t D2 = D0.squaredNorm();
        if (dir == 1)
            der_beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * (D2*(ev2(2,0)*ev(0,1) + ev2(1,0)*ev(0,0)+
                ev2(5,0)*ev(1,1) + ev2(4,0)*ev(1,0)) -
                (ev.col(1).transpose() * ev.col(0))(0,0) * 2.0 * (ev2(1,0)*ev(0,1) + ev2(4,0)*ev(1,1)));
        else if (dir == 0)
            der_beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * (D2*(ev2(0,0)*ev(0,1) + ev2(2,0)*ev(0,0)+
                ev2(3,0)*ev(1,1) + ev2(5,0)*ev(1,0)) -
                (ev.col(1).transpose() * ev.col(0))(0,0) * 2.0 * (ev2(0,0)*ev(0,0) + ev2(3,0)*ev(1,0)));

        std::vector<gsMatrix<>> ders;
        m_mp.patch(patchIdx).basis().evalAllDersFunc_into(uv.col(i),m_mp.patch(patchIdx).coefs(),4,ders); // TODO before
        ev3 = ders[3];

        if (dir == 1)
            der2_beta(0,i) = (patchIdx == 0 ? -1 : 1) * D1 * D1 * D1 * (
                    -4 * D2 * (ev(0,1)*ev2(2,0) + ev(1,1)*ev2(5,0) +
                    ev(0,0)*ev2(1,0) + ev(1,0)*ev2(4,0)) *
                    (ev(0,1)*ev2(1,0) + ev(1,1)*ev2(4,0))
                    +
                    D2 * D2 * (2 * ev2(2,0)*ev2(1,0) + 2 * ev2(5,0)*ev2(4,0) +
                    ev(0,1)*ev3(3,0) + ev(1,1)*ev3(7,0) +
                    ev(0,0)*ev3(1,0) + ev(1,0)*ev3(4,0))
                    +
                    (ev(0,0)*ev(0,1) + ev(1,0)*ev(1,1)) *
                    (8 * (ev(0,1)*ev2(1,0) + ev(1,1)*ev2(4,0)) *
                    (ev(0,1)*ev2(1,0) + ev(1,1)*ev2(4,0)) -
                    2 * (ev(0,1)*ev(0,1) + ev(1,1)*ev(1,1)) *
                    (ev2(1,0)*ev2(1,0) + ev2(4,0)*ev2(4,0) +
                    ev(0,1)*ev3(1,0) + ev(1,1)*ev3(5,0))
                    )
                    );
    }

    gsInfo << "Beta: " << der2_beta << "\n";

    // Modify beta part 2
    gsMatrix<> ones;
    ones.setOnes(beta.rows(), beta.cols());
    beta = beta - lambdaL*(ones - points.row(dir)).cwiseProduct(alpha) - lambdaR*(points.row(dir)).cwiseProduct(alpha);

    der_beta = der_beta - lambdaL*(ones - points.row(dir)).cwiseProduct(der_alpha) - lambdaR*(points.row(dir)).cwiseProduct(der_alpha)
            + lambdaL*alpha - lambdaR*alpha;

    der2_beta = der2_beta - lambdaL*((ones - points.row(dir)).cwiseProduct(der2_alpha) - 2.0*der_alpha) -
            lambdaR*((points.row(dir)).cwiseProduct(der2_alpha) + 2.0*der_alpha);

    // End compute gluing data

    basis_geo.evalSingle_into(idx_geo == 1 ? 0 : idx_geo + 1, points.row(1-dir),N_0);
    basis_geo.evalSingle_into(idx_geo,points.row(1-dir),N_1);

    basis_geo.derivSingle_into(idx_geo == 1 ? 0 : idx_geo + 1, points.row(1-dir),der_N_0);
    basis_geo.derivSingle_into(idx_geo, points.row(1-dir),der_N_1);

    basis_geo.deriv2Single_into(idx_geo == 1 ? 0 : idx_geo + 1, points.row(1-dir),der2_N_0); // u
    basis_geo.deriv2Single_into(idx_geo, points.row(1-dir),der2_N_1); // u

    // TODO Make matrix multiplication instead of a for loop
    // Compute plus basis functions
    for (index_t i_plus = 0; i_plus < g1active[0].rows(); i_plus++)
    {
        index_t bfID = g1active[0](i_plus,0);

        basis_pm[0].evalSingle_into(bfID, points.row(dir), N_i_plus);
        basis_pm[0].derivSingle_into(bfID, points.row(dir), der_N_i_plus);
        basis_pm[0].deriv2Single_into(bfID, points.row(dir),der2_N_i_plus);
        basis_pm[0].evalDerSingle_into(bfID, points.row(dir),3,der3_N_i_plus);

        gsMatrix<T> temp = beta.cwiseProduct(der_N_i_plus);
        gsMatrix<T> der_temp = der_beta.cwiseProduct(der_N_i_plus) + beta.cwiseProduct(der2_N_i_plus);
        gsMatrix<T> der2_temp = 2.0*der_beta.cwiseProduct(der2_N_i_plus) + beta.cwiseProduct(der3_N_i_plus) + der2_beta.cwiseProduct(der_N_i_plus);

        result[0].row(i_plus) = N_i_plus.cwiseProduct(N_0 + N_1) - temp.cwiseProduct(N_1) * tau_1 / p;

        result[1].row(2*i_plus + 1-dir) = N_i_plus.cwiseProduct(der_N_0 + der_N_1) - temp.cwiseProduct(der_N_1) * tau_1 / p;
        result[1].row(2*i_plus + dir) = der_N_i_plus.cwiseProduct(N_0 + N_1) - der_temp.cwiseProduct(N_1) * tau_1 / p;

        result[2].row(3*i_plus + 1-dir) = N_i_plus.cwiseProduct(der2_N_0 + der2_N_1) - temp.cwiseProduct(der2_N_1) * tau_1 / p;
        result[2].row(3*i_plus + dir) = der2_N_i_plus.cwiseProduct(N_0 + N_1) - der2_temp.cwiseProduct(N_1) * tau_1 / p;
        result[2].row(3*i_plus + 2) = der_N_i_plus.cwiseProduct(der_N_0 + der_N_1) - der_temp.cwiseProduct(der_N_1) * tau_1 / p;
    }
    //  End plus basis

    // Compute minus basis
    for (index_t i_minus = 0; i_minus < g1active[1].rows(); i_minus++)
    {
        index_t bfID = g1active[1](i_minus,0);

        basis_pm[1].evalSingle_into(bfID, points.row(dir), N_j_minus);
        basis_pm[1].derivSingle_into(bfID, points.row(dir),der_N_j_minus);
        basis_pm[1].deriv2Single_into(bfID, points.row(dir), der2_N_j_minus); // v

        result[0].row(g1active[0].rows() + i_minus) = alpha.cwiseProduct(N_j_minus.cwiseProduct(N_1)) * tau_1 / p; // (dir == 0 ? -1 : 1)

        result[1].row(2*g1active[0].rows() + 2*i_minus + 1-dir) = alpha.cwiseProduct(N_j_minus.cwiseProduct(der_N_1)) * tau_1 / p;
        result[1].row(2*g1active[0].rows() + 2*i_minus + dir) = (der_alpha.cwiseProduct(N_j_minus)+alpha.cwiseProduct(der_N_j_minus)).cwiseProduct(N_1) * tau_1 / p;

        result[2].row(3*g1active[0].rows() + 3*i_minus + 1-dir) = alpha.cwiseProduct(N_j_minus.cwiseProduct(der2_N_1)) * tau_1 / p;
        result[2].row(3*g1active[0].rows() + 3*i_minus + dir) = (2.0*der_alpha.cwiseProduct(der_N_j_minus)+alpha.cwiseProduct(der2_N_j_minus)+der2_alpha.cwiseProduct(N_j_minus)).cwiseProduct(N_1) * tau_1 / p;
        result[2].row(3*g1active[0].rows() + 3*i_minus + 2) = (der_alpha.cwiseProduct(N_j_minus)+alpha.cwiseProduct(der_N_j_minus)).cwiseProduct(der_N_1) * tau_1 / p;
    }
    // End minus basis
} // deriv2_into


} // namespace gismo

