# - Try to find Gamma
# Once done this will define

#  GAMMA_FOUND - system has Gamma
#  GAMMA_INCLUDE_DIRS - the Gamma include directory
#  GAMMA_LIBRARY - Link these to use Gamma

#  Copyright (C) 2006  Wengo

#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (GAMMA_LIBRARY AND GAMMA_INCLUDE_DIR)
  # in cache already
  set(GAMMA_FOUND TRUE)
else (GAMMA_LIBRARY AND GAMMA_INCLUDE_DIR)

  find_path(GAMMA_INCLUDE_DIR
    NAMES
          Gamma/Gamma.h
    PATHS
        ${CMAKE_SOURCE_DIR}/../Gamma
        ${CMAKE_SOURCE_DIR}/../../Gamma
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
        ${CMAKE_SOURCE_DIR}/../Gamma/build/lib
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

  if (GAMMA_INCLUDE_DIR AND GAMMA_LIBRARY)
	set(GAMMA_FOUND TRUE)
  endif (GAMMA_INCLUDE_DIR AND GAMMA_LIBRARY)

  if (GAMMA_FOUND)
	if (NOT Gamma_FIND_QUIETLY)
	  message(STATUS "Found Gamma: ${GAMMA_LIBRARY}")
	endif (NOT Gamma_FIND_QUIETLY)
  else (GAMMA_FOUND)
	if (Gamma_FIND_REQUIRED)
	  message(FATAL_ERROR "Could not find Gamma")
	endif (Gamma_FIND_REQUIRED)
  endif (GAMMA_FOUND)

  # show the GAMMA_INCLUDE_DIRS and GAMMA_LIBRARIES variables only in the advanced view
  # mark_as_advanced(GAMMA_INCLUDE_DIRS GAMMA_LIBRARIES)

endif (GAMMA_LIBRARY AND GAMMA_INCLUDE_DIR)
