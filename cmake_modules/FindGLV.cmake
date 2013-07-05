# - Try to find GLV
# Once done this will define
#
#  GLV_FOUND - system has GLV
#  GLV_INCLUDE_DIR - the GLV include directory
#  GLV_LIBRARIES - Link these to use GLV
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(GLV_PKGCONF libGLV)

# Include dir
find_path(GLV_INCLUDE_DIR
  NAMES GLV/glv.h
  PATHS ${GLV_PKGCONF_INCLUDE_DIRS} ./GLV ../GLV ../../GLV /usr/include /usr/local/include /opt/local/include
)

# Finally the library itself
find_library(GLV_LIBRARY
  NAMES GLV
  PATHS ${GLV_PKGCONF_LIBRARY_DIRS} ./GLV/build/lib ../GLV/build/lib ../../GLV/build/lib /usr/lib /usr/local/lib /opt/local/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(GLV_PROCESS_INCLUDES GLV_INCLUDE_DIR)
set(GLV_PROCESS_LIBS GLV_LIBRARY)
libfind_process(GLV)



