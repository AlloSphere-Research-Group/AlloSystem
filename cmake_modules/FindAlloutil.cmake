# - Try to find Alloutil
# Once done this will define
#
#  ALLOUTIL_FOUND - system has Alloutil
#  ALLOUTIL_INCLUDE_DIR - the Alloutil include directory
#  ALLOUTIL_LIBRARY - Link these to use Alloutil
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(ALLOUTIL_PKGCONF liballoutil)

# Include dir
find_path(ALLOUTIL_INCLUDE_DIR
  NAMES alloutil/al_Lua.hpp
  PATHS ${ALLOUTIL_PKGCONF_INCLUDE_DIRS} ./ ../alloutil /usr/include /usr/local/include /opt/local/include 
        ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/alloutil
)

# Finally the library itself
find_library(ALLOUTIL_LIBRARY
  NAMES alloutil
  PATHS ${ALLOUTIL_PKGCONF_LIBRARY_DIRS} ./build/lib ../build/lib /usr/lib /usr/local/lib /opt/local/lib 
        ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/build/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(ALLOUTIL_PROCESS_INCLUDES ALLOUTIL_INCLUDE_DIR)
set(ALLOUTIL_PROCESS_LIBS ALLOUTIL_LIBRARY)
libfind_process(ALLOUTIL)



