# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
# Modified by Andres Cabrera <andres@mat.ucsb.edu>
#
# Find Gamma.

#FIND_PATH(GAMMA_INCLUDE_DIR sndfile.h)

#SET(GAMMA_NAMES ${GAMMA_NAMES} Gamma libGamma)
#FIND_LIBRARY(GAMMA_LIBRARY NAMES ${GAMMA_NAMES} PATH)

#IF(GAMMA_INCLUDE_DIR AND GAMMA_LIBRARY)
#	SET(GAMMA_FOUND TRUE)
#ENDIF(GAMMA_INCLUDE_DIR AND GAMMA_LIBRARY)

#IF(GAMMA_FOUND)
#	IF(NOT Gamma_FIND_QUIETLY)
#		MESSAGE(STATUS "Found Gamma: ${GAMMA_LIBRARY}")
#	ENDIF (NOT Gamma_FIND_QUIETLY)
#ELSE(GAMMA_FOUND)
#	IF(Gamma_FIND_REQUIRED)
#		MESSAGE(FATAL_ERROR "Could not find Gamma")
#	ENDIF(Gamma_FIND_REQUIRED)
#ENDIF (GAMMA_FOUND)


# - Try to find libsndfile
# Once done this will define

#  GAMMA_FOUND - system has libsndfile
#  GAMMA_INCLUDE_DIRS - the libsndfile include directory
#  GAMMA_LIBRARIES - Link these to use libsndfile

#  Copyright (C) 2006  Wengo

#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (GAMMA_LIBRARIES AND GAMMA_INCLUDE_DIRS)
  # in cache already
  set(GAMMA_FOUND TRUE)
else (GAMMA_LIBRARIES AND GAMMA_INCLUDE_DIRS)

  find_path(GAMMA_INCLUDE_DIR
    NAMES
	  Gamma.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(GAMMA_LIBRARY
    NAMES
	  Gamma
	  libGamma
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(GAMMA_INCLUDE_DIRS
	${GAMMA_INCLUDE_DIR}
  )
  set(GAMMA_LIBRARIES
	${GAMMA_LIBRARY}
  )

  if (GAMMA_INCLUDE_DIRS AND GAMMA_LIBRARIES)
	set(GAMMA_FOUND TRUE)
  endif (GAMMA_INCLUDE_DIRS AND GAMMA_LIBRARIES)

  if (GAMMA_FOUND)
	if (NOT Gamma_FIND_QUIETLY)
	  message(STATUS "Found Gamma: ${GAMMA_LIBRARIES}")
	endif (NOT Gamma_FIND_QUIETLY)
  else (GAMMA_FOUND)
	if (Gamma_FIND_REQUIRED)
	  message(FATAL_ERROR "Could not find Gamma")
	endif (Gamma_FIND_REQUIRED)
  endif (GAMMA_FOUND)

  # show the GAMMA_INCLUDE_DIRS and GAMMA_LIBRARIES variables only in the advanced view
  mark_as_advanced(GAMMA_INCLUDE_DIRS GAMMA_LIBRARIES)

endif (GAMMA_LIBRARIES AND GAMMA_INCLUDE_DIRS)
