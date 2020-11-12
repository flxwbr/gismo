/** @file poisson_example.cpp

    @brief Tutorial on how to use G+Smo to solve the Poisson equation,
    see the \ref PoissonTutorial

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): J. Sogn
*/

//! [Include namespace]
# include <gismo.h>

using namespace gismo;
//! [Include namespace]

int main(int argc, char *argv[])
{
    //! [Parse command line]
    bool plot = false;

    gsCmdLine cmd("Tutorial on solving a Poisson problem.");
    cmd.addSwitch("plot", "Create a ParaView visualization file with the solution", plot);
    try { cmd.getValues(argc,argv); } catch (int rv) { return rv; }
    //! [Parse command line]

    //! [Function data]
    // Define source function
    gsFunctionExpr<> laplace ("16*pi*pi*(2*cos(4*pi*x)*cos(4*pi*y) - cos(4*pi*x) - cos(4*pi*y))",2);
    gsFunctionExpr<> solVal("(cos(4*pi*x) - 1) * (cos(4*pi*y) - 1)",2);
    gsFunctionExpr<>sol1der ("-4*pi*(cos(4*pi*y) - 1)*sin(4*pi*x)",
                             "-4*pi*(cos(4*pi*x) - 1)*sin(4*pi*y)",2);

    // Print out source function and solution
    gsInfo<<"Source function "<< laplace << "\n";
    gsInfo<<"Exact solution "<< solVal <<"\n\n";
    //! [Function data]

    //! [Geometry data]
    // Define Geometry, must be a gsMultiPatch object
    gsMultiPatch<> patches;

    gsFileData<> fd("planar/two_squares.xml");
    gsInfo << "Loaded file "<< fd.lastPath() <<"\n";
    fd.getId(0, patches); // id=0: Multipatch domain
    patches.computeTopology();
    gsInfo << "The domain is a "<< patches <<"\n";
    //! [Geometry data]

    // For single patch unit square of quadratic elements use (Note:
    // you need to update the bounadry conditions section for this to
    // work properly!) :
    // patches = gsMultiPatch<>(*gsNurbsCreator<>::BSplineSquare(2));

    // Geometry can also be read from file (if gsMultiPatch):
    // gsReadFile<>("planar/lshape_p2.xml", patches);

    // Define Boundary conditions. Note that if one boundary is
    // "free", eg. if no condition is defined, then it is a natural
    // boundary (zero Neumann condition)
    // Also, remember that a pure Neumann problem has no unique
    // solution, thereforer implies a singular matrix. In this case
    // a corner DoF can be fixed to a given value to obtain a unique solution.
    // (example: bcInfo.addCornerValue(boundary::southwest, value, patch);)

    //! [Boundary conditions]
    gsBoundaryConditions<> bcInfo;
    for (gsMultiPatch<>::const_biterator
             bit = patches.bBegin(); bit != patches.bEnd(); ++bit)
    {
        bcInfo.addCondition( *bit, condition_type::dirichlet, &solVal );
    }


    //! [Refinement]
    // Copy basis from the geometry
    gsMultiBasis<> refine_bases( patches );

    // Number for h-refinement of the computational (trial/test) basis.
    const int numRefine  = 6;

    // Number for p-refinement of the computational (trial/test) basis.
    const int degree     = 2;

    // h-refine each basis (4, one for each patch)
    for ( int i = 0; i < numRefine; ++i)
      refine_bases.uniformRefine();

    // k-refinement (set degree)
    for ( size_t i = 0; i < refine_bases.nBases(); ++ i )
        refine_bases[i].setDegreePreservingMultiplicity(degree);

    //! [Refinement]

    ////////////// Setup solver and solve //////////////
    // Initialize Solver
    // Setup method for handling Dirichlet boundaries, options:
    //
    // * elimination: Eliminate the Dirichlet DoFs from the linear system.
    //
    // * nitsche: Keep the Dirichlet DoFs and enforce the boundary
    //
    // condition weakly by a penalty term.
    // Setup method for handling patch interfaces, options:
    //
    // * glue:Glue patches together by merging DoFs across an interface into one.
    //   This only works for conforming interfaces
    //
    // * dg: Use discontinuous Galerkin-like coupling between adjacent patches.
    //       (This option might not be available yet)
    //! [Assemble]
    gsPoissonAssembler<real_t> assembler(patches,refine_bases,bcInfo,laplace,
                                       //dirichlet::elimination, iFace::glue);
                                         dirichlet::nitsche    , iFace::none);

    // Generate system matrix and load vector
    gsInfo<< "Assembling...\n";
    assembler.assemble();
    gsInfo << "Have assembled a system (matrix and load vector) with "
           << assembler.numDofs() << " dofs.\n";
    //! [Assemble]

    //! [Solve]
    // Initialize the conjugate gradient solver
    gsInfo << "Solving...\n";
    gsSparseSolver<>::CGDiagonal solver( assembler.matrix() );
    gsMatrix<> solVector = solver.solve( assembler.rhs() );
    gsInfo << "Solved the system with CG solver.\n";
    //! [Solve]

    //! [Construct solution]
    // Construct the solution as a scalar field
    gsMultiPatch<> mpsol;
    assembler.constructSolution(solVector, mpsol);
    gsField<> sol( assembler.patches(), mpsol);
    //! [Construct solution]

    if (plot)
    {
        //! [Plot in Paraview]
        // Write approximate and exact solution to paraview files
        gsInfo<<"Plotting in Paraview...\n";
        gsWriteParaview<>(sol, "poisson2d", 1000);
        const gsField<> exact( assembler.patches(), solVal, false );
        gsWriteParaview<>( exact, "poisson2d_exact", 1000);
        //! [Plot in Paraview]
    }
    else
    {
        gsInfo << "Done. No output created, re-run with --plot to get a ParaView "
                  "file containing the solution.\n";
    }
    return EXIT_SUCCESS;

}// end main
