/** @file tutorialPoissonMajorant.cpp

    @brief Example for testing the gsPoissonSolver
    with adaptive refinement based on THB-splines using functional error estimates for the error
    control and following from them indicators as a refinement criteria.

    This file is poissonAssemblerrt of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): S. Matculevich
*/

#include <iostream>

#include <gismo.h>

#include <gsAssembler/gsSeminormH1.h>
#include <gsAssembler/gsNormL2.h>
#include <gsAssembler/gsAdaptiveRefUtils.h>
#include <gsAssembler/gsErrEstPoissonResidual.h>

#include <gsErrorEstimates/gsTestMajorant.h>
#include <gsErrorEstimates/gsNormFields.h>

using namespace gismo;

// S. Matculevich
//
// This is a test example for a illustrating the adaptive refinement procedure implemented for the gsPoissonAssembler
// Flags, parameters, geometry and prescribed exact solution are specified within the main() function

void gsParseCommandLine(int argc, char **argv, bool plot)
{
    //! [Parse command line]
    gsCmdLine cmd("Tutorial on solving a Poisson problem with guaranteed error control using the funcional error estimate.");
    cmd.addSwitch("plot", "Create a ParaView visualization file with the gmresSolver", plot);
    cmd.getValues(argc,argv);
}

int main(int argc, char *argv[])
{

    // Define stopwatch to measure the performance of the routines
    gsCPUStopwatch clock, clock_total;
    clock_total.restart();

    //! [Initialize Test Parameters]
    // -------------------------------------------------------------------------------------------------------------- //
    // Define constants and preliminaries
    real_t rho = 1.0 / 16.0;
    real_t TOL = 1e-2;

    // Define parameters of I/O
    bool plotToParaview = false; // Flag indicating whether objects must be plotted in ParaView
    bool saveToFile     = false;  // Flag indicating whether objects must be plotted in ParaView
    bool isAdaptive     = false;

    try { gsParseCommandLine(argc, argv, plotToParaview); } catch (int rv) { return rv; }

    // Define test-case parameters
    // 2d
    const unsigned exampleNumber(2);        // 2 example: 2d unit square, u = (1 - x)*x*x*(1 - y)*y
    // int exampleNumber(3);        // 3 example: 2d unit square, u = sin(pi*x)*sin(pi*y)
    // int exampleNumber(4);        // 4 example: 2d unit square, u = sin(6.0*pi*x)*sin(3.0*pi*y)
    // int exampleNumber(6);        // 6 example: 2d unit square, u = (x^2 - x) * (y^2 - y) * exp(-100 * ((x - 0.8)^2 + (y - 0.05)^2))
    // int exampleNumber(7);        // 7 example: 2d rectangle $(0, 2) \times (0, 1)$, u = (2 - x)*x*x*(1 - y)*y
    // int exampleNumber(8);        // 8 example: 2d rectangle $(0, 2) \times (0, 1)$, u = (x^2 - 2*x) * (y^2 - y) * exp(-100 * ((x - 1.4)^2 + (y - 0.95)^2))
    // int exampleNumber(9);        // 9 example: 2d l-shape, u = if( y > 0, (x^2 + y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) - pi)/3.0 ), (x^2+y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) + 3.0*pi)/3.0 ) )
    // int exampleNumber(10);       // 10 example: 2d quater annulus, u = cos(x)*exp(y)
    // int exampleNumber(11);       // 11 example: 2d l-shape, u = if( y > 0, (x^2 + y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) - pi)/3.0 ), (x^2+y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) + 3.0*pi)/3.0 ) )
    // int exampleNumber(12);        // 12 example: 2d l-shape, u = if( y > 0, (x^2 + y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) - pi)/3.0 ), (x^2+y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) + 3.0*pi)/3.0 ) )
    // int exampleNumber(16);       // 16 example: 2d mix of quater annulus and l-shape domains, u = if( y > 0, (x^2 + y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) - pi)/3.0 ), (x^2+y^2)^(1.0/3.0) * sin( (2.0*atan2(y,x) + 3.0*pi)/3.0 ) )
    const unsigned d(2);

    // 3d
    //const unsigned exampleNumber(13);  // 3d unit cube: u = (1 - x)*x^2*(1 - y)*y^2*(1 - z)*z^2
    //const unsigned exampleNumber(14);  // 3d G-shape: u = cos(x)*exp(y)*10*z
    //const unsigned exampleNumber(18);    // 3d G-shape: u = tanh(1 - (x + 2*y + 4*z - 2))
    //const unsigned exampleNumber(19);    // 3d unit cube: u = tanh(1 - (x + 2*y + 4*z - 2))
    //const unsigned exampleNumber(20);    // 3d unit cube: u = sin(pi*x)*sin(pi*y)*sin(6*pi*x)
    //const unsigned exampleNumber(17);  // 3d unit cube: u = cos(x)*exp(y)
    //const unsigned d(3);    // Dimension must be prescribed

    gsTestMajorant<d> testMajorant(exampleNumber, isAdaptive);

    // Init the degree of basis S^{p, p}, where p = vDegree
    int vDegree(2), m(2), l(2);
    // Init the degree of the basis for the flux: y \in S^{p + k, p + k} + S^{p + k, p + k}, p = vDegree
    // yDegree must be >= 2 to ensure that S^{p + k, p + k} + S^{p + k, p + k} \subset H(\Omega, div§)
    int yDegree(vDegree + m);
    int wDegree(vDegree + l);

    // Refinement strategy
    int numInitUniformRefV(1), numInitUniformRefY(1), numInitUniformRefW(1), numTotalRef(6);    // Number of initial uniform  and total adaptive refinement steps
    //MarkingStrategy adaptRefCrit(GARU); // with alternatives GARU, PUCA, and BULK

    MarkingStrategy adaptRefCrit(BULK); // with alternatives GARU, PUCA, and BULK
    real_t markingParamTheta(0.4);
    int yBasisRefDelay(4);              // M, parameter for the delay in updating the flux for uniform and adaptive refinement
    int wBasisRefDelay(4);              // L

    testMajorant.gsCreateResultsFolder(saveToFile,
                                       vDegree, yDegree, wDegree,
                                       yBasisRefDelay, wBasisRefDelay,
                                       numTotalRef, adaptRefCrit, markingParamTheta);
    // -------------------------------------------------------------------------------------------------------------- //
    //! [Initialize Test Parameters]

    //! [Define Problem Data]
    // -------------------------------------------------------------------------------------------------------------- //
    gsFunctionExpr<> uDFunc, fFunc, uFunc;
    gsPiecewiseFunction<> alpha;
    gsBoundaryConditions<> bcInfo;
    testMajorant.gsInitializeProblemData(uDFunc, fFunc, alpha, uFunc, bcInfo);
    testMajorant.gsLogProblemData();
    // -------------------------------------------------------------------------------------------------------------- //
    //! [Define Problem Data]

    //! [Define Basis]
    // -------------------------------------------------------------------------------------------------------------- //
    gsMultiBasis<> basisV, basisY, basisW;
    testMajorant.gsGetInitialBasis(vDegree, yDegree, wDegree,
                                   basisV, basisY, basisW,
                                   numInitUniformRefV,
                                   numInitUniformRefY,
                                   numInitUniformRefW);
    // -------------------------------------------------------------------------------------------------------------- //
    //! [Define Basis]

    //! [Define Auxiliary Structures for Storing of the Results]
    // -------------------------------------------------------------------------------------------------------------- //
    // Initialize arrays to store the error and estimators on each refinement step
    gsVector<real_t> eH1Vector(numTotalRef),
            majVector(numTotalRef),
            mdVector(numTotalRef),
            meqVector(numTotalRef),
            minVector(numTotalRef),
            etaVector(numTotalRef),
            relErrorVector(numTotalRef-1),     // delta_i = || u_i - u_{i-1} ||, i > 0
            relError0Vector(numTotalRef-1),    // eps0_i = || u_i - u_0 ||, i > 0
            thetaVector(numTotalRef-1),       // theta_i = || u_i - u || / || u_{i-1} - u ||, i > 0
            stopcritVector(numTotalRef-1);   // stop_crit_i = (1 - theta_i) * TOL * eps0_i * rho, i > 0;
    // Initialize arrays to DOFs for v and y on each refinement step
    gsVector<index_t> vDOFs(numTotalRef), wDOFs(numTotalRef);
    gsMatrix<index_t> yDOFs(numTotalRef, testMajorant.patches.nPatches());
    // Initialize vectors with assembling and computation times on each refinement step
    gsVector<real_t> timeAsmbV(numTotalRef),
            timeAsmbDivDivY(numTotalRef),
            timeAsmbMMY(numTotalRef),
            timeAsmbY(numTotalRef),
            timeAsmbW(numTotalRef),
            timeEvalError(numTotalRef),
            timeEvalMajorant(numTotalRef),
            timeEvalMinorant(numTotalRef),
            timeEvalEtaIndicator(numTotalRef);
    timeEvalMajorant.setZero(numTotalRef);

    int numOfSolvers = 2;
    // matrix are used since we compare the performance of different solvers
    // [0] is the direct solver
    // [1] is the iterative solver
    gsMatrix<real_t> timeSolvV(numTotalRef, numOfSolvers),
            timeSolvY(numTotalRef, numOfSolvers),
            timeSolvW(numTotalRef, numOfSolvers),
            timeSolvEquilY(numTotalRef, numOfSolvers);
    timeSolvY.setZero(numTotalRef, 2);
    timeSolvV.setZero(numTotalRef, 2);
    timeSolvW.setZero(numTotalRef, 2);
    etaVector.setZero(numTotalRef);

    // Initialize auxiliary vector of all the basis' for y to store the history along the refinement
    std::vector< gsMultiBasis<> > basisYVector;
    std::vector< gsMultiBasis<> > basisWVector;

    std::vector< gsField<> > solutionFieldVector;
    std::vector< gsMultiPatch<> > solutionMPVector;

    // -------------------------------------------------------------------------------------------------------------- //
    //! [Define Auxiliary Structures to Store the Results]
    // Why first elimination and the l2Projection??
    gsPoissonHeterogeneousPde<real_t> poissonPde(testMajorant.patches, bcInfo, fFunc, alpha);
    gsPoissonHeterogeneousAssembler<real_t> poissonAssembler(poissonPde, basisV, (dirichlet::strategy) dirichlet::elimination);
    gsPoissonHeterogeneousAssembler<real_t> poissonAssemblerW(poissonPde, basisW, (dirichlet::strategy) dirichlet::elimination);
    poissonAssembler.options().setInt("DirichletValues", dirichlet::l2Projection);
    poissonAssemblerW.options().setInt("DirichletValues", dirichlet::l2Projection);

    gsMultiPatch<> mpV, mpY, mpW;
    gsMatrix<> vVector(1, 1), yVector(1, 1), wVector(1, 1); // [nDofsV x 1], [nDofsY x 1], [nDofsW x 1]
    gsField<> w;
    gsField<real_t> v;

    //! [Refinement Iterations]
    // -------------------------------------------------------------------------------------------------------------- //
    for( int refCount = 0; refCount < numTotalRef ; refCount++ )
    {
        testMajorant.gsLogRefinementBasisInfo(refCount, numTotalRef,
                                              poissonAssembler.multiBasis(0),
                                              basisY,
                                              poissonAssemblerW.multiBasis(0));

        // Reconstruct degrees of freedom, gsMultiPatch, and gsField representation of the approximation v
        testMajorant.gsRecontructV(refCount, poissonAssembler, bcInfo,
                                    vVector, mpV, v, vDOFs, stopcritVector,
                                    timeAsmbV, timeSolvV);
        // Store the last gsField- and sMultipatch-representation of v
        solutionFieldVector.push_back(v);
        solutionMPVector.push_back(mpV);
        //! [Recover the Approximation Field]

        //! [Error, Majorant (Optimal Flux), and Residual Estimate Computation]
        // ---------------------------------------------------------------------------------------------------------- //
        // ---------------------------------------------------------------------------------------------------------- //
        std::vector<size_t> elemNumber(testMajorant.patches.nPatches(), 0);
        for(size_t np = 0; np < testMajorant.patches.nPatches(); ++np) {
            elemNumber[np] += poissonAssembler.multiBasis(0).basis(np).numElements();
        }
        std::vector<real_t> mdDistr, majDistr, edDistr, etaDistr, minDistr;
        mdDistr.reserve(std::accumulate(elemNumber.begin(), elemNumber.end(), 0));
        edDistr.reserve(std::accumulate(elemNumber.begin(), elemNumber.end(), 0));
        etaDistr.reserve(std::accumulate(elemNumber.begin(), elemNumber.end(), 0));
        minDistr.reserve(std::accumulate(elemNumber.begin(), elemNumber.end(), 0));

        gsSeminormH1<real_t> eH1Seminorm(v, uFunc);
        gsErrEstPoissonResidual<real_t> etaIndicator(v, fFunc, bcInfo);

        clock.restart();

        // commentede ut to avoid the error in cdash
        //omp_set_nested(2);

        //#pragma omp parallel sections
        {
            //#pragma omp section
            {
                // Majorant Estimate and Optimal Flux Computation
                testMajorant.gsRecontructMajorantBasedOnOptimalFlux(refCount, basisY, yDegree,
                                                                    yVector, mpY, yDOFs,
                                                                    mpV, v, stopcritVector,
                                                                    fFunc,
                                                                    timeAsmbDivDivY, timeAsmbMMY, timeAsmbY, timeSolvY,
                                                                    timeEvalMajorant,
                                                                    majVector, mdVector, meqVector, mdDistr, majDistr,
                                                                    elemNumber);
            }/*
            //#pragma omp section
            {
                // Minorant Estimate and Optimal W  Computation
                testMajorant.gsRecontructMinorantBasedOnAuxiliaryW(refCount, poissonAssemblerW, bcInfo,
                                                                   wVector, mpW, w, wDOFs, stopcritVector,
                                                                   v, fFunc,
                                                                   timeAsmbW, timeSolvW, timeEvalMinorant,
                                                                   minVector, minDistr,
                                                                   elemNumber);

            }
        }
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                // Compute the residual between two successive iterations
                if (refCount > 0) {

                    gsField<> u_cur = solutionFieldVector[refCount];
                    gsMultiPatch<> u_cur_MP = solutionMPVector[refCount];
                    gsMultiPatch<> u_prev_MP = solutionMPVector[refCount-1];

                    gsMatrix<> u_prevIntValsFree, u_curIntPoints;
                    gsMultiPatch<> mp;
                    for(size_t np = 0; np < testMajorant.patches.nPatches(); ++np)
                    {
                        u_curIntPoints = u_cur.igaFunction(np).basis().anchors();
                        // truncate the fixed Dirichlet BC node and leave only free coefficients
                        testMajorant.interpolateToRefVectorWithDirichletBC(poissonAssembler, u_prev_MP, u_curIntPoints,
                                                                           u_prevIntValsFree, np);

                        mp.addPatch(u_cur_MP.piece(np));

                    }
                    //mp.computeTopology(); // TODO: Check if necessary/wrong
                    // reconstruct a multi-patch function from the obtained free coeffs.
                    gsMultiPatch<> u_prevIntMP;
                    poissonAssembler.constructSolution(u_prevIntValsFree, u_prevIntMP);

                    // reconstruct a field multi-patch function from the obtained free coeffs.
                    gsField<> u_prevInt(mp, u_prevIntMP);

                    gsNormFields<real_t> ucur_minus_uprev(u_cur, u_prevInt);
                    relErrorVector[refCount - 1] = ucur_minus_uprev.compute(false, elemNumber);
                }
            }
            #pragma omp section
            {
                if (refCount > 0) {
                    gsField<> u_cur = solutionFieldVector[refCount];

                    gsMultiPatch<> u_0_MP = solutionMPVector[0];
                    gsMultiPatch<> u_cur_MP = solutionMPVector[refCount];

                    gsMatrix<> u_0IntValsFree, u_curIntPoints;
                    gsMultiPatch<> mp;
                    for(size_t np = 0; np < testMajorant.patches.nPatches(); ++np)
                    {
                        u_curIntPoints = u_cur.igaFunction(np).basis().anchors();

                        // truncate the fixed Dirichlet BC node and leave only free coefficients
                        testMajorant.interpolateToRefVectorWithDirichletBC(poissonAssembler, u_0_MP, u_curIntPoints,
                                                                           u_0IntValsFree, np);

                        mp.addPatch(u_cur_MP.piece(np));
                    }
                    //mp.computeTopology(); //TODO: Is this step necessary/wrong?
                    // reconstruct a multi-patch function from the obtained free coeffs.
                    gsMultiPatch<> u_0IntMP;
                    poissonAssembler.constructSolution(u_0IntValsFree, u_0IntMP);
                    // reconstruct a field multi-patch function from the obtained free coeffs.
                    gsField<> u_0Int(mp, u_0IntMP);

                    gsNormFields<real_t> ucur_minus_u0(u_cur, u_0Int);
                    relError0Vector[refCount-1] = ucur_minus_u0.compute(false, elemNumber);
                }

            }
            #pragma omp section
            {
                // Compute the H1-seminorm of error
                testMajorant.gsCalculateDistribution(eH1Seminorm, edDistr, elemNumber,
                                                     eH1Vector, timeEvalError, refCount);
            }
        }
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                // Compute the residual error indicator
                testMajorant.gsCalculateDistribution(etaIndicator, etaDistr, elemNumber,
                                                     etaVector, timeEvalEtaIndicator, refCount);
            }
        }

        if (refCount > 0) {
            thetaVector[refCount-1]    = eH1Vector[refCount] / eH1Vector[refCount-1];
            stopcritVector[refCount-1] = (1 - thetaVector[refCount-1]) * TOL * rho * relError0Vector[refCount-1];
        }

        // Log computational costs results
        testMajorant.gsLogAssemblingSolvingTime(refCount,
                                                timeAsmbDivDivY, timeAsmbMMY, timeAsmbY, timeAsmbW,
                                                timeSolvV, timeSolvY, timeSolvW,
                                                timeEvalMajorant, timeEvalMinorant);
        //! [Error and Residual Estimate Computation]


        // Log and plotToParaview the results
        testMajorant.gsLogRefinementIterationInfo(refCount, vDOFs, yDOFs, wDOFs,
                                                  eH1Vector, relErrorVector, relError0Vector, thetaVector, stopcritVector,
                                                  majVector, minVector, etaVector);

        if (refCount <= numTotalRef - 1) {
            testMajorant.gsSaveToFileRefinementIterationInfo(saveToFile,
                                                             v, poissonAssembler.multiBasis(),
                                                             edDistr, mdDistr, refCount);
        }
        //! [Refine]
        if (refCount < numTotalRef - 1) {
            testMajorant.gsExecuteRefinement(poissonAssembler,
                                             basisY, poissonAssemblerW,
                                             basisYVector, basisWVector,
                                             mdDistr,
                                             adaptRefCrit,
                                             markingParamTheta,
                                             refCount, yBasisRefDelay,
                                             wBasisRefDelay);
            poissonAssembler.refresh();
            poissonAssemblerW.refresh();

            gsMatrix<> vRefVector, yRefVector, wRefVector;
            for(size_t np = 0; np < testMajorant.patches.nPatches(); ++np)
            {
                gsMatrix<> vInterpPoints = poissonAssembler.multiBasis().basis(np).anchors(); // newCoeff
                gsMatrix<> yInterpPoints = basisY.basis(np).anchors();
                gsMatrix<> wInterpPoints = poissonAssemblerW.multiBasis().basis(np).anchors(); // newCoeff

                testMajorant.interpolateToRefVectorWithDirichletBC(poissonAssembler, mpV, vInterpPoints, vRefVector, np);
                gsInfo << "size: " << vRefVector.rows() << "\n";

                yRefVector = mpY.patch(np).eval(yInterpPoints);
                yRefVector.transposeInPlace();

                testMajorant.interpolateToRefVectorWithDirichletBC(poissonAssemblerW, mpW, wInterpPoints, wRefVector, np);
            }
            testMajorant.gsSetVRefVector(vRefVector);
            testMajorant.gsSetYRefVector(yRefVector);
            testMajorant.gsSetWRefVector(wRefVector);
*/
        }

    }

    // -------------------------------------------------------------------------------------------------------------- //
    //! [Refinement Iterations]
    testMajorant.gsLogTestResults(vDegree, yDegree, wDegree,
                                  m, l,
                                  yBasisRefDelay, wBasisRefDelay,
                                  markingParamTheta, numInitUniformRefV, numTotalRef,
                                  vDOFs, yDOFs, wDOFs,
                                  timeAsmbV, timeAsmbDivDivY, timeAsmbMMY, timeAsmbY, timeAsmbW,
                                  timeSolvV, timeSolvY, timeSolvW,
                                  timeEvalError, timeEvalMajorant, timeEvalMinorant, timeEvalEtaIndicator,
                                  eH1Vector, relErrorVector, relError0Vector,
                                  majVector, mdVector, meqVector, minVector, etaVector);

    testMajorant.gsSaveToFileTestResults(saveToFile,
                                         vDOFs, yDOFs, wDOFs,
                                         eH1Vector, majVector, minVector, etaVector,
                                         numTotalRef);

    gsInfo << "\nTotal execution time : " << clock_total.stop() << "\n";
    return 0;
}
