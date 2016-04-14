# - Try to find PHASESPACE
# Once done this will define
#
#  PHASESPACE_FOUND - system has PHASESPACE
#  PHASESPACE_INCLUDE_DIR - the PHASESPACE include directory
#  PHASESPACE_LIBRARIES - Link these to use PHASESPACE
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(PHASESPACE_PKGCONF libPhasespace)

# Include dir
find_path(PHASESPACE_INCLUDE_DIR
  NAMES phasespace/Phasespace.h
  PATHS ${PHASESPACE_PKGCONF_INCLUDE_DIRS} ./phasespace ../phasespace ../../phasespace /usr/include /usr/local/include /opt/local/include
)

# Finally the library itself
find_library(PHASESPACE_LIBRARY
  NAMES PHASESPACE
  PATHS ${PHASESPACE_PKGCONF_LIBRARY_DIRS} ./phasespace/build/lib ../phasespace/build/lib ../../phasespace/build/lib /usr/lib /usr/local/lib /opt/local/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(PHASESPACE_PROCESS_INCLUDES PHASESPACE_INCLUDE_DIR)
set(PHASESPACE_PROCESS_LIBS PHASESPACE_LIBRARY)
libfind_process(PHASESPACE)



