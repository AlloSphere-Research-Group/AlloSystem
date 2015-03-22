# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libsndfile.

#FIND_PATH(LIBSNDFILE_INCLUDE_DIR sndfile.h)

#SET(LIBSNDFILE_NAMES ${LIBSNDFILE_NAMES} sndfile libsndfile)
#FIND_LIBRARY(LIBSNDFILE_LIBRARY NAMES ${LIBSNDFILE_NAMES} PATH)

#IF(LIBSNDFILE_INCLUDE_DIR AND LIBSNDFILE_LIBRARY)
#	SET(LIBSNDFILE_FOUND TRUE)
#ENDIF(LIBSNDFILE_INCLUDE_DIR AND LIBSNDFILE_LIBRARY)

#IF(LIBSNDFILE_FOUND)
#	IF(NOT LibSndFile_FIND_QUIETLY)
#		MESSAGE(STATUS "Found LibSndFile: ${LIBSNDFILE_LIBRARY}")
#	ENDIF (NOT LibSndFile_FIND_QUIETLY)
#ELSE(LIBSNDFILE_FOUND)
#	IF(LibSndFile_FIND_REQUIRED)
#		MESSAGE(FATAL_ERROR "Could not find sndfile")
#	ENDIF(LibSndFile_FIND_REQUIRED)
#ENDIF (LIBSNDFILE_FOUND)


# - Try to find libsndfile
# Once done this will define
#
#  SNDFILE_FOUND - system has libsndfile
#  SNDFILE_INCLUDE_DIRS - the libsndfile include directory
#  SNDFILE_LIBRARIES - Link these to use libsndfile
#
#  Copyright (C) 2006  Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LIBSNDFILE_PKGCONF libsndfile)

if (SNDFILE_LIBRARIES AND SNDFILE_INCLUDE_DIRS)
  # in cache already
  set(SNDFILE_FOUND TRUE)
else (SNDFILE_LIBRARIES AND SNDFILE_INCLUDE_DIRS)

  find_path(SNDFILE_INCLUDE_DIR
    NAMES
      sndfile.h
    PATHS
      ${LIBSNDFILE_PKGCONF_INCLUDE_DIRS}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      /mingw64/include
  )

  find_library(SNDFILE_LIBRARY
    NAMES
      sndfile
      libsndfile
    PATHS
      ${LIBSNDFILE_PKGCONF_LIBRARY_DIRS}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      /mingw64/lib/
  )

  set(SNDFILE_INCLUDE_DIRS
    ${SNDFILE_INCLUDE_DIR}
  )
  set(SNDFILE_LIBRARIES
    ${SNDFILE_LIBRARY}
  )

  if (SNDFILE_INCLUDE_DIRS AND SNDFILE_LIBRARIES)
    set(SNDFILE_FOUND TRUE)
  endif (SNDFILE_INCLUDE_DIRS AND SNDFILE_LIBRARIES)

  if (SNDFILE_FOUND)
    if (NOT SndFile_FIND_QUIETLY)
#      message(STATUS "Found libsndfile: ${SNDFILE_LIBRARIES}")
    endif (NOT SndFile_FIND_QUIETLY)
  else (SNDFILE_FOUND)
    if (SndFile_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libsndfile")
    endif (SndFile_FIND_REQUIRED)
  endif (SNDFILE_FOUND)

  # show the SNDFILE_INCLUDE_DIRS and SNDFILE_LIBRARIES variables only in the advanced view
  mark_as_advanced(SNDFILE_INCLUDE_DIRS SNDFILE_LIBRARIES)

endif (SNDFILE_LIBRARIES AND SNDFILE_INCLUDE_DIRS)
