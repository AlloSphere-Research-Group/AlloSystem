# - Try to find Assimp
# Once done this will define
#
#  ASSIMP_FOUND - system has Assimp
#  ASSIMP_INCLUDE_DIR - the Assimp include directory
#  ASSIMP_LIBRARIES - Link these to use Assimp
#

SET(ASSIMP "assimp")

FIND_PATH(ASSIMP_INCLUDE_DIR
  NAMES
    assimp.h
  PATHS
    /usr/include/assimp
    /usr/local/include
    /opt/local/include/assimp
    /usr/local/Cellar/assimp/2.0.863/include/assimp
)
 

FIND_LIBRARY(LIBASSIMP
  NAMES 
    ${ASSIMP}
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
)

SET (ASSIMP_LIBRARY
  ${LIBASSIMP} 
)

IF(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)
   SET(ASSIMP_FOUND TRUE)
ENDIF(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)

# show the COLLADA_DOM_INCLUDE_DIR and COLLADA_DOM_LIBRARIES variables only in the advanced view
IF(ASSIMP_FOUND)
  MARK_AS_ADVANCED(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARIES )
ENDIF(ASSIMP_FOUND)


