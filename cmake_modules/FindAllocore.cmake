# - Try to find Allocore
# Once done this will define
#
#  ALLOCORE_FOUND - system has Allocore
#  ALLOCORE_INCLUDE_DIR - the Allocore include directory
#  ALLOCORE_LIBRARY - Link these to use Allocore
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(ALLOCORE_PKGCONF liballocore)

# Include dir
find_path(ALLOCORE_INCLUDE_DIR
  NAMES allocore/al_Allocore.hpp
  PATHS ${ALLOCORE_PKGCONF_INCLUDE_DIRS} ./ ../allocore /usr/include /usr/local/include /opt/local/include ${CMAKE_CURRENT_SOURCE_DIR}/AlloSystem/allocore ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/allocore
)

# Finally the library itself
find_library(ALLOCORE_LIBRARY
  NAMES allocore
  PATHS ${ALLOCORE_PKGCONF_LIBRARY_DIRS} ./build/lib ../build/lib /usr/lib /usr/local/lib /opt/local/lib ${CMAKE_CURRENT_SOURCE_DIR}/AlloSystem/build/lib ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/build/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(ALLOCORE_PROCESS_INCLUDES ALLOCORE_INCLUDE_DIR)
set(ALLOCORE_PROCESS_LIBS ALLOCORE_LIBRARY)
libfind_process(ALLOCORE)



