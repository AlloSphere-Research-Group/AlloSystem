# - Try to find VSR
# Once done this will define
#
#  VSR_FOUND - system has VSR
#  VSR_INCLUDE_DIR - the VSR include directory
#  VSR_LIBRARIES - Link these to use VSR
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(VSR_PKGCONF libvsr)

# Include dir
find_path(VSR_INCLUDE_DIR
  NAMES VSR/vsr.h
  PATHS ${VSR_PKGCONF_INCLUDE_DIRS} ./vsr ../vsr ../../vsr /usr/include /usr/local/include /opt/local/include
)

# Finally the library itself
find_library(VSR_LIBRARY
  NAMES vsr
  PATHS ${VSR_PKGCONF_LIBRARY_DIRS} ./vsr/build/lib ../vsr/build/lib ../../vsr/build/lib /usr/lib /usr/local/lib /opt/local/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(VSR_PROCESS_INCLUDES VSR_INCLUDE_DIR)
set(VSR_PROCESS_LIBS VSR_LIBRARY)
libfind_process(VSR)



