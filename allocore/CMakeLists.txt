cmake_minimum_required (VERSION 2.8)
project (Allocore)

message("==== Configuring allocore.")

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

set_target_properties(allocore 
  PROPERTIES 
  ALLOCORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  )
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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules")


find_package(Portaudio REQUIRED QUIET)
find_package(LibSndFile REQUIRED QUIET)
find_package(APR REQUIRED QUIET)
find_package(Assimp REQUIRED QUIET)
find_package(FreeImage REQUIRED QUIET)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED QUIET)
find_package(GLUT REQUIRED QUIET)
find_package(OpenGL REQUIRED QUIET)

if(NOT ALLOSYSTEM_BUILD)
  find_package(Gamma)
endif(NOT ALLOSYSTEM_BUILD)

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
  set(EXAMPLE_LINUX_LIBS asound)
  add_definitions(-DAL_LINUX)

  find_package(Threads)
  set(LINUX_LIBS ${LINUX_LIBS} avahi-common avahi-client ${CMAKE_THREAD_LIBS_INIT})
  # For HID
  find_package(Udev REQUIRED QUIET)
  include_directories(${UDEV_INCLUDE_DIR})
  find_package(Libusb REQUIRED QUIET)
  set(LINUX_LIBS ${LINUX_LIBS} ${LIBUSB_1_LIBRARY} ${UDEV_LIBRARY})
  #message("libudev lib = ${UDEV_LIBRARY}")
endif()

# ---- Windows ----
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions(-DAL_WINDOWS)
  # For HID
  set(WINDOWS_LIBS windows setupapi)
endif()

# Include directories
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

set_target_properties(allocore 
  PROPERTIES 
  ALLOCORE_LINK_LIBRARIES "${ALLOCORE_LINK_LIBRARIES}"
  )
target_link_libraries(allocore ${ALLOCORE_LINK_LIBRARIES})

# Doxygen documentation target
add_custom_target("doc"
	COMMAND "doxygen" "Doxyfile"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc"
	COMMENT "Generate doxygen documentation")

#--------------------------------------------------------------------------
# Examples
if(NOT NO_EXAMPLES)
  set(EXAMPLE_DIRS av graphics io math simulation spatial system)
  foreach(dir ${EXAMPLE_DIRS})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/${dir})
    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*.cpp)
    foreach(example ${EXAMPLE_FILES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt)
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt EXTRA_COMPILER_FLAGS)
      message("WARNING: Using additional flags from /examples/${dir}/flags.txt: " ${EXTRA_COMPILER_FLAGS})
    endif()
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      if (ALLOSYSTEM_BUILD)
	if ("${dir}" STREQUAL ".")
	  set(EXAMPLE_TARGET allocore_examples_${EXAMPLE_NAME})
	else()
	  set(EXAMPLE_TARGET allocore_examples_${dir}_${EXAMPLE_NAME})
	endif()
      else(ALLOSYSTEM_BUILD)
	set(EXAMPLE_TARGET examples_${dir}_${EXAMPLE_NAME})
      endif(ALLOSYSTEM_BUILD)
      add_executable(${EXAMPLE_TARGET} ${example})
      target_link_libraries("${EXAMPLE_TARGET}" allocore ${ALLOCORE_LINK_LIBRARIES})
      if (apple)
	find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries("${EXAMPLE_TARGET}" ${AUDIOUNIT_FM})
      endif()
      target_link_libraries("${EXAMPLE_TARGET}" ${EXAMPLE_LINUX_LIBS})
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example}
	COMMENT "Running: ${EXAMPLE_TARGET}")
    endforeach(example)
  endforeach(dir)

  if (GAMMA_FOUND OR GAMMA_LIBRARY)
    include_directories(${GAMMA_INCLUDE_DIR})
    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/sound/*.cpp)
    foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      add_executable(allocore_sound_${EXAMPLE_NAME} ${example})
      target_link_libraries(allocore_sound_${EXAMPLE_NAME} allocore ${ALLOCORE_LINK_LIBRARIES})
       if (apple)
        find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries(allocore_sound_${EXAMPLE_NAME} ${AUDIOUNIT_FM})
      endif()
      target_link_libraries(allocore_sound_${EXAMPLE_NAME} ${EXAMPLE_LINUX_LIBS} ${GAMMA_LIBRARIES})
    endforeach(example)
  else()
    message("Gamma not found. Not building allocore sound examples.")
  endif(GAMMA_FOUND OR GAMMA_LIBRARY)
endif(NOT NO_EXAMPLES)


# installation
install(DIRECTORY allocore/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/allocore)
install(TARGETS allocore DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
