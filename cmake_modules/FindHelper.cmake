#
# - Try to find a library using pkg-config if available,
#   than portable macros FIND_PATH and FIND_LIBRARY
#
# Once done this will define
#
#  ${prefix}_FOUND          - system has the library
#  ${prefix}_INCLUDE_DIR    - the library include directory
#  ${prefix}_LIBRARIES      - Link these to library
#  ${prefix}_DEFINITIONS    - Compiler switches required for using this libray
#
# Copyright (c) 2009, JÃ©rÃ©my Zurcher, <jeremy@asynk.ch>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(FIND_HELPER prefix pkg_name header lib)

    if(${prefix}_INCLUDE_DIR AND ${prefix}_LIBRARIES)
        set(${prefix}_FIND_QUIETLY TRUE)
    else()
        if(NOT WIN32)
            FIND_PACKAGE(PkgConfig)
        endif()
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(${prefix} ${pkg_name})
            #MESSAGE(STATUS  " LDFLAGS       ${${prefix}_LDFLAGS}" )
            #MESSAGE(STATUS  " CFLAGS        ${${prefix}_CFLAGS}" )
            #MESSAGE(STATUS  " INCLUDEDIRS   ${${prefix}_INCLUDE_DIRS}" )
            set(${prefix}_DEFINITIONS ${prefix}_CFLAGS_OTHER )
        else()
            MESSAGE(STATUS "Checking for module '${pkg_name}'")
        endif()

        #set(CMAKE_PREFIX_PATH ${${prefix}_PATH} )

        find_path(${prefix}_INCLUDE_DIR
            NAMES ${header}
            PATHS ${${prefix}_INCLUDE_DIRS} ${${prefix}_INCLUDEDIR} ${${prefix}_INCLUDE_PATHS} ENV ${prefix}_INCLUDE
        )

        find_library(${prefix}_LIBRARIES
            NAMES ${lib}
            PATHS ${${prefix}_LIBDIR} ${${prefix}_LIBRARY_DIRS} ${${prefix}_LIB_PATHS} ENV ${prefix}_PATH
        )
    endif()

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(${prefix} DEFAULT_MSG ${prefix}_LIBRARIES ${prefix}_INCLUDE_DIR)
    mark_as_advanced(${prefix}_INCLUDE_DIR  ${prefix}_LIBRARIES)

endmacro()
