#-------------------------------------------------------------------------------
# SuiteSparse/BTF/CMakeLists.txt:  cmake for BTF
#-------------------------------------------------------------------------------

# BTF, Copyright (c) 2004-2023, University of Florida.  All Rights Reserved.
# Author: Timothy A. Davis.
# SPDX-License-Identifier: LGPL-2.1+

#-------------------------------------------------------------------------------
# get the version
#-------------------------------------------------------------------------------

cmake_minimum_required ( VERSION 3.20 )

set ( BTF_DATE "Sept 18, 2023" )
set ( BTF_VERSION_MAJOR 2 )
set ( BTF_VERSION_MINOR 2 )
set ( BTF_VERSION_SUB   1 )

message ( STATUS "Building BTF version: v"
    ${BTF_VERSION_MAJOR}.
    ${BTF_VERSION_MINOR}.
    ${BTF_VERSION_SUB} " (" ${BTF_DATE} ")" )

#-------------------------------------------------------------------------------
# define the project
#-------------------------------------------------------------------------------

project ( btf
    VERSION "${BTF_VERSION_MAJOR}.${BTF_VERSION_MINOR}.${BTF_VERSION_SUB}"
    LANGUAGES C )

#-------------------------------------------------------------------------------
# SuiteSparse policies
#-------------------------------------------------------------------------------

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/../SuiteSparse_config/cmake_modules )

include ( SuiteSparsePolicy )

#-------------------------------------------------------------------------------
# find library dependencies
#-------------------------------------------------------------------------------

if ( NOT SUITESPARSE_ROOT_CMAKELISTS )
    option( BUILD_SHARED_LIBS "Build shared libraries" on )
    option( BUILD_STATIC_LIBS "Build static libraries" on )
    find_package ( SuiteSparse_config 7.2.0
        PATHS ${CMAKE_SOURCE_DIR}/../SuiteSparse_config/build NO_DEFAULT_PATH )
    if ( NOT TARGET SuiteSparse::SuiteSparseConfig )
        find_package ( SuiteSparse_config 7.2.0 REQUIRED )
    endif ( )
    if ( BUILD_SHARED_LIBS AND TARGET SuiteSparse::SuiteSparseConfig )
        add_library ( SuiteSparseConfig ALIAS SuiteSparse::SuiteSparseConfig )
    endif ( )
    if ( BUILD_STATIC_LIBS AND TARGET SuiteSparse::SuiteSparseConfig_static )
        add_library ( SuiteSparseConfig_static ALIAS SuiteSparse::SuiteSparseConfig_static )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# configure files
#-------------------------------------------------------------------------------

configure_file ( "Config/btf.h.in" "${PROJECT_SOURCE_DIR}/Include/btf.h"
    NEWLINE_STYLE LF )

#-------------------------------------------------------------------------------
# include directories
#-------------------------------------------------------------------------------

include_directories ( Source Include )

#-------------------------------------------------------------------------------
# dynamic btf library properties
#-------------------------------------------------------------------------------

file ( GLOB BTF_SOURCES "Source/*.c" )

if ( BUILD_SHARED_LIBS )
    add_library ( BTF SHARED ${BTF_SOURCES} )

    set_target_properties ( BTF PROPERTIES
        VERSION ${BTF_VERSION_MAJOR}.${BTF_VERSION_MINOR}.${BTF_VERSION_SUB}
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        OUTPUT_NAME btf
        SOVERSION ${BTF_VERSION_MAJOR}
        PUBLIC_HEADER "Include/btf.h" 
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

    target_include_directories ( BTF
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                  $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )
endif ( )

#-------------------------------------------------------------------------------
# static btf library properties
#-------------------------------------------------------------------------------

if ( BUILD_STATIC_LIBS )
    add_library ( BTF_static STATIC ${BTF_SOURCES} )

    set_target_properties ( BTF_static PROPERTIES
        VERSION ${BTF_VERSION_MAJOR}.${BTF_VERSION_MINOR}.${BTF_VERSION_SUB}
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        OUTPUT_NAME btf
        SOVERSION ${BTF_VERSION_MAJOR}
        PUBLIC_HEADER "Include/btf.h" )

    if ( MSVC )
        set_target_properties ( BTF_static PROPERTIES
            OUTPUT_NAME btf_static )
    endif ( )

    target_include_directories ( BTF_static
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
                  $<INSTALL_INTERFACE:${SUITESPARSE_INCLUDEDIR}> )

endif ( )

#-------------------------------------------------------------------------------
# add the library dependencies
#-------------------------------------------------------------------------------

# SuiteSparseConfig:
if ( BUILD_SHARED_LIBS )
    target_link_libraries ( BTF PRIVATE SuiteSparseConfig )
    target_include_directories ( BTF PUBLIC
        "$<TARGET_PROPERTY:SuiteSparseConfig,INTERFACE_INCLUDE_DIRECTORIES>" )
endif ( )
if ( BUILD_STATIC_LIBS )
    if ( TARGET SuiteSparseConfig_static )
        target_link_libraries ( BTF_static PUBLIC SuiteSparseConfig_static )
    else ( )
        target_link_libraries ( BTF_static PUBLIC SuiteSparseConfig )
    endif ( )
endif ( )

# libm:
if ( NOT WIN32 )
    if ( BUILD_SHARED_LIBS )
        target_link_libraries ( BTF PRIVATE m )
    endif ( )
    if ( BUILD_STATIC_LIBS )
        set ( BTF_STATIC_LIBS "${BTF_STATIC_LIBS} -lm" )
        target_link_libraries ( BTF_static PUBLIC m )
    endif ( )
endif ( )

#-------------------------------------------------------------------------------
# BTF installation location
#-------------------------------------------------------------------------------

include ( CMakePackageConfigHelpers )

if ( BUILD_SHARED_LIBS )
    install ( TARGETS BTF
        EXPORT BTFTargets
        LIBRARY DESTINATION ${SUITESPARSE_LIBDIR}
        ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR}
        RUNTIME DESTINATION ${SUITESPARSE_BINDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
endif ( )
if ( BUILD_STATIC_LIBS )
    install ( TARGETS BTF_static
        EXPORT BTFTargets
        ARCHIVE DESTINATION ${SUITESPARSE_LIBDIR}
        PUBLIC_HEADER DESTINATION ${SUITESPARSE_INCLUDEDIR} )
endif ( )

# create (temporary) export target file during build
export ( EXPORT BTFTargets
    NAMESPACE SuiteSparse::
    FILE ${CMAKE_CURRENT_BINARY_DIR}/BTFTargets.cmake )

# install export target, config and version files for find_package
install ( EXPORT BTFTargets
    NAMESPACE SuiteSparse::
    DESTINATION ${SUITESPARSE_LIBDIR}/cmake/BTF )

configure_package_config_file (
    Config/BTFConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/BTFConfig.cmake
    INSTALL_DESTINATION ${SUITESPARSE_LIBDIR}/cmake/BTF )

write_basic_package_version_file (
    ${CMAKE_CURRENT_BINARY_DIR}/BTFConfigVersion.cmake
    COMPATIBILITY SameMajorVersion )

install ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/BTFConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/BTFConfigVersion.cmake
    DESTINATION ${SUITESPARSE_LIBDIR}/cmake/BTF )

#-------------------------------------------------------------------------------
# create pkg-config file
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
        Config/BTF.pc.in
        BTF.pc
        @ONLY
        NEWLINE_STYLE LF )
    install ( FILES
        ${CMAKE_CURRENT_BINARY_DIR}/BTF.pc
        DESTINATION ${SUITESPARSE_LIBDIR}/pkgconfig )
endif ( )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

if ( NOT SUITESPARSE_ROOT_CMAKELISTS )
    include ( SuiteSparseReport )
endif ( )
