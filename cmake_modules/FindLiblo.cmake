# - Try to find LO
# Once done, this will define
#
#  LO_FOUND - system has LO
#  LO_INCLUDE_DIRS - the LO include directories
#  LO_LIBRARIES - link these to use LO

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LO_PKGCONF liblo)

# Include dir
find_path(LO_INCLUDE_DIR
  NAMES lo/lo.h
  PATHS ${LO_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(LO_LIBRARY
  NAMES lo
  PATHS ${LO_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LO_PROCESS_INCLUDES LO_INCLUDE_DIR)
set(LO_PROCESS_LIBS LO_LIBRARY)
libfind_process(LO)
