#-------------------------------------------------------------------------------
# SuiteSparse/KLU/CMakeLists.txt:  cmake for KLU
#-------------------------------------------------------------------------------

# KLU, Copyright (c) 2004-2023, University of Florida.  All Rights Reserved.
# Authors: Timothy A. Davis and Ekanathan Palamadai.
# SPDX-License-Identifier: LGPL-2.1+

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

cmake_minimum_required ( VERSION 3.20 )

set ( KLU_DATE "Sept 18, 2023" )
set ( KLU_VERSION_MAJOR 2 )
set ( KLU_VERSION_MINOR 2 )
set ( KLU_VERSION_SUB   1 )

message ( STATUS "Building KLU version: v"
    ${KLU_VERSION_MAJOR}.
    ${KLU_VERSION_MINOR}.
    ${KLU_VERSION_SUB} " (" ${KLU_DATE} ")" )

#-------------------------------------------------------------------------------
# define the project
#-------------------------------------------------------------------------------

project ( klu
    VERSION "${KLU_VERSION_MAJOR}.${KLU_VERSION_MINOR}.${KLU_VERSION_SUB}"
    LANGUAGES C )
#-------------------------------------------------------------------------------
# SuiteSparse policies
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/../SuiteSparse_config/cmake_modules )

option ( ENABLE_CUDA "Enable CUDA acceleration" on )

include ( SuiteSparsePolicy )

if ( SUITESPARSE_CUDA )
    # KLU with CHOLMOD (which can use CUDA)
    enable_language ( CXX CUDA )
endif ( )

#-------------------------------------------------------------------------------
# find library dependencies
#-------------------------------------------------------------------------------

if ( NOT SUITESPARSE_ROOT_CMAKELISTS )
    find_package ( SuiteSparse_config 7.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../SuiteSparse_config/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::SuiteSparseConfig )
        find_package ( SuiteSparse_config 7.2.0 REQUIRED )
    endif ( )
    add_library ( SuiteSparseConfig ALIAS SuiteSparse::SuiteSparseConfig )
    if ( NOT NSTATIC AND TARGET SuiteSparse::SuiteSparseConfig_static )
        add_library ( SuiteSparseConfig_static ALIAS SuiteSparse::SuiteSparseConfig_static )
    endif ( )

    find_package ( AMD 3.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../AMD/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::AMD )
        find_package ( AMD 3.2.0 REQUIRED )
    endif ( )
    add_library ( AMD ALIAS SuiteSparse::AMD )
    if ( NOT NSTATIC AND TARGET SuiteSparse::AMD_static )
        add_library ( AMD_static ALIAS SuiteSparse::AMD_static )
    endif ( )

    find_package ( COLAMD 3.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../COLAMD/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::COLAMD )
        find_package ( COLAMD 3.2.0 REQUIRED )
    endif ( )
    add_library ( COLAMD ALIAS SuiteSparse::COLAMD )
    if ( NOT NSTATIC AND TARGET SuiteSparse::COLAMD_static )
        add_library ( COLAMD_static ALIAS SuiteSparse::COLAMD_static )
    endif ( )

    find_package ( BTF 2.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../BTF/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::BTF )
        find_package ( BTF 2.2.0 REQUIRED )
    endif ( )
    add_library ( BTF ALIAS SuiteSparse::BTF )
    if ( NOT NSTATIC AND TARGET SuiteSparse::BTF_static )
        add_library ( BTF_static ALIAS SuiteSparse::BTF_static )
    endif ( )
endif ( )

option ( NCHOLMOD "ON: do not use CHOLMOD.  OFF (default): use CHOLMOD" OFF )

if ( NOT SUITESPARSE_ROOT_CMAKELISTS AND NOT NCHOLMOD )
    # look for CHOLMOD (optional fill-reducing orderings)
    find_package ( CHOLMOD 4.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../CHOLMOD/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::CHOLMOD )
        find_package ( CHOLMOD 4.2.0 )
    endif ( )
    if ( CHOLMOD_FOUND )
        add_library ( CHOLMOD ALIAS SuiteSparse::CHOLMOD )
        if ( NOT NSTATIC AND TARGET SuiteSparse::CHOLMOD_static )
            add_library ( CHOLMOD_static ALIAS SuiteSparse::CHOLMOD_static )
        endif ( )
    endif ( )

    # look for CHOLMOD's dependencies: AMD and COLAMD are required.  CAMD and
    # CCOLAMD are optional, but must be found if CHOLMOD was built with them.
    find_package ( CAMD 3.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../CAMD/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::CAMD )
        find_package ( CAMD 3.2.0 )
    endif ( )
    if ( CAMD_FOUND )
        add_library ( CAMD ALIAS SuiteSparse::CAMD )
        if ( NOT NSTATIC AND TARGET SuiteSparse::CAMD_static )
            add_library ( CAMD_static ALIAS SuiteSparse::CAMD_static )
        endif ( )
    endif ( )

    find_package ( CCOLAMD 3.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../CCOLAMD/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::CCOLAMD )
        find_package ( CCOLAMD 3.2.0 )
    endif ( )
    if ( CCOLAMD_FOUND )
        add_library ( CCOLAMD ALIAS SuiteSparse::CCOLAMD )
        if ( NOT NSTATIC AND TARGET SuiteSparse::CCOLAMD_static )
            add_library ( CCOLAMD_static ALIAS SuiteSparse::CCOLAMD_static )
        endif ( )
    endif ( )

    if ( NOT CHOLMOD_FOUND OR NOT AMD_FOUND OR NOT COLAMD_FOUND )
        # CHOLMOD not found so disable it
        set ( NCHOLMOD true )
    endif ( )
    if ( SUITESPARSE_CUDA AND NOT CHOLMOD_CUDA_FOUND )
        set ( NCHOLMOD true )
    endif ( )
endif ( )

if ( NCHOLMOD )
    # tell KLU that CHOLMOD is not available
    message ( STATUS "CHOLMOD not found or not requested" )
    add_compile_definitions ( NCHOLMOD )
else ( )
    message ( STATUS "Using CHOLMOD for addtional pre-ordering options" )
endif ( )

#-------------------------------------------------------------------------------
# configure files
#-------------------------------------------------------------------------------

configure_file ( "Config/klu.h.in"
    "${PROJECT_SOURCE_DIR}/Include/klu.h"
    NEWLINE_STYLE LF )
configure_file ( "Config/klu_version.tex.in"
    "${PROJECT_SOURCE_DIR}/Doc/klu_version.tex"
    NEWLINE_STYLE LF )

#-------------------------------------------------------------------------------
# include directories
#-------------------------------------------------------------------------------

include_directories ( Source Include User )

#-------------------------------------------------------------------------------
# dynamic klu library properties
#-------------------------------------------------------------------------------

file ( GLOB KLU_SOURCES "Source/*.c" )

add_library ( KLU SHARED ${KLU_SOURCES} )

set_target_properties ( KLU PROPERTIES
    VERSION ${KLU_VERSION_MAJOR}.${KLU_VERSION_MINOR}.${KLU_VERSION_SUB}
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    OUTPUT_NAME klu
    SOVERSION ${KLU_VERSION_MAJOR}
    PUBLIC_HEADER "Include/klu.h"
    WINDOWS_EXPORT_ALL_SYMBOLS ON )

target_include_directories ( KLU 
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
              $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )

#-------------------------------------------------------------------------------
# static klu library properties
#-------------------------------------------------------------------------------

if ( NOT NSTATIC )
    add_library ( KLU_static STATIC ${KLU_SOURCES} )

    set_target_properties ( KLU_static PROPERTIES
        VERSION ${KLU_VERSION_MAJOR}.${KLU_VERSION_MINOR}.${KLU_VERSION_SUB}
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        OUTPUT_NAME klu
        SOVERSION ${KLU_VERSION_MAJOR} )

    if ( MSVC )
        set_target_properties ( KLU_static PROPERTIES
            OUTPUT_NAME klu_static )
    endif ( )

    target_include_directories ( KLU_static
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                  $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )

endif ( )

#-------------------------------------------------------------------------------
# klu_cholmod library properties
#-------------------------------------------------------------------------------

if ( NOT NCHOLMOD )

    file ( GLOB KLU_CHOLMOD_SOURCES "User/*.c" )

    add_library ( KLU_CHOLMOD SHARED ${KLU_CHOLMOD_SOURCES} )

    set_target_properties ( KLU_CHOLMOD PROPERTIES
        VERSION ${KLU_VERSION_MAJOR}.${KLU_VERSION_MINOR}.${KLU_VERSION_SUB}
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        OUTPUT_NAME klu_cholmod
        SOVERSION ${KLU_VERSION_MAJOR}
        PUBLIC_HEADER "User/klu_cholmod.h" )

    target_include_directories ( KLU_CHOLMOD 
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                  $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )

    if ( NOT NSTATIC )
        add_library ( KLU_CHOLMOD_static STATIC ${KLU_CHOLMOD_SOURCES} )

        set_target_properties ( KLU_CHOLMOD_static PROPERTIES
            VERSION ${KLU_VERSION_MAJOR}.${KLU_VERSION_MINOR}.${KLU_VERSION_SUB}
            C_STANDARD 11
            C_STANDARD_REQUIRED ON
            OUTPUT_NAME klu_cholmod
            SOVERSION ${KLU_VERSION_MAJOR} )

        if ( MSVC )
            set_target_properties ( KLU_CHOLMOD_static PROPERTIES
                OUTPUT_NAME klu_cholmod_static )
        endif ( )

        target_include_directories ( KLU_CHOLMOD_static 
            INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                      $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )

    endif ( )

endif ( )

#-------------------------------------------------------------------------------
# add the library dependencies
#-------------------------------------------------------------------------------

# SuiteSparseConfig:
target_link_libraries ( KLU PRIVATE SuiteSparseConfig )
if ( NOT NSTATIC )
    if ( TARGET SuiteSparseConfig_static )
        target_link_libraries ( KLU_static PUBLIC SuiteSparseConfig_static )
    else ( )
        target_link_libraries ( KLU_static PUBLIC SuiteSparseConfig )
    endif ( )
endif ( )

# libm:
if ( NOT WIN32 )
    target_link_libraries ( KLU PRIVATE m )
    if ( NOT NSTATIC )
        set ( KLU_STATIC_LIBS "${KLU_STATIC_LIBS} -lm" )
        target_link_libraries ( KLU_static PUBLIC m )
    endif ( )
endif ( )

# AMD:
target_link_libraries ( KLU PRIVATE AMD )
target_include_directories ( KLU PUBLIC
    "$<TARGET_PROPERTY:AMD,INTERFACE_INCLUDE_DIRECTORIES>" )
if ( NOT NSTATIC )
    if ( TARGET AMD_static )
        target_link_libraries ( KLU_static PUBLIC AMD_static )
    else ( )
        target_link_libraries ( KLU_static PUBLIC AMD )
    endif ( )
endif ( )

# COLAMD:
target_link_libraries ( KLU PRIVATE COLAMD )
target_include_directories ( KLU PUBLIC
    "$<TARGET_PROPERTY:COLAMD,INTERFACE_INCLUDE_DIRECTORIES>" )
if ( NOT NSTATIC )
    if ( TARGET COLAMD_static )
        target_link_libraries ( KLU_static PUBLIC COLAMD_static )
    else ( )
        target_link_libraries ( KLU_static PUBLIC COLAMD )
    endif ( )
endif ( )

# BTF:
target_link_libraries ( KLU PRIVATE BTF )
target_include_directories ( KLU PUBLIC
    "$<TARGET_PROPERTY:BTF,INTERFACE_INCLUDE_DIRECTORIES>" )
if ( NOT NSTATIC )
    if ( TARGET BTF_static )
        target_link_libraries ( KLU_static PUBLIC BTF_static )
    else ( )
        target_link_libraries ( KLU_static PUBLIC BTF )
    endif ( )
endif ( )

if ( NOT NCHOLMOD )

    # CHOLMOD:
    # link with CHOLMOD and its dependencies, both required and optional
    # CHOLMOD without CUDA
    target_link_libraries ( KLU PRIVATE CHOLMOD )
    target_link_libraries ( KLU_CHOLMOD PRIVATE CHOLMOD )
    if ( NOT NSTATIC )
       set ( KLU_STATIC_MODULES "${KLU_STATIC_MODULES} CHOLMOD" )
       if ( TARGET CHOLMOD_static )
            target_link_libraries ( KLU_static PUBLIC CHOLMOD_static )
            target_link_libraries ( KLU_CHOLMOD_static PUBLIC CHOLMOD_static )
        else ( )
            target_link_libraries ( KLU_static PUBLIC CHOLMOD )
            target_link_libraries ( KLU_CHOLMOD_static PUBLIC CHOLMOD )
        endif ( )
    endif ( )

    # klu:
    target_link_libraries ( KLU_CHOLMOD PRIVATE KLU BTF )
    if ( NOT NSTATIC )
        if ( TARGET BTF_static )
            target_link_libraries ( KLU_CHOLMOD_static PUBLIC BTF_static )
        else ( )
            target_link_libraries ( KLU_CHOLMOD_static PUBLIC BTF )
        endif ( )
    endif ( )

endif ( )

#-------------------------------------------------------------------------------
# KLU installation location
#-------------------------------------------------------------------------------

include ( CMakePackageConfigHelpers )

install ( TARGETS KLU
    EXPORT KLUTargets
    LIBRARY DESTINATION ${SUITESPARSE_LIBDIR}
    ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR}
    RUNTIME DESTINATION ${SUITESPARSE_BINDIR}
    PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )

if ( NOT NSTATIC )
    install ( TARGETS KLU_static
        EXPORT KLUTargets
        ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR} )
endif ( )

# create (temporary) export target file during build
export ( EXPORT KLUTargets
    NAMESPACE SuiteSparse::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/KLUTargets.cmake )

# install export target, config and version files for find_package
install ( EXPORT KLUTargets
    NAMESPACE SuiteSparse::
    DESTINATION ${SUITESPARSE_LIBDIR}/cmake/KLU )

configure_package_config_file (
    Config/KLUConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KLUConfig.cmake
    INSTALL_DESTINATION ${SUITESPARSE_LIBDIR}/cmake/KLU )

write_basic_package_version_file (
    ${CMAKE_CURRENT_BINARY_DIR}/KLUConfigVersion.cmake
    COMPATIBILITY SameMajorVersion )

install ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KLUConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KLUConfigVersion.cmake
    DESTINATION ${SUITESPARSE_LIBDIR}/cmake/KLU )

#-------------------------------------------------------------------------------
# create pkg-config file for KLU
#-------------------------------------------------------------------------------

if ( NOT MSVC )
    set ( prefix "${CMAKE_INSTALL_PREFIX}" )
    set ( exec_prefix "\${prefix}" )
    cmake_path ( IS_ABSOLUTE SUITESPARSE_LIBDIR SUITESPARSE_LIBDIR_IS_ABSOLUTE )
    if (SUITESPARSE_LIBDIR_IS_ABSOLUTE)
        set ( libdir "${SUITESPARSE_LIBDIR}")
    else ( )
        set ( libdir "\${exec_prefix}/${SUITESPARSE_LIBDIR}")
    endif ( )
    cmake_path ( IS_ABSOLUTE SUITESPARSE_INCLUDEDIR SUITESPARSE_INCLUDEDIR_IS_ABSOLUTE )
    if (SUITESPARSE_INCLUDEDIR_IS_ABSOLUTE)
        set ( includedir "${SUITESPARSE_INCLUDEDIR}")
    else ( )
        set ( includedir "\${prefix}/${SUITESPARSE_INCLUDEDIR}")
    endif ( )
    configure_file (
        Config/KLU.pc.in
        KLU.pc
        @ONLY
        NEWLINE_STYLE LF )
    install ( FILES
        ${CMAKE_CURRENT_BINARY_DIR}/KLU.pc
        DESTINATION ${SUITESPARSE_LIBDIR}/pkgconfig )
endif ( )

#-------------------------------------------------------------------------------
# KLU_CHOLMOD installation
#-------------------------------------------------------------------------------

if ( NOT NCHOLMOD )
    install ( TARGETS KLU_CHOLMOD
        EXPORT KLU_CHOLMODTargets
        LIBRARY DESTINATION ${SUITESPARSE_LIBDIR}
        ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR}
        RUNTIME DESTINATION ${SUITESPARSE_BINDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
    if ( NOT NSTATIC )
        install ( TARGETS KLU_CHOLMOD_static
            EXPORT KLU_CHOLMODTargets
            ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR} )
    endif ( )

    # create (temporary) export target file during build
    export ( EXPORT KLU_CHOLMODTargets
        NAMESPACE SuiteSparse::
        FILE ${CMAKE_CURRENT_BINARY_DIR}/KLU_CHOLMODTargets.cmake )

    # install export target, config and version files for find_package
    install ( EXPORT KLU_CHOLMODTargets
        NAMESPACE SuiteSparse::
        DESTINATION ${SUITESPARSE_LIBDIR}/cmake/KLU_CHOLMOD )

    configure_package_config_file (
        Config/KLU_CHOLMODConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/KLU_CHOLMODConfig.cmake
        INSTALL_DESTINATION ${SUITESPARSE_LIBDIR}/cmake/KLU_CHOLMOD )

    write_basic_package_version_file (
        ${CMAKE_CURRENT_BINARY_DIR}/KLU_CHOLMODConfigVersion.cmake
        COMPATIBILITY SameMajorVersion )

    install ( FILES
        ${CMAKE_CURRENT_BINARY_DIR}/KLU_CHOLMODConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/KLU_CHOLMODConfigVersion.cmake
        DESTINATION ${SUITESPARSE_LIBDIR}/cmake/KLU_CHOLMOD )

    #---------------------------------------------------------------------------
    # create pkg-config file for KLU_CHOLMOD
    #---------------------------------------------------------------------------

    if ( NOT MSVC )
        set ( prefix "${CMAKE_INSTALL_PREFIX}" )
        set ( exec_prefix "\${prefix}" )
        cmake_path ( IS_ABSOLUTE SUITESPARSE_LIBDIR SUITESPARSE_LIBDIR_IS_ABSOLUTE )
        if (SUITESPARSE_LIBDIR_IS_ABSOLUTE)
            set ( libdir "${SUITESPARSE_LIBDIR}")
        else ( )
            set ( libdir "\${exec_prefix}/${SUITESPARSE_LIBDIR}")
        endif ( )
        cmake_path ( IS_ABSOLUTE SUITESPARSE_INCLUDEDIR SUITESPARSE_INCLUDEDIR_IS_ABSOLUTE )
        if (SUITESPARSE_INCLUDEDIR_IS_ABSOLUTE)
            set ( includedir "${SUITESPARSE_INCLUDEDIR}")
        else ( )
            set ( includedir "\${prefix}/${SUITESPARSE_INCLUDEDIR}")
        endif ( )
        configure_file (
            Config/KLU_CHOLMOD.pc.in
            KLU_CHOLMOD.pc
            @ONLY
            NEWLINE_STYLE LF )
        install ( FILES
            ${CMAKE_CURRENT_BINARY_DIR}/KLU_CHOLMOD.pc
            DESTINATION ${SUITESPARSE_LIBDIR}/pkgconfig )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# Demo library and programs
#-------------------------------------------------------------------------------

option ( DEMO "ON: Build the demo programs.  OFF (default): do not build the demo programs." off )
if ( DEMO )

    #---------------------------------------------------------------------------
    # demo library
    #---------------------------------------------------------------------------

    message ( STATUS "Also compiling the demos in KLU/Demo" )

    #---------------------------------------------------------------------------
    # Demo programs
    #---------------------------------------------------------------------------

    add_executable ( klu_simple  "Demo/klu_simple.c" )
    if ( NOT NCHOLMOD )
        add_executable ( kludemo     "Demo/kludemo.c" )
        add_executable ( kluldemo    "Demo/kluldemo.c" )
    endif ( )

    # Libraries required for Demo programs
    target_link_libraries ( klu_simple  PUBLIC KLU )
    if ( NOT NCHOLMOD )
        target_link_libraries ( kludemo  PUBLIC KLU_CHOLMOD KLU CHOLMOD )
        target_link_libraries ( kluldemo PUBLIC KLU_CHOLMOD KLU CHOLMOD )
        if ( NOT WIN32 )
            target_link_libraries ( kludemo  PUBLIC m )
            target_link_libraries ( kluldemo PUBLIC m )
        endif ( )
    endif ( )

else ( )

    message ( STATUS "Skipping the demos in KLU/Demo" )

endif ( )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

if ( NOT SUITESPARSE_ROOT_CMAKELISTS )
    include ( SuiteSparseReport )
endif ( )
