/**
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#pragma once

/// The LinearSolverLibrarySetup takes care of proper initialization and
/// shutting down of an external linear solver library. The concrete
/// implementation is chosen by the build system.
/// An object of this class must be created at the beginning of the scope where
/// it is used. When the scope closes (or the object is destroyed explicitly)
/// library shutting down functions are automatically called.
/// The default implementation is empty providing polymorphic behaviour when
/// using this class.

#include "NumLib/DOF/GlobalMatrixProviders.h"

#if defined(USE_PETSC)
#include <petsc.h>
#include <mpi.h>
namespace ApplicationsLib
{
struct LinearSolverLibrarySetup final
{
    LinearSolverLibrarySetup(int argc, char* argv[])
    {
        MPI_Init(&argc, &argv);
        char help[] = "ogs6 with PETSc \n";
        PetscInitialize(&argc, &argv, nullptr, help);
        MPI_Comm_set_errhandler(PETSC_COMM_WORLD, MPI_ERRORS_RETURN);
    }

    ~LinearSolverLibrarySetup()
    {
        NumLib::cleanupGlobalMatrixProviders();
        PetscFinalize();
        MPI_Finalize();
    }
};
}    // ApplicationsLib
#elif defined(USE_LIS)
#include <lis.h>
namespace ApplicationsLib
{
struct LinearSolverLibrarySetup final
{
    LinearSolverLibrarySetup(int argc, char* argv[])
    {
        lis_initialize(&argc, &argv);
    }

    ~LinearSolverLibrarySetup()
    {
        NumLib::cleanupGlobalMatrixProviders();
        lis_finalize();
    }
};
}    // ApplicationsLib
#else
namespace ApplicationsLib
{
struct LinearSolverLibrarySetup final
{
    LinearSolverLibrarySetup(int /*argc*/, char* /*argv*/[]) {}
    ~LinearSolverLibrarySetup()
    {
        NumLib::cleanupGlobalMatrixProviders();
    }
};
}  // namespace ApplicationsLib
#endif
