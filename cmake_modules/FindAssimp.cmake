# - Try to find Assimp
# Once done this will define
#
#  ASSIMP_FOUND - system has Assimp
#  ASSIMP_INCLUDE_DIR - the Assimp include directory
#  ASSIMP_LIBRARIES - Link these to use Assimp
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)
if(USE_ASSIMP_V3)
    add_definitions(-DUSE_ASSIMP3)
endif()

if((NOT ASSIMP_INCLUDE_DIR) AND (NOT ASSIMP_LIBRARY)) 
  # Use pkg-config to get hints about paths
  libfind_pkg_check_modules(ASSIMP_PKGCONF libassimp)

  # Include dir
  find_path(ASSIMP2_INCLUDE_DIR
    NAMES assimp/assimp.h
    PATHS ${ASSIMP_PKGCONF_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include
    )

  find_path(ASSIMP3_INCLUDE_DIR
    NAMES assimp/types.h
    PATHS ${ASSIMP_PKGCONF_INCLUDE_DIRS} /usr/include /usr/local/include /opt/local/include
    )

  if(ASSIMP2_INCLUDE_DIR)
    message(STATUS "Assimp 2 found")
    set(ASSIMP_INCLUDE_DIR ${ASSIMP2_INCLUDE_DIR})
  endif(ASSIMP2_INCLUDE_DIR)

  if(ASSIMP3_INCLUDE_DIR)
    message(STATUS "Assimp 3 found")
    set(ASSIMP_INCLUDE_DIR ${ASSIMP3_INCLUDE_DIR})
    set(USE_ASSIMP_V3 1 CACHE STRING "Use assimp v3")
    add_definitions(-DUSE_ASSIMP3)
  endif(ASSIMP3_INCLUDE_DIR)

  # Finally the library itself
  find_library(ASSIMP_LIBRARY
    NAMES assimp
    PATHS ${ASSIMP_PKGCONF_LIBRARY_DIRS}
    )

  #/usr/include/assimp
  #/usr/local/include
  #/opt/local/include/assimp
  #/usr/local/Cellar/assimp/2.0.863/include/assimp

  # Set the include dir variables and the libraries and let libfind_process do the rest.
  # NOTE: Singular variables for this library, plural for libraries this this lib depends on.
  set(ASSIMP_PROCESS_INCLUDES ${ASSIMP_INCLUDE_DIR})
  set(ASSIMP_PROCESS_LIBS ${ASSIMP_LIBRARY})
  libfind_process(ASSIMP)

endif((NOT ASSIMP_INCLUDE_DIR) AND (NOT ASSIMP_LIBRARY)) 

