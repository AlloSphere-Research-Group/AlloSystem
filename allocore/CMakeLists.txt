cmake_minimum_required (VERSION 2.8)
project (Allocore)

# Allocore Library
set(ALLOCORE_SRC src/graphics/al_Asset.cpp
src/graphics/al_FBO.cpp
src/graphics/al_GPUObject.cpp
src/graphics/al_Graphics.cpp
src/graphics/al_Image.cpp
src/graphics/al_Isosurface.cpp
src/graphics/al_Lens.cpp
src/graphics/al_Light.cpp
src/graphics/al_Mesh.cpp
src/graphics/al_Shader.cpp
src/graphics/al_Shapes.cpp
src/graphics/al_Stereographic.cpp
src/graphics/al_Texture.cpp
src/graphics/al_Font.cpp
src/io/al_App.cpp
src/io/al_AudioIO.cpp
src/io/al_File.cpp
src/io/al_MIDI.cpp
src/io/al_Window.cpp
src/io/al_FileAPR.cpp
src/io/al_SocketAPR.cpp
src/io/al_WindowGLUT.cpp
src/protocol/oscpack/osc/OscOutboundPacketStream.cpp
src/protocol/oscpack/osc/OscPrintReceivedElements.cpp
src/protocol/oscpack/osc/OscReceivedElements.cpp
src/protocol/oscpack/osc/OscTypes.cpp
src/protocol/al_OSC.cpp
src/protocol/al_XML.cpp
src/protocol/al_Serialize.cpp
src/protocol/al_Zeroconf.cpp
src/sound/al_Ambisonics.cpp
src/spatial/al_HashSpace.cpp
src/spatial/al_Pose.cpp
src/system/al_Info.cpp
src/system/al_MainLoop.cpp
src/system/al_Printing.cpp
src/system/al_ThreadNative.cpp
src/system/al_Time.cpp
src/system/al_Watcher.cpp
src/types/al_Array.cpp
src/types/al_Color.cpp
src/types/al_MsgQueue.cpp)

include_directories(${CMAKE_SOURCE_DIR})
add_library(allocore ${ALLOCORE_SRC})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../build/bin)

# FIXME check for endianness!
include(TestBigEndian)
test_big_endian(TEST_ENDIAN)
if (TEST_ENDIAN MATCHES 0)
  add_definitions(-DOSC_HOST_LITTLE_ENDIAN)
  #add_definitions(-D__LITTLE_ENDIAN__)
else()
  add_definitions(-DOSC_HOST_BIG_ENDIAN)
  #add_definitions(-D__BIG_ENDIAN__)
endif()

# Dependencies

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")
find_package(Portaudio REQUIRED)
message("-- Portaudio headers: ${PORTAUDIO_INCLUDE_DIRS}   ${PORTAUDIO_DEFINITIONS}")
find_package(LibSndFile REQUIRED)
message("-- libsndfile headers: ${SNDFILE_INCLUDE_DIRS}")

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

include_directories(${APR_INCLUDE_DIR}
  ${ASSIMP_INCLUDE_DIR}
  ${FREEIMAGE_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${LIBSNDFILE_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS}
  ${SNDFILE_INCLUDE_DIRS}
  ${GAMMA_INCLUDE_DIRS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# Needed by portaudio
   find_library(AUDIOUNIT_FM AudioUnit)
   find_library(COREAUDIO_FM CoreAudio)
   find_library(CORESERVICES_FM CoreServices)
   find_library(AUDIOTOOLBOX_FM AudioToolbox)
   set(PA_FRAMEWORKS ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${CORESERVICES_FM} ${AUDIOTOOLBOX_FM})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# TODO are avahi libraries needed on all platforms?
set(ALLOCORE_LINK_LIBRARIES
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
avahi-common avahi-client
${PA_FRAMEWORKS})

target_link_libraries(allocore ${ALLOCORE_LINK_LIBRARIES})

message(${GAMMA_LIBRARIES})

# Examples
set(EXAMPLE_DIRS algorithmic curves effects filter function generator io spectral)
foreach(dir ${EXAMPLE_DIRS})
  file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*.cpp)
  foreach(example ${EXAMPLE_FILES})
	get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
	add_executable(${dir}_${EXAMPLE_NAME} ${example})
	target_link_libraries(${dir}_${EXAMPLE_NAME} allocore ${ALLOCORE_LINK_LIBRARIES})
	if (apple)
	  find_library(AUDIOUNIT_FM AudioUnit)
	  target_link_libraries(${dir}_${EXAMPLE_NAME} ${AUDIOUNIT_FM})
	endif()
  endforeach(example)
endforeach(dir)


# Unit tests
# TODO make optional
#add_executable(unitTests tests/unitTests.cpp)
#target_link_libraries(unitTests Gamma portaudio sndfile ${PA_FRAMEWORKS})
