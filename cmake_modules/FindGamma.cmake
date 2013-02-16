# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
# Modified by Andres Cabrera <andres@mat.ucsb.edu>
#
# Find Gamma.

#FIND_PATH(GAMMA_INCLUDE_DIR Gamma.h)

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


# - Try to find Gamma
# Once done this will define

#  GAMMA_FOUND - system has Gamma
#  GAMMA_INCLUDE_DIRS - the Gamma include directory
#  GAMMA_LIBRARIES - Link these to use Gamma

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
	${CMAKE_SOURCE_DIR}/../../Gamma/Gamma
      /usr/include/Gamma
      /usr/local/include/Gamma
      /opt/local/include/Gamma
      /sw/include/Gamma
  )

  find_library(GAMMA_LIBRARY
    NAMES
	  Gamma
	  libGamma
    PATHS
	${CMAKE_SOURCE_DIR}/../../Gamma/build/lib
      /usr/lib
      /opt/local/lib
      /usr/local/lib
      /sw/lib
  )

  if (${GAMMA_LIBRARY} STREQUAL "")
    find_path(GAMMA_LIBRARY
    libGamma.a
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib)
  endif()


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
