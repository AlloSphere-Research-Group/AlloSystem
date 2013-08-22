# - try to find glut library and include files
#  GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  GLUT_LIBRARIES, the libraries to link against
#  GLUT_FOUND, If false, do not try to use GLUT.
# Also defined, but not for general use are:
#  GLUT_glut_LIBRARY = the full path to the glut library.
#  GLUT_Xmu_LIBRARY  = the full path to the Xmu library.
#  GLUT_Xi_LIBRARY   = the full path to the Xi Library.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

include(LibFindMacros)

if ((NOT GLUT_INCLUDE_DIR) AND (NOT GLUT_LIBRARY))
  libfind_pkg_check_modules(GLUT_PKGCONF glut)

  IF (WIN32)
    FIND_PATH( GLUT_INCLUDE_DIR NAMES GL/glut.h 
      PATHS  ${GLUT_PKGCONF_INCLUDE_DIRS} ${GLUT_ROOT_PATH}/include )
    FIND_LIBRARY( GLUT_glut_LIBRARY NAMES glut glut32
      PATHS
      ${GLUT_PKGCONF_LIBRARY_DIRS}
      ${OPENGL_LIBRARY_DIR}
      ${GLUT_ROOT_PATH}/Release
      )
  ELSE (WIN32)
    
    IF (APPLE)
      # These values for Apple could probably do with improvement.
      FIND_PATH( GLUT_INCLUDE_DIR glut.h
	/System/Library/Frameworks/GLUT.framework/Versions/A/Headers
	${OPENGL_LIBRARY_DIR}
	)
      SET(GLUT_glut_LIBRARY "-framework GLUT" CACHE STRING "GLUT library for OSX") 
      SET(GLUT_cocoa_LIBRARY "-framework Cocoa" CACHE STRING "Cocoa framework for OSX")
    ELSE (APPLE)
      message("glut   	${GLUT_PKGCONF_INCLUDE_DIRS}")
      if (NOT GLUT_PKGCONF_INCLUDE_DIRS)
	libfind_pkg_check_modules(GLUT_PKGCONF freeglut3)
      endif(NOT GLUT_PKGCONF_INCLUDE_DIRS)
      FIND_PATH( GLUT_INCLUDE_DIR GL/glut.h
	${GLUT_PKGCONF_INCLUDE_DIRS}
	/usr/include
	/usr/openwin/share/include
	/usr/openwin/include
	/opt/graphics/OpenGL/include
	/opt/graphics/OpenGL/contrib/libglut
	)
      
      FIND_LIBRARY( GLUT_glut_LIBRARY glut
	${GLUT_PKGCONF_LIBRARY_DIRS}
	/usr/openwin/lib
	/usr/lib
	)
      
      #    FIND_LIBRARY( GLUT_Xi_LIBRARY Xi
      #      /usr/openwin/lib
      #      )
      
      #    FIND_LIBRARY( GLUT_Xmu_LIBRARY Xmu
      #      /usr/openwin/lib
      #      )
      
    ENDIF (APPLE)
    
  ENDIF (WIN32)

  SET( GLUT_FOUND "NO" )
  IF(GLUT_INCLUDE_DIR)
    IF(GLUT_glut_LIBRARY)
      # Is -lXi and -lXmu required on all platforms that have it?
      # If not, we need some way to figure out what platform we are on.
      SET( GLUT_LIBRARIES
	${GLUT_glut_LIBRARY}
	${GLUT_Xmu_LIBRARY}
	${GLUT_Xi_LIBRARY} 
	${GLUT_cocoa_LIBRARY}
	)
      SET( GLUT_FOUND "YES" )
      
      #The following deprecated settings are for backwards compatibility with CMake1.4
      SET (GLUT_LIBRARY ${GLUT_LIBRARIES})
      SET (GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIR})
      
    ENDIF(GLUT_glut_LIBRARY)
  ENDIF(GLUT_INCLUDE_DIR)

  MARK_AS_ADVANCED(
    GLUT_INCLUDE_DIR
    GLUT_glut_LIBRARY
    #  GLUT_Xmu_LIBRARY
    #  GLUT_Xi_LIBRARY
    )
endif ((NOT GLUT_INCLUDE_DIR) AND (NOT GLUT_LIBRARY))
