cmake_minimum_required (VERSION 2.8)
project (Allocore)

message(STATUS "==== Configuring allocore.")

# Allocore Library
list(APPEND ALLOCORE_SRC
  src/io/al_MIDI.cpp
  src/io/al_HID.cpp
  src/io/al_Serial.cpp
  src/io/hidapi.c
  src/protocol/al_Serialize.cpp
  src/spatial/al_HashSpace.cpp
  src/spatial/al_Pose.cpp
  src/system/al_Info.cpp
  src/system/al_Printing.cpp
  src/system/al_Watcher.cpp
  src/types/al_Array.cpp
  src/types/al_Array_C.c
  src/types/al_Color.cpp
  src/types/al_MsgQueue.cpp)

list(APPEND ALLOCORE_HEADERS
    allocore/al_Allocore.hpp
    allocore/graphics/al_Mesh.hpp
    allocore/graphics/al_Shapes.hpp
    allocore/io/al_HID.hpp
    allocore/io/al_MIDI.hpp
    allocore/io/al_Serial.hpp
    allocore/math/al_Analysis.hpp
    allocore/math/al_Complex.hpp
    allocore/math/al_Constants.hpp
    allocore/math/al_Frustum.hpp
    allocore/math/al_Functions.hpp
    allocore/math/al_Interpolation.hpp
    allocore/math/al_Interval.hpp
    allocore/math/al_Mat.hpp
    allocore/math/al_Matrix4.hpp
    allocore/math/al_Plane.hpp
    allocore/math/al_Quat.hpp
    allocore/math/al_Random.hpp
    allocore/math/al_Ray.hpp
    allocore/math/al_Spherical.hpp
    allocore/math/al_Vec.hpp
    allocore/protocol/al_Serialize.h
    allocore/protocol/al_Serialize.hpp
    allocore/spatial/al_Curve.hpp
    allocore/spatial/al_DistAtten.hpp
    allocore/spatial/al_HashSpace.hpp
    allocore/spatial/al_Pose.hpp
    allocore/system/al_Config.h
    allocore/system/al_Info.hpp
    allocore/system/al_PeriodicThread.hpp
    allocore/system/al_Printing.hpp
    allocore/system/al_Thread.hpp
    allocore/system/al_Watcher.hpp
    allocore/system/pstdint.h
    allocore/types/al_Array.h
    allocore/types/al_Array.hpp
    allocore/types/al_Buffer.hpp
    allocore/types/al_Color.hpp
    allocore/types/al_Conversion.hpp
    allocore/types/al_MsgQueue.hpp
    allocore/types/al_MsgTube.hpp
    allocore/types/al_SingleRWRingBuffer.hpp
)

set(ALLOCORE_DUMMY_HEADERS "")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND ALLOCORE_SRC
    src/system/al_InfoOSX.mm
)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package(Threads QUIET)
  if(CMAKE_THREAD_LIBS_INIT)
  list(APPEND ALLOCORE_SRC
    src/system/al_ThreadNative.cpp
)
  else()
    message("NOT building native thread Library (pthreads not found).")
  endif()
else()
# Windows and OS X come with threading libraries installed.
  list(APPEND ALLOCORE_SRC
    src/system/al_ThreadNative.cpp
)
endif()

# Dependencies --------------------------------------------
set(ALLOCORE_DEP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/build/include)

# System dependent libs
# ---- OS X ----
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DAL_OSX)
    # Needed by HID
  find_library(IOKIT_FM IOKit)
  find_library(COREFOUNDATION_FM CoreFoundation)
  list(APPEND ALLOCORE_LINK_LIBRARIES ${IOKIT_FM} ${COREFOUNDATION_FM})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# ---- Linux ------
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DAL_LINUX)

  # For clock_gettime, etc.
  list(APPEND ALLOCORE_LINK_LIBRARIES rt)

  # For HID
  find_package(Udev QUIET)
  include_directories(${UDEV_INCLUDE_DIR})
  find_package(Libusb QUIET)
  list(APPEND ALLOCORE_LINK_LIBRARIES ${LIBUSB_1_LIBRARY} ${UDEV_LIBRARY})
  #message("libudev lib = ${UDEV_LIBRARY}")
endif()

# ---- Windows ----
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions(-DAL_WINDOWS)
  # For HID
  #list(APPEND ALLOCORE_LINK_LIBRARIES windows setupapi) # Commented out for mingw buildmaybe required for msvs?
endif()

# optional dependendencies ----------------------------
include(modules/gl_module.cmake)
include(modules/glut_module.cmake)
# Font (freetype), freeimage and assimp modules depend on the gl module
include(modules/font_module.cmake)
include(modules/freeimage_module.cmake)
include(modules/assimp_module.cmake)

include(modules/apr_module.cmake)
include(modules/portaudio_module.cmake)
include(modules/libsndfile_module.cmake)

include(modules/osc_module.cmake)
include(modules/zeroconf_module.cmake)

# allocore library -----------------------

set(ALLOCORE_LIB "allocore${DEBUG_SUFFIX}")

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Add headers to the list of source file so they appear in generated projects.
add_library(${ALLOCORE_LIB} ${ALLOCORE_SRC} ${ALLOCORE_HEADERS})

# Copy headers to build directory (only if directory does not exist)
foreach(header ${ALLOCORE_HEADERS})
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/build/include/${header}") # Don't copy if exists, otherwise the copy forces a full rebuild.
        execute_process(COMMAND ${CMAKE_COMMAND} -E
                        copy ${header} ${CMAKE_SOURCE_DIR}/build/include/${header}
                        WORKING_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endforeach()

# Write dummy headers

foreach(HEADER_INFO ${ALLOCORE_DUMMY_HEADERS})
    string(REPLACE "::::" ";" HEADER_PARTS ${HEADER_INFO})
    list(GET HEADER_PARTS 0 header) # First element is header
    list(GET HEADER_PARTS 1 dependency) # Second element is dependency

    set(DUMMY_HEADER_TEXT "#error \"ERROR: Header not supported. Dependency '${dependency}' not met.\"\n")
    file(WRITE "${CMAKE_SOURCE_DIR}/build/include/${header}" ${DUMMY_HEADER_TEXT})
    message("Writing dummy header: ${header} ---- Unmet dependency: ${dependency}")
    list(APPEND ALLOCORE_HEADERS "${CMAKE_SOURCE_DIR}/build/include/${header}")
endforeach()

# Export include directories and libraries for use when building AlloSystem or AlloProject
set_target_properties(${ALLOCORE_LIB}
  PROPERTIES
  ALLOCORE_DEP_INCLUDE_DIRS "${ALLOCORE_DEP_INCLUDE_DIRS}"
  )
set_target_properties(${ALLOCORE_LIB}
  PROPERTIES 
  ALLOCORE_LINK_LIBRARIES "${ALLOCORE_LINK_LIBRARIES}"
  )

include_directories(${ALLOCORE_DEP_INCLUDE_DIRS})
target_link_libraries(${ALLOCORE_LIB} ${ALLOCORE_LINK_LIBRARIES})


# Doxygen documentation target
add_custom_target("doc"
	COMMAND "doxygen" "Doxyfile"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc"
	COMMENT "Generate doxygen documentation")

#--------------------------------------------------------------------------
# Examples targets
if(BUILD_EXAMPLES)
  set(EXAMPLE_DIRS av graphics io math simulation spatial system)
  foreach(dir ${EXAMPLE_DIRS})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/${dir})
    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*.cpp)
    foreach(example ${EXAMPLE_FILES})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt)
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt EXTRA_COMPILER_FLAGS)
      message(STATUS "NOTE: Using additional flags from /examples/${dir}/flags.txt: " ${EXTRA_COMPILER_FLAGS})
      STRING(REGEX REPLACE "[\r\n]" " " EXTRA_COMPILER_FLAGS "${EXTRA_COMPILER_FLAGS}")
    endif()
    get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
    if("${dir}" STREQUAL ".")
      set(EXAMPLE_TARGET allocore_examples_${EXAMPLE_NAME})
    else()
      set(EXAMPLE_TARGET allocore_examples_${dir}_${EXAMPLE_NAME})
    endif()
      add_executable(${EXAMPLE_TARGET} EXCLUDE_FROM_ALL ${example})
      set_target_properties(${EXAMPLE_TARGET} PROPERTIES
	COMPILE_FLAGS "${EXTRA_COMPILER_FLAGS}")
      target_link_libraries("${EXAMPLE_TARGET}" ${ALLOCORE_LIB} ${ALLOCORE_LINK_LIBRARIES})
      if (apple)
	find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries("${EXAMPLE_TARGET}" ${AUDIOUNIT_FM})
      endif()
      target_link_libraries("${EXAMPLE_TARGET}")
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example}
	COMMENT "Running: ${EXAMPLE_TARGET}")
    endforeach(example)
  endforeach(dir)

  if(NOT TARGET Gamma)
    find_package(Gamma)
  endif(NOT TARGET Gamma)

  if (GAMMA_FOUND OR GAMMA_LIBRARY)
    include_directories(${GAMMA_INCLUDE_DIR})
    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/sound/*.cpp)
    foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      add_executable(allocore_sound_${EXAMPLE_NAME} ${example})
      target_link_libraries(allocore_sound_${EXAMPLE_NAME} ${ALLOCORE_LIB} ${ALLOCORE_LINK_LIBRARIES})
       if (apple)
        find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries(allocore_sound_${EXAMPLE_NAME} ${AUDIOUNIT_FM})
      endif()
      target_link_libraries(allocore_sound_${EXAMPLE_NAME} ${GAMMA_LIBRARIES})
    endforeach(example)
  else()
    message("Gamma not found. Not building allocore sound examples.")
  endif(GAMMA_FOUND OR GAMMA_LIBRARY)
endif(BUILD_EXAMPLES)

# installation
install(FILES ${ALLOCORE_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)
install(TARGETS ${ALLOCORE_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
