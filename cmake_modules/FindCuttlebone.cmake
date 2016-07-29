# - Try to find CUTTLEBONE
# Once done this will define
#
#  CUTTLEBONE_FOUND - system has CUTTLEBONE
#  CUTTLEBONE_INCLUDE_DIR - the CUTTLEBONE include directory
#  CUTTLEBONE_LIBRARIES - Link these to use CUTTLEBONE
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(CUTTLEBONE_PKGCONF libCuttlebone)

# Include dir
find_path(CUTTLEBONE_INCLUDE_DIR
  NAMES cuttlebone/Cuttlebone.h
  PATHS ${CUTTLEBONE_PKGCONF_INCLUDE_DIRS} ./cuttlebone ../cuttlebone ../../cuttlebone
 		./Cuttlebone ../Cuttlebone ../../Cuttlebone
		 /usr/include /usr/local/include
		 /opt/local/include /tmp/cuttlebone
)

# Finally the library itself
find_library(CUTTLEBONE_LIBRARY
  NAMES CUTTLEBONE
  PATHS ${CUTTLEBONE_PKGCONF_LIBRARY_DIRS}
	./cuttlebone/build/lib ../cuttlebone/build/lib ../../cuttlebone/build/lib
	./Cuttlebone/build/lib ../Cuttlebone/build/lib ../..Cuttlebone/build/lib
	/usr/lib /usr/local/lib /opt/local/lib /tmp/cuttlebone
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(CUTTLEBONE_PROCESS_INCLUDES CUTTLEBONE_INCLUDE_DIR)
set(CUTTLEBONE_PROCESS_LIBS CUTTLEBONE_LIBRARY)
libfind_process(CUTTLEBONE)
