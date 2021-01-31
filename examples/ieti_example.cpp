/** @file ieti_example.cpp

    @brief Provides examples for the ieti solver.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): S. Takacs
*/

#include <gismo.h>
#include <gsIeti/gsIetiMapper.h>
#include <gsIeti/gsIetiSystem.h>
#include <gsIeti/gsScaledDirichletPrec.h>

using namespace gismo;

int main(int argc, char *argv[])
{
    /************** Define command line options *************/

    std::string geometry("domain2d/yeti_mp2.xml");
    index_t splitPatches = 0;
    real_t stretchGeometry = 1;
    index_t refinements = 3;
    index_t degree = 2;
    real_t tolerance = 1.e-8;
    index_t maxIterations = 100;
    bool plot = false;
    bool sol = false;
    std::string boundaryConditions("d");

    gsCmdLine cmd("Solves a PDE with an isogeometric discretization using a multigrid solver.");
    cmd.addString("g", "Geometry",              "Geometry file", geometry);
    cmd.addInt   ("",  "SplitPatches",          "Split every patch that many times in 2^d patches", splitPatches);
    cmd.addReal  ("",  "StretchGeometry",       "Stretch geometry in x-direction by the given factor", stretchGeometry);
    cmd.addInt   ("r", "Refinements",           "Number of uniform h-refinement steps to perform before solving", refinements);
    cmd.addInt   ("p", "Degree",                "Degree of the B-spline discretization space", degree);
    cmd.addReal  ("t", "Solver.Tolerance",      "Stopping criterion for linear solver", tolerance);
    cmd.addInt   ("",  "Solver.MaxIterations",  "Stopping criterion for linear solver", maxIterations);
    cmd.addString("b", "BoundaryConditions",    "Boundary conditions", boundaryConditions);
    cmd.addSwitch(     "sol",                   "Write the computed solution to console", sol);
    cmd.addSwitch(     "plot",                  "Plot the result with Paraview", plot);

    try { cmd.getValues(argc,argv); } catch (int rv) { return rv; }

    gsOptionList opt = cmd.getOptionList();

    if ( ! gsFileManager::fileExists(geometry) )
    {
        gsInfo << "Geometry file could not be found.\n";
        gsInfo << "I was searching in the current directory and in: " << gsFileManager::getSearchPaths() << "\n";
        return EXIT_FAILURE;
    }

    gsInfo << "Run ieti_example with options:\n" << opt << std::endl;

    /******************* Define geometry ********************/

    gsInfo << "Define geometry... " << std::flush;

    gsMultiPatch<>::uPtr mpPtr = gsReadFile<>(geometry);
    if (!mpPtr)
    {
        gsInfo << "No geometry found in file " << geometry << ".\n";
        return EXIT_FAILURE;
    }
    gsMultiPatch<>& mp = *mpPtr;

    for (index_t i=0; i<splitPatches; ++i)
    {
        gsInfo << "split patches uniformly... " << std::flush;
        mp = mp.uniformSplit();
    }

    if (stretchGeometry!=1)
    {
       gsInfo << "and stretch it... " << std::flush;
       for (size_t i=0; i!=mp.nPatches(); ++i)
           const_cast<gsGeometry<>&>(mp[i]).scale(stretchGeometry,0);
       // Const cast is allowed since the object itself is not const. Stretching the
       // overall domain keeps its topology.
    }

    gsInfo << "done.\n";

    /************** Define boundary conditions **************/

    gsInfo << "Define boundary conditions... " << std::flush;

    gsConstantFunction<> one(1.0, mp.geoDim());

    gsBoundaryConditions<> bc;
    {
        const index_t len = boundaryConditions.length();
        index_t i = 0;
        for (gsMultiPatch<>::const_biterator it = mp.bBegin(); it < mp.bEnd(); ++it)
        {
            char b_local;
            if ( len == 1 )
                b_local = boundaryConditions[0];
            else if ( i < len )
                b_local = boundaryConditions[i];
            else
            {
                gsInfo << "\nNot enough boundary conditions given.\n";
                return EXIT_FAILURE;
            }

            if ( b_local == 'd' )
                bc.addCondition( *it, condition_type::dirichlet, &one );
            else if ( b_local == 'n' )
                bc.addCondition( *it, condition_type::neumann, &one );
            else
            {
                gsInfo << "\nInvalid boundary condition given; only 'd' (Dirichlet) and 'n' (Neumann) are supported.\n";
                return EXIT_FAILURE;
            }

            ++i;
        }
        if ( len > i )
            gsInfo << "\nToo much boundary conditions have been specified. Ingnoring the remaining ones.\n";
        gsInfo << "done. "<<i<<" boundary conditions set.\n";
    }


    /************ Setup bases and adjust degree *************/

    gsMultiBasis<> mb(mp);

    gsInfo << "Setup bases and adjust degree... " << std::flush;

    for ( size_t i = 0; i < mb.nBases(); ++ i )
        mb[i].setDegreePreservingMultiplicity(degree);

    for ( index_t i = 0; i < refinements; ++i )
        mb.uniformRefine();

    gsInfo << "done.\n";

    /********* Setup assembler and assemble matrix **********/

    gsInfo << "Setup assembler and assemble matrix... " << std::flush;

    gsDofMapper dm;
    mb.getMapper(
       dirichlet::elimination,
       iFace::glue,
       bc,
       dm,
       0
    );

    const size_t nr_patches = mp.nPatches();

    gsIetiSystem<> ieti;
    ieti.localMatrixOps.reserve(nr_patches+1);
    ieti.localRhs.reserve(nr_patches+1);

    std::vector<gsDofMapper> dm_local(nr_patches);

    {
        //gsSparseMatrix<> primalMatrix;
        //gsMatrix<> primalRhs;
    
        for (size_t i=0; i<nr_patches; ++i)
        {
            gsBoundaryConditions<> bc_local;
            bc.getConditionsForPatch(i,bc_local);
            gsMultiBasis<> mb_local = mb[i];

            mb_local.getMapper(
               dirichlet::elimination,
               iFace::glue,
               bc_local,
               dm_local[i],
               0
            );

            gsPoissonAssembler<> assembler(
                mp[i],
                mb_local,
                bc_local,
                gsConstantFunction<>(1,mp.geoDim()),
                dirichlet::elimination,
                iFace::glue // Does not matter
            );
            assembler.assemble();

            ieti.localMatrixOps.push_back(makeMatrixOp(gsSparseMatrix<>(assembler.matrix()).moveToPtr()));
            ieti.localRhs.push_back(give(assembler.rhs()));
          
        }
      
        //ieti.localMatrixOps.push_back(makeMatrixOp(primalMatrix.moveToPtr()));
        //ieti.localRhs.push_back(give(primalRhs));
    }

    gsIetiMapper<> ietiMapper(give(dm),give(dm_local));
    ieti.jumpMatrices = ietiMapper.jumpMatrices();


    gsScaledDirichletPrec<> prec;
    prec.jumpMatrices.reserve(nr_patches);
    for (size_t i=0; i<nr_patches; ++i)
    {
        prec.localMatrixOps.push_back( ieti.localMatrixOps[i] );
        prec.jumpMatrices.push_back( ieti.jumpMatrices[i] );
        prec.localSchurOps.push_back( ieti.localMatrixOps[i] );//TODO: wrong
    }

    //ieti.jumpMatrices.push_back( ietiAssembler.primalJumpMatrix() );

    gsInfo << "done.\n";

    /**************** Setup solver and solve ****************/

    gsInfo << "Setup solver and solve... \n"
        "    Setup LU solvers... " << std::flush;

    ieti.setupSparseLUSolvers();
    gsInfo << "done.\n    Setup multiplicity scaling... " << std::flush;

    prec.setupMultiplicityScaling();

    gsMatrix<> lambda, errorHistory;
    lambda.setRandom( ieti.numberOfLagrangeMultipliers(), 1 );

    gsInfo << "done.\n    Setup rhs... " << std::flush;
    gsMatrix<> rhsForSchur = ieti.rhsForSchurComplement();
    gsInfo << "done.\n    Setup cg solver for Lagrange multipliers and solve... " << std::flush;

    gsConjugateGradient<>( ieti.schurComplement(), prec.secaledDirichletPreconditioner() )
        .setOptions( opt.getGroup("Solver") )
        .solveDetailed( rhsForSchur, lambda, errorHistory );

    gsInfo << "done.\n    Reconstruct solution from Lagrange multipliers... " << std::flush;
    gsMatrix<> x = ietiMapper.constructGlobalSolutionFromLocalSolutions(
          ieti.constructSolutionFromLagrangeMultipliers(lambda) );
    gsInfo << "done.\n\n";

    /******************** Print end Exit ********************/

    const index_t iter = errorHistory.rows()-1;
    const bool success = errorHistory(iter,0) < tolerance;
    if (success)
        gsInfo << "Reached desired tolerance after " << iter << " iterations:\n";
    else
        gsInfo << "Did not reach desired tolerance after " << iter << " iterations:\n";

    if (errorHistory.rows() < 20)
        gsInfo << errorHistory.transpose() << "\n\n";
    else
        gsInfo << errorHistory.topRows(5).transpose() << " ... " << errorHistory.bottomRows(5).transpose()  << "\n\n";

    if (sol)
        gsInfo << x.transpose() << "\n\n";

    if (plot)
    {
        // Construct the solution as a scalar field
        // For this purpose, we use a global assembler
        gsInfo << "Setting up global assembler and assembler..." << std::flush;
        gsPoissonAssembler<> globalAssembler(
                mp,
                mb,
                bc,
                gsConstantFunction<>(1,mp.geoDim()),
                dirichlet::elimination,
                iFace::glue
            );
        globalAssembler.assemble();
        gsInfo << "done.\nPlotting in Paraview.\n\n" << std::flush;
        gsMultiPatch<> mpsol;
        globalAssembler.constructSolution(x, mpsol);
        gsField<> sol( globalAssembler.patches(), mpsol );
        // Write approximate and exact solution to paraview files
        gsWriteParaview<>(sol, "ieti_result", 1000);
        gsFileManager::open("ieti_result.pvd");
    }
    else
    {
        gsInfo << "Done. No output created, re-run with --plot to get a ParaView "
                  "file containing the solution.\n";
    }
    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
