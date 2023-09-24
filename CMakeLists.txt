#-------------------------------------------------------------------------------
# SuiteSparse/CMakeLists.txt:  root CMake build rules
#-------------------------------------------------------------------------------

# Copyright (c) 2023, Timothy A. Davis.
# All Rights Reserved.

# The actually required minimum CMake version might be higher depending on the
# selected SuiteSparse projects.
cmake_minimum_required ( VERSION 3.20 )

project ( "SuiteSparse" )

#-------------------------------------------------------------------------------
# SuiteSparse policies
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/SuiteSparse_config/cmake_modules )

option ( ENABLE_CUDA "Enable CUDA acceleration" ON )

# SuiteSparsePolicy enables SUITESPARSE_CUDA if a CUDA compiler can be found.
include ( SuiteSparsePolicy )

#-------------------------------------------------------------------------------
# build options
#-------------------------------------------------------------------------------

# lower-case list of all projects that can be built by this root CMake file
set ( SUITESPARSE_ALL_PROJECTS
    "suitesparse_config;mongoose;amd;btf;camd;ccolamd;colamd;cholmod;cxsparse;ldl;klu;umfpack;rbio;spqr;graphblas;spex" )

# lower-case list of extra projects that can be built by this root CMake file
set ( SUITESPARSE_EXTRA_PROJECTS
    "suitesparse_gpuruntime;gpuqrengine;csparse" )

# lower-case list of known projects that can be built by this root CMake file
set ( SUITESPARSE_KNOWN_PROJECTS "${SUITESPARSE_ALL_PROJECTS};${SUITESPARSE_EXTRA_PROJECTS}" )

set ( SUITESPARSE_ENABLE_PROJECTS "all" CACHE STRING
    "Semicolon-separated list of SuiteSparse projects to be built (${SUITESPARSE_KNOWN_PROJECTS}, or \"all\")" )

if ( SUITESPARSE_CUDA )
    set ( SUITESPARSE_ALL_PROJECTS
        "${SUITESPARSE_ALL_PROJECTS};suitesparse_gpuruntime;gpuqrengine" )
endif ( )

# expand "all" early on
if ( "all" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    set ( SUITESPARSE_ENABLE_PROJECTS "${SUITESPARSE_ENABLE_PROJECTS};${SUITESPARSE_ALL_PROJECTS}" )
    list ( REMOVE_ITEM SUITESPARSE_ENABLE_PROJECTS "all" )
    list ( REMOVE_DUPLICATES SUITESPARSE_ENABLE_PROJECTS )
endif ( )

# check for unknown projects in list
foreach ( proj ${SUITESPARSE_ENABLE_PROJECTS} )
  if ( NOT "${proj}" IN_LIST SUITESPARSE_KNOWN_PROJECTS )
     message ( FATAL_ERROR "${proj} is not a known project: ${SUITESPARSE_KNOWN_PROJECTS}." )
  endif ( )
endforeach ( )

# CHOLMOD options affecting dependencies
option ( NCAMD "ON: do not use CAMD/CCOLAMD.  OFF (default): use CAMD/CCOLAMD" OFF )

# KLU and UMFPACK options affecting dependencies
option ( NCHOLMOD "ON: do not use CHOLMOD.  OFF (default): use CHOLMOD" OFF )

# overwrite NSTATIC specifically for GraphBLAS because building the library
# takes a long time
option ( GRAPHBLAS_NSTATIC "ON (default): set NSTATIC for GraphBLAS project.  OFF: Use same value of NSTATIC for GraphBLAS like in the other projects" ON )

# options to build with libraries installed on the system instead of building
# dependencies automatically
option ( USE_SYSTEM_BTF "ON: use BTF libraries installed on the build system.  OFF (default): Automatically build BTF as dependency if needed." OFF )
option ( USE_SYSTEM_CHOLMOD "ON: use CHOLMOD libraries installed on the build system.  OFF (default): Automatically build CHOLMOD as dependency if needed." OFF )
option ( USE_SYSTEM_AMD "ON: use AMD libraries installed on the build system.  OFF (default): Automatically build AMD as dependency if needed." OFF )
option ( USE_SYSTEM_COLAMD "ON: use COLAMD libraries installed on the build system.  OFF (default): Automatically build COLAMD as dependency if needed." OFF )
option ( USE_SYSTEM_CAMD "ON: use CAMD libraries installed on the build system.  OFF (default): Automatically build CAMD as dependency if needed." OFF )
option ( USE_SYSTEM_CCOLAMD "ON: use CCOLAMD libraries installed on the build system.  OFF (default): Automatically build CCOLAMD as dependency if needed." OFF )
option ( USE_SYSTEM_GPUQRENGINE "ON: use GPUQREngine libraries installed on the build system.  OFF (default): Automatically build GPUQREngine as dependency if needed." OFF )
option ( USE_SYSTEM_SUITESPARSE_GPURUNTIME "ON: use SuiteSparse GPURuntime libraries installed on the build system.  OFF (default): Automatically build SuiteSparse GPURuntime as dependency if needed." OFF )
option ( USE_SYSTEM_SUITESPARSE_CONFIG "ON: use SuiteSparse Config libraries installed on the build system.  OFF (default): Automatically build SuiteSparse Config as dependency if needed." OFF )

#-------------------------------------------------------------------------------
# global variables
#-------------------------------------------------------------------------------

# Set to indicate that we are building from a root CMake file.
# That will change the directory layout and (imported) target names (namespace!)
# during the build process.
set ( SUITESPARSE_ROOT_CMAKELISTS ON )

#-------------------------------------------------------------------------------
# common SuiteSparse modules
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/SuiteSparse_config/cmake_modules )

include ( SuiteSparsePolicy )

#-------------------------------------------------------------------------------
# check/add project dependencies
#-------------------------------------------------------------------------------

if ( "klu" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_BTF AND NOT "btf" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"btf\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "btf" )
    endif ( )
endif ( )

if ( "klu" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "umfpack" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "spqr" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_CHOLMOD AND NOT NCHOLMOD AND NOT "cholmod" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"cholmod\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "cholmod" )
    endif ( )
endif ( )

if ( "cholmod" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "ldl" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "umfpack" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "spex" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_AMD AND NOT "amd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"amd\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "amd" )
    endif ( )
endif ( )

if ( "cholmod" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "spex" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_COLAMD AND NOT "colamd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"colamd\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "colamd" )
    endif ( )
endif ( )

if ( NOT NCAMD AND "cholmod" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_CAMD AND NOT "camd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"camd\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "camd" )
    endif ( )
    if ( NOT USE_SYSTEM_CCOLAMD AND NOT "ccolamd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"ccolamd\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "ccolamd" )
    endif ( )
endif ( )

if ( SUITESPARSE_CUDA AND "spqr" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_GPUQRENGINE AND NOT "gpuqrengine" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"gpuqrengine\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "gpuqrengine" )
    endif ( )
endif ( )

if ( SUITESPARSE_CUDA AND "gpuqrengine" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_SUITESPARSE_GPURUNTIME AND NOT "suitesparse_gpuruntime" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"suitesparse_gpuruntime\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "suitesparse_gpuruntime" )
    endif ( )
endif ( )

if ( "mongoose" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "amd" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "btf" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "camd" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "ccolamd" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "colamd" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "cholmod" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "cxsparse" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "ldl" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "klu" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "umfpack" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "rbio" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "suitesparse_gpuruntime" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "gpuqrengine" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "spqr" IN_LIST SUITESPARSE_ENABLE_PROJECTS
        OR "spex" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    if ( NOT USE_SYSTEM_SUITESPARSE_CONFIG AND NOT "suitesparse_config" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
        message ( STATUS "Adding \"suitesparse_config\" to the list of built targets." )
        list ( APPEND SUITESPARSE_ENABLE_PROJECTS "suitesparse_config" )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# include selected projects
#-------------------------------------------------------------------------------

if ( "suitesparse_config" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "SuiteSparse_config" )
endif ( )

if ( "mongoose" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "Mongoose" )
endif ( )

if ( "amd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "AMD" )
endif ( )

if ( "btf" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "BTF" )
endif ( )

if ( "camd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "CAMD" )
endif ( )

if ( "ccolamd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "CCOLAMD" )
endif ( )

if ( "colamd" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "COLAMD" )
endif ( )

if ( "cholmod" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "CHOLMOD" )
endif ( )

if ( "cxsparse" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "CXSparse" )
endif ( )

if ( "ldl" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "LDL" )
endif ( )

if ( "klu" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "KLU" )
endif ( )

if ( "umfpack" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "UMFPACK" )
endif ( )

if ( "rbio" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "RBio" )
endif ( )

if ( "suitesparse_gpuruntime" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "SuiteSparse_GPURuntime" )
endif ( )

if ( "gpuqrengine" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "GPUQREngine" )
endif ( )

if ( "spqr" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "SPQR" )
endif ( )

if ( "graphblas" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "GraphBLAS" )
endif ( )

if ( "spex" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "SPEX" )
endif ( )

if ( "csparse" IN_LIST SUITESPARSE_ENABLE_PROJECTS )
    add_subdirectory ( "CSparse" )
endif ( )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

include ( SuiteSparseReport )
