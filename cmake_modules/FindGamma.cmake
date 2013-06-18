# - Try to find Gamma
# Once done this will define

#  GAMMA_FOUND - system has libsndfile
#  GAMMA_INCLUDE_DIRS - the libsndfile include directory
#  GAMMA_LIBRARIES - Link these to use libsndfile

if (GAMMA_LIBRARIES AND GAMMA_INCLUDE_DIRS)
  # in cache already
  set(GAMMA_FOUND TRUE)
else (GAMMA_LIBRARIES AND GAMMA_INCLUDE_DIRS)

  find_path(GAMMA_INCLUDE_DIR
    NAMES
	  Gamma.h
    PATHS
      /usr/include/Gamma
      /usr/local/include/Gamma
      /opt/local/include/Gamma
  )

  find_library(GAMMA_LIBRARY
    NAMES
	  Gamma
	  libGamma
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
  )

  if (GAMMA_INCLUDE_DIR AND GAMMA_LIBRARY)
	set(GAMMA_FOUND TRUE)
  endif (GAMMA_INCLUDE_DIR AND GAMMA_LIBRARY)

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
  # mark_as_advanced(GAMMA_INCLUDE_DIRS GAMMA_LIBRARIES)

endif (GAMMA_LIBRARIES AND GAMMA_INCLUDE_DIRS)
