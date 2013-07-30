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
src/io/al_HID.cpp
src/io/al_Serial.cpp
src/io/hidapi.c
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


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(ALLOCORE_SRC ${ALLOCORE_SRC}
 src/system/al_MainLoopOSX.mm
 src/system/al_InfoOSX.mm
 src/protocol/al_Zeroconf_OSX.mm)

endif()

file(GLOB ALLOCORE_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} allocore/*/*.hpp)
#file(GLOB ALLOCORE_HEADER_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} allocore/*)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(allocore ${ALLOCORE_SRC} ${ALLOCORE_HEADERS})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

include(TestBigEndian)
test_big_endian(TEST_ENDIAN)
if (TEST_ENDIAN MATCHES 0)
  add_definitions(-DOSC_HOST_LITTLE_ENDIAN)
else()
  add_definitions(-DOSC_HOST_BIG_ENDIAN)
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

set(ALLOCORE_INCLUDE_DIRS ${APR_INCLUDE_DIR}
  ${ASSIMP_INCLUDE_DIR}
  ${FREEIMAGE_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${LIBSNDFILE_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS}
  ${SNDFILE_INCLUDE_DIRS})

include_directories(${ALLOCORE_INCLUDE_DIRS})

# System dependent libs
# ---- OS X ----
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   add_definitions(-DAL_OSX)
# Needed by portaudio
   find_library(AUDIOUNIT_FM AudioUnit)
   find_library(COREAUDIO_FM CoreAudio)
   find_library(COREMIDI_FM CoreMidi)
   find_library(CORESERVICES_FM CoreServices)
   find_library(AUDIOTOOLBOX_FM AudioToolbox)
   set(MAC_LIBS ${MAC_LIBS} ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${COREMIDI_FM} ${CORESERVICES_FM} ${AUDIOTOOLBOX_FM})
# Needed by HID
   find_library(IOKIT_FM IOKit)
   find_library(COREFOUNDATION_FM CoreFoundation)
   set(MAC_LIBS ${MAC_LIBS} ${IOKIT_FM} ${COREFOUNDATION_FM})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


# ---- Linux ------
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(EXAMPLE_LINUX_LIBS asound pthread)
add_definitions(-DAL_LINUX)

set(LINUX_LIBS ${LINUX_LIBS} avahi-common avahi-client)
# For HID
find_package(Udev REQUIRED)
include_directories(${UDEV_INCLUDE_DIR})
find_package(Libusb REQUIRED)
set(LINUX_LIBS ${LINUX_LIBS} ${LIBUSB_1_LIBRARY} ${UDEV_LIBRARY})
message("libudev lib = ${UDEV_LIBRARY}")

endif()

# ---- Windows ----
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
add_definitions(-DAL_WINDOWS)
# For HID
set(WINDOWS_LIBS windows setupapi)
endif()

# Link libraries
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
${SNDFILE_LIBRARIES}
${MAC_LIBS}
${LINUX_LIBS}
${WINDOWS_LIBS})

target_link_libraries(allocore ${ALLOCORE_LINK_LIBRARIES})

#--------------------------------------------------------------------------
# Examples
if(NOT NO_EXAMPLES)
set(EXAMPLE_DIRS av graphics io math simulation spatial system)
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
        target_link_libraries(${dir}_${EXAMPLE_NAME} ${EXAMPLE_LINUX_LIBS})
  endforeach(example)
endforeach(dir)

if (GAMMA_FOUND)
file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/sound/*.cpp)
foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      add_executable(${dir}_${EXAMPLE_NAME} ${example})
      target_link_libraries(${dir}_${EXAMPLE_NAME} allocore ${ALLOCORE_LINK_LIBRARIES})
      include_directories(${GAMMA_INCLUDE_DIRS})
      if (apple)
        find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries(sound_${EXAMPLE_NAME} ${AUDIOUNIT_FM})
      endif()
      target_link_libraries(${dir}_${EXAMPLE_NAME} ${EXAMPLE_LINUX_LIBS} ${GAMMA_LIBRARIES})
endforeach(example)
else()
message("Gamma not found. Not building allocore sound examples.")
endif(GAMMA_FOUND)
endif(NOT NO_EXAMPLES)


# installation
install(DIRECTORY allocore/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/allocore)
install(TARGETS allocore DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
