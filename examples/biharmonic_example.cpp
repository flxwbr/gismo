/** @file biharmonic_example.cpp

    @brief A Biharmonic example with G1 Basis.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): P. Weinmueller
*/

# include <gismo.h>
# include <gsAssembler/gsBiharmonicAssembler.h>

# include <gsG1Basis/gsG1Norm.h>


using namespace gismo;

int main(int argc, char *argv[])
{
    index_t numRefine = 0;
    index_t numDegree = 3;

    index_t geometry = 0;

    bool plot = false;
    bool neumann = false;

    gsCmdLine cmd("Example for solving the biharmonic problem.");
    cmd.addInt("r", "refine", "Number of refinement steps", numRefine);
    cmd.addInt("p", "degree", "Polynomial degree", numDegree);
    cmd.addInt("g", "geometry", "Geometry type", geometry);
    cmd.addSwitch( "neumann", "Compute the biharmonic problem with neumann bdy", neumann );
    cmd.addSwitch( "plot", "Plot result in ParaView format", plot );
    try { cmd.getValues(argc,argv); } catch (int rv) { return rv; }

    dirichlet::strategy dirStrategy = dirichlet::elimination;
    iFace::strategy intStrategy = iFace::none;

    gsFunctionExpr<> source  ("256*pi*pi*pi*pi*(4*cos(4*pi*x)*cos(4*pi*y) - cos(4*pi*x) - cos(4*pi*y))",2);
    gsFunctionExpr<> laplace ("-16*pi*pi*(2*cos(4*pi*x)*cos(4*pi*y) - cos(4*pi*x) - cos(4*pi*y))",2);
    gsFunctionExpr<> solVal("(cos(4*pi*x) - 1) * (cos(4*pi*y) - 1)",2);
    gsFunctionExpr<> sol1der ("-4*pi*(cos(4*pi*y) - 1)*sin(4*pi*x)",
                              "-4*pi*(cos(4*pi*x) - 1)*sin(4*pi*y)",2);
    gsFunctionExpr<> sol2der ("-16*pi^2*(cos(4*pi*y) - 1)*cos(4*pi*x)",
                              "-16*pi^2*(cos(4*pi*x) - 1)*cos(4*pi*y)",
                              " 16*pi^2*sin(4*pi*x)*sin(4*pi*y)", 2);
/*
    gsFunctionExpr<> source  ("0",2);
    gsFunctionExpr<> laplace ("0",2);
    gsFunctionExpr<> solVal("1",2);
    gsFunctionExpr<> sol1der ("0",
                              "0",2);
    gsFunctionExpr<> sol2der ("0",
                              "0",
                              "0", 2);
    */
    gsFunctionWithDerivatives<real_t> solution(solVal, sol1der, sol2der);


    //! [Geometry input]
    index_t initDegree = 0;
    std::string string_geo;
    switch(geometry)
    {
        case 0: // 1 Patch
            string_geo = "planar/one_square.xml";
            initDegree = 1;
            break;
        case 1: // 2 Patch
            string_geo = "planar/two_squares.xml";
            initDegree = 1;
            break;
        case 2: // 2 Patch
            string_geo = "planar/two_squares_curved.xml";
            initDegree = 3;
            break;
        default:
            gsInfo << "No geometry is used! \n";
            break;
    }

    gsFileData<> fd(string_geo);
    gsInfo << "Loaded file " << fd.lastPath() <<"\n";

    gsMultiPatch<> geo;
    fd.getId(0, geo); // id=0: Multipatch domain
    geo.computeTopology();

    gsMultiBasis<> basis(geo);

    for (int i = 0; i < numDegree-initDegree; ++i) // Assume that we have the same degree for all patches
        basis.degreeElevate();

    //! [Setting up the boundary condition]
    gsBoundaryConditions<> bcInfo, bcInfo2;
    for (gsMultiPatch<>::const_biterator bit = geo.bBegin(); bit != geo.bEnd(); ++bit)
    {
        bcInfo.addCondition(*bit, condition_type::dirichlet, &solVal);
        if (!neumann)
            bcInfo2.addCondition(*bit, condition_type::laplace, &laplace);
        else
            bcInfo2.addCondition(*bit,  condition_type::neumann, &sol1der);
    }

    gsStopwatch clock;

    //! [Solver loop]
    gsVector<> l2err(numRefine+1), h1err(numRefine+1), h2err(numRefine+1), meshSize(numRefine+1);
    gsInfo<< "(dot1=assembled, dot2=solved, dot3=got_error)\n"
             "\nDoFs: ";
    for (int r=0; r<=numRefine; ++r)
    {
        basis.uniformRefine(1,basis.maxCwiseDegree()-1); // TODO for now: r=p-1
        meshSize[r] = basis.basis(0).getMinCellLength();

        bool test_g1 = false;
        if (geo.nPatches() == 2 && test_g1)
        {
            // Basis Test
            index_t numPoints = 5;
            std::vector<gsMatrix<real_t>>result_L, result_R;
            gsMatrix<> points_L(2, numPoints), points_R(2, numPoints);
            points_L.setOnes();
            points_R.setZero();

            gsVector<> vec;
            vec.setLinSpaced(numPoints, 0.01, 0.99);
            points_L.row(1) = vec.transpose().setZero(); // v
            points_R.row(1) = vec.transpose().setZero(); // v

            gsG1MultiBasis<real_t> g1MultiBasis(geo, basis); // Maybe earlier? Only need for #patches>2
            g1MultiBasis.eval_deriv_deriv2_into(points_L, result_L, 0);
            g1MultiBasis.eval_deriv_deriv2_into(points_R, result_R, 1);

            gsInfo << "Result Deriv 1 L : " << result_L[1] << "\n";
            gsInfo << "Result Deriv 1 R : " << result_R[1] << "\n";

            gsInfo << "Result L - R" << result_L[0] - result_R[0] << "\n";
            //gsInfo << "Result Deriv L - Deriv R" << result_L[1] - result_R[1] << "\n";
            // END


            gsMatrix<> alpha_L, alpha_R, beta_L, beta_R, beta;
            g1MultiBasis.eval_alpha_S_into(points_L.row(1), alpha_L, 0);
            g1MultiBasis.eval_alpha_S_into(points_R.row(1), alpha_R, 1);

            g1MultiBasis.eval_beta_S_into(points_L.row(1), beta_L, 0);
            g1MultiBasis.eval_beta_S_into(points_R.row(1), beta_R, 1);

            g1MultiBasis.eval_beta_into(points_L.row(1), beta);

            gsInfo << "alpha_L: " << alpha_L << "\n";
            gsInfo << "alpha_R: " << alpha_R << "\n";
            gsInfo << "beta_L " << beta_L << "\n";
            gsInfo << "beta_R: " << beta_R << "\n";
            gsInfo << "Beta: " << beta << "\n";

            gsInfo << "Gluing data condition: " << alpha_L.cwiseProduct(beta_R) + alpha_R.cwiseProduct(beta_L) - beta << "\n";
            gsInfo << "G1 condition: \n";
            for (index_t row_i = 0; row_i < result_L[1].rows(); row_i+=2)
                gsInfo << result_L[1].row(row_i) *  alpha_R.transpose() - result_R[1].row(row_i) * alpha_L.transpose() << " ";
            gsInfo << "\n";
        }


        //Initilize solver
        gsBiharmonicAssembler<real_t> BiharmonicAssembler(geo, basis, bcInfo, bcInfo2, source,
                                                          dirStrategy, intStrategy);

        gsInfo << BiharmonicAssembler.numDofs() <<std::flush;

        clock.restart();
        BiharmonicAssembler.assemble();
        gsInfo << " " << clock.stop() << " ";

        gsInfo<< "." <<std::flush;// Assemblying done

        clock.restart();
        gsSparseSolver<real_t>::CGDiagonal solver;
        solver.analyzePattern(BiharmonicAssembler.matrix());
        solver.factorize(BiharmonicAssembler.matrix());
        gsMatrix<> solVector = solver.solve(BiharmonicAssembler.rhs());
        gsInfo << " " << clock.stop() << " ";

        //gsInfo << "rhs: " << BiharmonicAssembler.matrix().toDense() << "\n";
        //gsInfo << "rhs: " << BiharmonicAssembler.rhs() << "\n";
        //gsInfo << "Vector: " << solVector << "\n";

        gsInfo<< "." <<std::flush; // Linear solving done


        //Reconstruct solution
        gsMultiPatch<> mpsol;
        BiharmonicAssembler.constructSolution(solVector, mpsol);
        gsField<> solField(BiharmonicAssembler.patches(), mpsol);

        // Construct G1 Solution
        gsMatrix<real_t> g1Sol;
        BiharmonicAssembler.constructG1Solution(solVector, g1Sol);

        //Contruct the H2 norm, part by part.
        real_t errorH2Semi, errorH1Semi;

#pragma omp parallel for
        for (index_t e = 0; e < 3; ++e)
        {
            /*
            if (e == 0)
                errorH2Semi = solField.distanceH2(solution, false);
            else if (e == 1)
                errorH1Semi = solField.distanceH1(solution, false);
            else if (e == 2)
                l2err[r] = solField.distanceL2(solution, false);
            */
            if (e == 2)
            {
                gsG1Norm<real_t> g1Norm(geo, basis, mpsol, g1Sol, solution);
                g1Norm.compute();
                //gsInfo << "g1Norm: " << g1Norm.valueL2() << " : " << g1Norm.valueH1() << " : " << g1Norm.valueH2()
                //       << "\n";

                l2err[r] = g1Norm.valueL2();
                errorH1Semi = g1Norm.valueH1();
                errorH2Semi = g1Norm.valueH2();
            }
        }

        h1err[r] = math::sqrt(errorH1Semi * errorH1Semi + l2err[r] * l2err[r]);
        h2err[r] = math::sqrt(errorH2Semi * errorH2Semi + errorH1Semi * errorH1Semi + l2err[r] * l2err[r]);

        gsInfo<< ". " <<std::flush; // Error computations done

        // Plot solution in paraview
        if (plot && r == numRefine)
        {
            // Write approximate and exact solution to paraview files
            gsInfo<<"Plotting in ParaView...\n";
            //gsWriteParaview<>(solField, "Biharmonic2d", 5000);
            const gsField<> exact( geo, solVal, false );
            gsWriteParaview<>( exact, "Biharmonic2d_exact", 5000);

            BiharmonicAssembler.plotParaview(solField, solVector); // G1Biharmonic.pvd
        }
        else if (r == numRefine)
            gsInfo << "Done. No output created, re-run with --plot to get a ParaView "
                      "file containing the solution.\n";
    }

    gsInfo << "Mesh size: " << meshSize << "\n";

    //! [Error and convergence rates]
    gsInfo<< "\n\nL2 error: "<<std::scientific<<std::setprecision(3)<<l2err.transpose()<<"\n";
    gsInfo<< "H1 error: "<<std::scientific<<h1err.transpose()<<"\n";
    gsInfo<< "H2 error: "<<std::scientific<<h2err.transpose()<<"\n";

    if (numRefine>0)
    {
        gsInfo<< "\nEoC (L2): " << std::fixed<<std::setprecision(2)
              << ( l2err.head(numRefine).array() /
                   l2err.tail(numRefine).array() ).log().transpose() / std::log(2.0) <<"\n";

        gsInfo<<   "EoC (H1): "<< std::fixed<<std::setprecision(2)
              <<( h1err.head(numRefine).array() /
                  h1err.tail(numRefine).array() ).log().transpose() / std::log(2.0) <<"\n";

        gsInfo<<   "EoC (H2): "<< std::fixed<<std::setprecision(2)
              <<( h2err.head(numRefine).array() /
                  h2err.tail(numRefine).array() ).log().transpose() / std::log(2.0) <<"\n";
    }
    //! [Error and convergence rates]


    return  0;
}
