# - Try to find AlloAux
# Once done this will define
#
#  ALLOAUX_FOUND - system has AlloAux
#  ALLOAUX_INCLUDE_DIR - the AlloAux include directory
#  ALLOAUX_LIBRARY - Link these to use AlloAux
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(ALLOAUX_PKGCONF liballoaux)

# Include dir
find_path(ALLOAUX_INCLUDE_DIR
  NAMES alloaux/al_Lua.hpp
  PATHS ${ALLOAUX_PKGCONF_INCLUDE_DIRS} ./ ../alloaux /usr/include /usr/local/include /opt/local/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/alloaux
)

# Finally the library itself
find_library(ALLOAUX_LIBRARY
  NAMES alloaux
  PATHS ${ALLOAUX_PKGCONF_LIBRARY_DIRS} ./build/lib ../build/lib /usr/lib /usr/local/lib /opt/local/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/build/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(ALLOAUX_PROCESS_INCLUDES ALLOAUX_INCLUDE_DIR)
set(ALLOAUX_PROCESS_LIBS ALLOAUX_LIBRARY)
libfind_process(ALLOAUX)



