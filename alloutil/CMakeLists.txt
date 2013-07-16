cmake_minimum_required (VERSION 2.8)
project (Alloutil)

set(ALLOUTIL_BUILD_EXAMPLES 1)

set(ALLOUTIL_SRC
  src/al_FileWatcher.cpp
  src/al_OmniStereo.cpp
  src/al_ResourceManager.cpp
  src/al_WarpBlend.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(alloutil ${ALLOUTIL_SRC})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../build/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")

find_package(Allocore REQUIRED)
find_package(Lua51 REQUIRED)

include_directories(${LUA_INCLUDE_DIR} ${ALLOCORE_INCLUDE_DIR})
set(ALLOUTIL_LINK_LIBRARIES ${LUA_LIBRARIES} ${ALLOCORE_LIBRARY})
target_link_libraries(alloutil ${ALLOUTIL_LINK_LIBRARIES})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Examples
if(ALLOUTIL_BUILD_EXAMPLES)
find_package(APR REQUIRED)
find_package(Assimp REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(LibSndFile REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Portaudio REQUIRED)
find_package(Gamma REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# Needed by portaudio
add_definitions(-DAL_OSX)
   find_library(AUDIOUNIT_FM AudioUnit)
   find_library(COREAUDIO_FM CoreAudio)
   find_library(COREMIDI_FM CoreMidi)
   find_library(CORESERVICES_FM CoreServices)
   find_library(AUDIOTOOLBOX_FM AudioToolbox)
   set(PA_FRAMEWORKS ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${COREMIDI_FM} ${CORESERVICES_FM} ${AUDIOTOOLBOX_FM})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(EXAMPLE_LINUX_LIBS asound pthread)

add_definitions(-DAL_LINUX)
set(ALLOCORE_LINK_LIBRARIES ${ALLOCORE_LIBRARIES} avahi-common avahi-client)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
add_definitions(-DAL_WINDOWS)
endif()

set(ALLOCORE_LINK_LIBRARIES ${ALLOCORE_LINK_LIBRARIES}
${APR_LIBRARY}
${ASSIMP_LIBRARY}
${FREEIMAGE_LIBRARY}
${FREETYPE_LIBRARY}
${GLEW_LIBRARY}
${GLUT_LIBRARY}
${LIBSNDFILE_LIBRARY}
${OPENGL_LIBRARY}
${PORTAUDIO_LIBRARY}
${PORTAUDIO_LIBRARIES}
${GAMMA_LIBRARIES}
${SNDFILE_LIBRARIES}
${PA_FRAMEWORKS}
${LINUX_LIBS}
${EXAMPLE_LINUX_LIBS})

file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
foreach(example ${EXAMPLE_FILES})
get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
add_executable(alloutil_${EXAMPLE_NAME} ${example})
target_link_libraries(alloutil_${EXAMPLE_NAME} alloutil ${ALLOUTIL_LINK_LIBRARIES} ${ALLOCORE_LINK_LIBRARIES})
endforeach(example)
endif(ALLOUTIL_BUILD_EXAMPLES)


# installation
install(DIRECTORY alloutil/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/alloutil)
install(TARGETS alloutil DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)


