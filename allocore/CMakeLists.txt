cmake_minimum_required (VERSION 2.8)
project (Allocore)

message(STATUS "==== Configuring allocore.")

# Allocore Library
list(APPEND ALLOCORE_SRC
  src/io/al_AudioIOData.cpp
  src/io/al_ControlNav.cpp
  src/io/al_File.cpp
  src/io/al_MIDI.cpp
  src/io/al_MRC.cpp
  src/io/al_HID.cpp
  src/io/al_Serial.cpp
  src/io/al_CSVReader.cpp
  src/io/hidapi.c
  src/protocol/al_Serialize.cpp
  src/spatial/al_HashSpace.cpp
  src/spatial/al_Pose.cpp
  src/system/al_Info.cpp
  src/system/al_PeriodicThread.cpp
  src/system/al_Printing.cpp
  src/system/al_Time.cpp
  src/system/al_Watcher.cpp
  src/types/al_Array.cpp
  src/types/al_Array_C.c
  src/types/al_Color.cpp
  src/types/al_MsgQueue.cpp
)

list(APPEND ALLOCORE_HEADERS
    allocore/al_Allocore.hpp
    allocore/graphics/al_Mesh.hpp
    allocore/graphics/al_MeshVBO.hpp
    allocore/graphics/al_Shapes.hpp
    allocore/graphics/al_Image.hpp
    allocore/graphics/al_EasyFBO.hpp
    allocore/io/al_AudioIOData.hpp
    allocore/io/al_File.hpp
    allocore/io/al_HID.hpp
    allocore/io/al_MIDI.hpp
    allocore/io/al_MRC.hpp
    allocore/io/al_Serial.hpp
    allocore/io/al_CSVReader.hpp
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
    allocore/system/al_Time.h
    allocore/system/al_Time.hpp
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
    allocore/ui/al_Gnomon.hpp
    allocore/ui/al_BoundingBox.hpp
    allocore/ui/al_Pickable.hpp
    allocore/ui/al_TranslateHandle.hpp
    allocore/ui/al_RotateHandle.hpp
    allocore/ui/al_SynthSequencer.hpp
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
set(ALLOCORE_DEP_INCLUDE_DIRS ${BUILD_ROOT_DIR}/build/include)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(NOT (COMPILER_SUPPORTS_CXX11 OR COMPILER_SUPPORTS_CXX0X))
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ")
endif()

# System dependent libs
# ---- OS X ----
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Needed by HID
  find_library(IOKIT_FM IOKit)
  find_library(COREFOUNDATION_FM CoreFoundation)
  list(APPEND ALLOCORE_LINK_LIBRARIES ${IOKIT_FM} ${COREFOUNDATION_FM})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# ---- Linux ------
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  # For clock_gettime, etc.
  list(APPEND ALLOCORE_LINK_LIBRARIES rt)

  # For HID
  find_package(Udev QUIET)
  include_directories(${UDEV_INCLUDE_DIR})
  find_package(Libusb QUIET)
  list(APPEND ALLOCORE_LINK_LIBRARIES ${LIBUSB_1_LIBRARY} ${UDEV_LIBRARY})
  #message("libudev lib = ${UDEV_LIBRARY}")
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


# Travis CI configuration

if(TRAVIS_BUILD)
add_definitions(-DTRAVIS_BUILD)
remove_definitions(-DAL_AUDIO_PORTAUDIO)
add_definitions(-DAL_AUDIO_DUMMY)
endif(TRAVIS_BUILD)

# allocore library -----------------------

set(ALLOCORE_LIB "allocore${DEBUG_SUFFIX}")

set(LIBRARY_OUTPUT_PATH ${BUILD_ROOT_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_ROOT_DIR}/build/bin)

# Add headers to the list of source file so they appear in generated projects.
add_library(${ALLOCORE_LIB} ${ALLOCORE_SRC} ${ALLOCORE_HEADERS})

# Copy headers to build directory (only if header has changed)
foreach(header ${ALLOCORE_HEADERS})
    execute_process(COMMAND ${CMAKE_COMMAND} -E
					copy_if_different "${header}" "${BUILD_ROOT_DIR}/build/include/${header}"
					WORKING_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

write_dummy_headers( "${ALLOCORE_DUMMY_HEADERS}" "${ALLOCORE_HEADERS}" )

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

#message(${ALLOCORE_DEP_INCLUDE_DIRS})

# Doxygen documentation target
add_custom_target("doc"
	COMMAND "doxygen" "Doxyfile"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc"
	COMMENT "Generate doxygen documentation")

#--------------------------------------------------------------------------
# Examples targets
if(BUILD_EXAMPLES)
  set(EXAMPLE_DIRS av graphics io math simulation spatial system ui)
  foreach(dir ${EXAMPLE_DIRS})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_ROOT_DIR}/build/bin/examples/${dir})
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
#if(COMPILER_SUPPORTS_CXX11)
#	set_property(TARGET ${EXAMPLE_TARGET} APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++11 ")
#elseif(COMPILER_SUPPORTS_CXX0X)
#	set_property(TARGET ${EXAMPLE_TARGET} APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++0x")
#endif()
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

  if (GAMMA_FOUND OR GAMMA_LIBRARY AND NOT TRAVIS_BUILD)
    get_target_property(GAMMA_LIBRARIES Gamma GAMMA_LINK_LIBRARIES)
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
      target_link_libraries(allocore_sound_${EXAMPLE_NAME} ${GAMMA_LIBRARIES} ${GAMMA_LIBRARY})
    endforeach(example)
  else()
    message("Gamma not found. Not building allocore sound examples.")
  endif(GAMMA_FOUND OR GAMMA_LIBRARY AND NOT TRAVIS_BUILD)
endif(BUILD_EXAMPLES)

# Unit tests

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD_ROOT_DIR}/build/bin") # Put back after the change for examples
add_subdirectory(unitTests)

# installation
install(FILES ${ALLOCORE_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)
install(TARGETS ${ALLOCORE_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
