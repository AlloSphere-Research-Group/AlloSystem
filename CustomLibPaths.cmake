
# Set the custom paths to your libraries here
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(LIB_PREFIX "${CMAKE_SOURCE_DIR}/external")


  set(PORTAUDIO_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(PORTAUDIO_LIBRARY "${LIB_PREFIX}/lib/portaudio.lib")

  set(SNDFILE_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(SNDFILE_LIBRARY "${LIB_PREFIX}/lib/libsndfile.lib")

  set(APR_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(APR_LIBRARY "${LIB_PREFIX}/lib/libapr.lib")

  set(ASSIMP_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(ASSIMP_LIBRARY "${LIB_PREFIX}/lib/libassimp.lib")

  set(FREEIMAGE_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(FREEIMAGE_LIBRARY "${LIB_PREFIX}/lib/freeimage.lib")

  set(FREETYPE_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(FREETYPE_LIBRARY "${LIB_PREFIX}/lib/freetype.lib")

  set(GLEW_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(GLEW_LIBRARY "${LIB_PREFIX}/lib/libglew32.lib")

  set(GLUT_INCLUDE_DIR "${LIB_PREFIX}/include")
  set(GLUT_LIBRARY "${LIB_PREFIX}/lib/glut.lib")

  #set(LUA_INCLUDE_DIR  "${LIB_PREFIX}/include")
  #set(LUA_LIBRARY "${LIB_PREFIX}/lib/lua.lib")

else()

  #set(PORTAUDIO_INCLUDE_DIR "${LIB_PREFIX}/include")
  #set(PORTAUDIO_LIBRARY "${LIB_PREFIX}/lib/portaudio.lib")

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")