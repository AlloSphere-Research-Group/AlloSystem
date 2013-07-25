cmake_minimum_required (VERSION 2.8)
project (SimpleApp)

# Allocore Library
set(APP_SRC src/simpleApp.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

# Dependencies

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")
find_package(Portaudio REQUIRED)
message("-- Portaudio headers: ${PORTAUDIO_INCLUDE_DIRS}   ${PORTAUDIO_DEFINITIONS}")
find_package(LibSndFile REQUIRED)
message("-- libsndfile headers: ${SNDFILE_INCLUDE_DIRS}")

find_package(Allocore REQUIRED)
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

set(APP_INCLUDE_DIRS ${ALLOCORE_INCLUDE_DIR}
  ${GAMMA_INCLUDE_DIRS})

include_directories(${APP_INCLUDE_DIRS})

#System dependent libs
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
set(APP_LINK_LIBRARIES ${ALLOCORE_LIBRARIES} avahi-common avahi-client)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
add_definitions(-DAL_WINDOWS)
endif()

# TODO are avahi libraries needed on all platforms?
set(APP_LINK_LIBRARIES ${ALLOCORE_LIBRARY}
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
${SNDFILE_LIBRARIES}
${GAMMA_LIBRARIES}
${PA_FRAMEWORKS}
${LINUX_LIBS})

add_executable(${CMAKE_PROJECT_NAME} ${APP_SRC})
target_link_libraries(${CMAKE_PROJECT_NAME} ${APP_LINK_LIBRARIES})

