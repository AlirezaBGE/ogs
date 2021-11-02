# Build dependencies via ExternalProject_Add() at configure time in
# ${PROJECT_BINARY_DIR}/_ext
include(BuildExternalProject)

if(OGS_USE_MFRONT)
    find_program(MFRONT mfront)
    if(NOT MFRONT)
        BuildExternalProject(
            TFEL GIT_REPOSITORY https://github.com/thelfer/tfel.git
            GIT_TAG rliv-${ogs.minimum_version.tfel-rliv}
            CMAKE_ARGS
                -DCMAKE_INSTALL_RPATH=<INSTALL_DIR>/lib
                -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE
        )
        set(TFELHOME ${PROJECT_BINARY_DIR}/_ext/TFEL CACHE PATH "")
    endif()
    list(APPEND CMAKE_INSTALL_RPATH ${TFELHOME}/lib)
endif()

if(OGS_USE_PETSC)
    # Force CMake to accept a given PETSc configuration in case the failure of
    # MPI tests. This may cause the compilation broken.
    option(FORCE_PETSC_EXECUTABLE_RUNS
           "Force CMake to accept a given PETSc configuration" ON
    )
    if(FORCE_PETSC_EXECUTABLE_RUNS)
        set(PETSC_EXECUTABLE_RUNS YES)
    endif()

    if(NOT OGS_PETSC_CONFIG_OPTIONS)
        find_package(PETSc ${ogs.minimum_version.petsc})
    endif()
    if(NOT PETSC_FOUND)
        set(_configure_opts "")
        if(NOT "--download-fc=1" IN_LIST OGS_PETSC_CONFIG_OPTIONS)
            list(APPEND _configure_opts --with-fc=0)
        endif()
        if(ENV{CC})
            list(APPEND _configure_opts --with-cc=$ENV{CC})
        endif()
        if(ENV{CXX})
            list(APPEND _configure_opts --with-cxx=$ENV{CXX})
        endif()
        BuildExternalProject(
            PETSc
            LOG_OUTPUT_ON_FAILURE ON
            GIT_REPOSITORY https://gitlab.com/petsc/petsc.git
            GIT_TAG v${ogs.minimum_version.petsc}
            CONFIGURE_COMMAND
                ./configure --download-f2cblaslapack=1 --prefix=<INSTALL_DIR>
                --with-debugging=$<CONFIG:Debug> ${_configure_opts}
                ${OGS_PETSC_CONFIG_OPTIONS}
            BUILD_IN_SOURCE ON
            BUILD_COMMAND make -j all
            INSTALL_COMMAND make -j install
        )
        set(PETSC_DIR ${PROJECT_BINARY_DIR}/_ext/PETSc)
        find_package(PETSc ${ogs.minimum_version.petsc} REQUIRED)
    endif()

    add_library(petsc SHARED IMPORTED)
    target_include_directories(petsc INTERFACE ${PETSC_INCLUDES})
    set_target_properties(petsc PROPERTIES IMPORTED_LOCATION ${PETSC_LIBRARIES})
    target_compile_definitions(petsc INTERFACE USE_PETSC)
endif()

if(OGS_USE_LIS)
    find_package(LIS)
    if(NOT LIS_FOUND)
        BuildExternalProject(
            LIS
            GIT_REPOSITORY https://github.com/anishida/lis.git
            GIT_TAG ${ogs.minimum_version.lis}
            CONFIGURE_COMMAND ./configure --enable-omp --prefix=<INSTALL_DIR>
            BUILD_IN_SOURCE ON
            BUILD_COMMAND make -j
            INSTALL_COMMAND make -j install
        )
        set(ENV{LIS_ROOT_DIR} ${PROJECT_BINARY_DIR}/_ext/LIS)
        find_package(LIS REQUIRED)
    endif()
endif()