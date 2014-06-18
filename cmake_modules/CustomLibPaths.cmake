
# Set the custom paths to your libraries here
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

  message("Using fixed paths for Windows dependencies.")
  if(NOT ${ALLOSYSTEM_DEP_PREFIX})
    set(ALLOSYSTEM_DEP_PREFIX "${CMAKE_SOURCE_DIR}/external")
  endif(NOT ${ALLOSYSTEM_DEP_PREFIX})

if(${CMAKE_GENERATOR} STREQUAL "MSYS Makefiles")
  set(PORTAUDIO_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(PORTAUDIO_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/libportaudio.dll.a")

  set(SNDFILE_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(SNDFILE_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/libsndfile.a")

  set(APR_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(APR_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/libapr-1.dll.a")

  add_definitions("-DUSE_ASSIMP3")
  set(ASSIMP_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(ASSIMP_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/assimp.lib")

  set(FREEIMAGE_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(FREEIMAGE_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/freeimage.lib")

  set(FREETYPE_INCLUDE_DIRS "${ALLOSYSTEM_DEP_PREFIX}/include/freetype2")
  set(FREETYPE_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/freetype.lib")

  set(GLEW_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(GLEW_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/libglew32.dll.a")

  set(GLUT_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(GLUT_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/glut.lib")

  set(LUA_INCLUDE_DIR  "${ALLOSYSTEM_DEP_PREFIX}/include")
  set(LUA_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/lib/lua.lib")

elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 9 2008")
  set(PORTAUDIO_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(PORTAUDIO_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/portaudio.lib")

  set(SNDFILE_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(SNDFILE_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/libsndfile.lib")

  set(APR_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(APR_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/libapr.lib")

  set(ASSIMP_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(ASSIMP_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/libassimp.lib")

  set(FREEIMAGE_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(FREEIMAGE_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/freeimage.lib")

  set(FREETYPE_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(FREETYPE_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/freetype.lib")

  set(GLEW_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(GLEW_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/libglew32.lib")

  set(GLUT_INCLUDE_DIR "${ALLOSYSTEM_DEP_PREFIX}/ivs/nclude")
  set(GLUT_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/glut.lib")

  set(LUA_INCLUDE_DIR  "${ALLOSYSTEM_DEP_PREFIX}/vs/include")
  set(LUA_LIBRARY "${ALLOSYSTEM_DEP_PREFIX}/vs/lib/lua.lib")

else()
  message("FATAL_ERROR: Unsupported CMake generator for AlloSystem.")
  message(FATAL_ERROR "Use 'Visual Studio 9 2008' or 'MSYS Makefiles'")
endif()

else() # Not Windows

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
