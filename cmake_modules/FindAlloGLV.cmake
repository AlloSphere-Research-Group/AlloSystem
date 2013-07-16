# - Try to find AlloGLV
# Once done this will define
#
#  ALLOGLV_FOUND - system has AlloGLV
#  ALLOGLV_INCLUDE_DIR - the AlloGLV include directory
#  ALLOGLV_LIBRARY - Link these to use AlloGLV
#

include(LibFindMacros)

# Dependencies
#libfind_package(LO Magick)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(ALLOGLV_PKGCONF liballoGLV)

# Include dir
find_path(ALLOGLV_INCLUDE_DIR
  NAMES alloGLV/al_ControlGLV.hpp
  PATHS ${ALLOGLV_PKGCONF_INCLUDE_DIRS} ./ ../alloGLV /usr/include /usr/local/include /opt/local/include 
        ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/alloGLV
)

# Finally the library itself
find_library(ALLOGLV_LIBRARY
  NAMES alloGLV
  PATHS ${ALLOGLV_PKGCONF_LIBRARY_DIRS} ./build/lib ../build/lib /usr/lib /usr/local/lib /opt/local/lib 
        ${CMAKE_CURRENT_SOURCE_DIR}/../AlloSystem/build/lib
)

#/usr/include/assimp
#/usr/local/include
#/opt/local/include/assimp
#/usr/local/Cellar/assimp/2.0.863/include/assimp

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(ALLOGLV_PROCESS_INCLUDES ALLOGLV_INCLUDE_DIR)
set(ALLOGLV_PROCESS_LIBS ALLOGLV_LIBRARY)
libfind_process(ALLOGLV)



