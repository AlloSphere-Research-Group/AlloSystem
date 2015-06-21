cmake_minimum_required (VERSION 2.8)
project (Alloaudio)

message(STATUS "==== Configuring alloaudio.")


find_package(LibSndFile REQUIRED QUIET)
if(NOT TARGET Gamma)
    find_package(Gamma QUIET)
endif(NOT TARGET Gamma)

if(NOT (GAMMA_FOUND OR GAMMA_LIBRARY))
    message("Alloaudio: Gamma not found. Not building Alloaudio." )
    return()
endif(NOT (GAMMA_FOUND OR GAMMA_LIBRARY))

set(ALLOAUDIO_SRC
  src/al_OutputMaster.cpp
  src/butter.cpp
  src/al_Convolver.cpp
  src/zita-convolver-3.1.0/libs/zita-convolver.cc
#  src/al_Decorrelation.cpp
  )

set(ALLOAUDIO_HEADERS
  alloaudio/al_OutputMaster.hpp
  alloaudio/al_Convolver.hpp
  src/zita-convolver-3.1.0/libs/zita-convolver.h
)

# Dependencies ------------------------------

get_target_property(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore${DEBUG_SUFFIX} ALLOCORE_LINK_LIBRARIES)

# FIXME check if this works on Windows, as pthreads are currently specifically required
if($CMAKE_SYSTEM_NAME} MATCHES "Linux")
find_package(Threads QUIET)
if(CMAKE_THREAD_LIBS_INIT)

else()
  message("phtreads not found. Not building Alloaudio.")
return()

endif()
endif($CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_library(FFTW_LIBRARY
     NAMES fftw3f)

message("Using fftw3: ${FFTW_LIBRARY}")

# Build library ----------------------------------

file(GLOB ALLOAUDIO_INSTALL_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} alloaudio/*.h*)
set(ALLOAUDIO_LIBRARY "alloaudio${DEBUG_SUFFIX}")
add_library(${ALLOAUDIO_LIBRARY} ${ALLOAUDIO_SRC} ${ALLOAUDIO_INSTALL_HEADERS})

set(ALLOAUDIO_LINK_LIBRARIES ${ALLOCORE_LIBRARY} ${FFTW_LIBRARY})
set(ALLOAUDIO_DIRS ${BUILD_ROOT_DIR}/build/include)

# export properties for AlloSystem
set_target_properties(${ALLOAUDIO_LIBRARY}
  PROPERTIES
  ALLOAUDIO_INCLUDE_DIR "${ALLOAUDIO_DIRS}"
  )
set_target_properties(${ALLOAUDIO_LIBRARY}
  PROPERTIES
  ALLOAUDIO_LINK_LIBRARIES "${ALLOAUDIO_LINK_LIBRARIES}"
  )

include_directories(${ALLOAUDIO_DIRS} ${ALLOCORE_DEP_INCLUDE_DIRS}
       ${CMAKE_CURRENT_SOURCE_DIR}/src/zita-convolver-3.1.0/libs/)
target_link_libraries(${ALLOAUDIO_LIBRARY} ${ALLOAUDIO_LINK_LIBRARIES})

add_dependencies(${ALLOAUDIO_LIBRARY} ${ALLOCORE_LIBRARY})

set(LIBRARY_OUTPUT_PATH ${BUILD_ROOT_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_ROOT_DIR}/build/bin)

# Copy headers to build directory (only if header has changed)
foreach(header ${ALLOAUDIO_INSTALL_HEADERS})
	execute_process(COMMAND ${CMAKE_COMMAND} -E
					copy_if_different "${header}" "${BUILD_ROOT_DIR}/build/include/${header}"
					WORKING_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

#message (STATUS "allocore_location for alloaudio == ${ALLOCORE_LIBRARY}")

# Build Examples ------------------------------------------------------
if(BUILD_EXAMPLES)
    include_directories(${GAMMA_INCLUDE_DIR})

    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_ROOT_DIR}/build/bin/examples/)
    foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      set(EXAMPLE_TARGET alloaudio_examples_${EXAMPLE_NAME})
      add_executable(${EXAMPLE_TARGET} ${example})
#      add_dependencies(${EXAMPLE_TARGET} ${ALLOAUDIO_LIBRARY})
      target_link_libraries("${EXAMPLE_TARGET}" ${ALLOAUDIO_LIBRARY} ${ALLOAUDIO_LINK_LIBRARIES})
      if(APPLE)
	set_target_properties(${EXAMPLE_TARGET} PROPERTIES
	  LINK_FLAGS "-pagezero_size 10000 -image_base 100000000")
      endif()
      if (EXTRA_COMPILER_FLAGS)
# This is the right way, but it crashes cmake... Bug has been fixed by cmake devs but not released as of this writing...
	#set_source_files_properties(${example_src}
        #  PROPERTIES COMPILE_FLAGS "-I/usr/include/mysql")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILER_FLAGS}")
      endif(EXTRA_COMPILER_FLAGS)
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example})
    endforeach(example)
endif(BUILD_EXAMPLES)

# installation
install(FILES ${ALLOAUDIO_INSTALL_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(TARGETS ${ALLOAUDIO_LIBRARY} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

# Tests that depend on cunit
find_library(CUNIT_LIBRARY cunit)
set(TEST_ARGS "")

if(CUNIT_LIBRARY)
	#ENABLE_TESTING()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD_ROOT_DIR}/build/bin")
    add_executable(alloaudioTests unitTests/alloaudioTests.cpp)
    target_link_libraries(alloaudioTests ${ALLOCORE_LIBRARY} ${ALLOAUDIO_LIBRARY} ${ALLOCORE_LINK_LIBRARIES} ${CUNIT_LIBRARY})
    add_dependencies(alloaudioTests ${ALLOCORE_LIBRARY})
    add_test(NAME alloaudioTests
            COMMAND $<TARGET_FILE:alloaudioTests> ${TEST_ARGS})

    add_executable(convolverTests unitTests/convolverTests.cpp)
    target_link_libraries(convolverTests ${ALLOAUDIO_LIBRARY} ${ALLOCORE_LIBRARY} ${ALLOCORE_LINK_LIBRARIES} ${CUNIT_LIBRARY} ${FFTW_LIBRARY} )
    add_dependencies(convolverTests ${ALLOCORE_LIBRARY})
    add_test(NAME convolverTests
            COMMAND $<TARGET_FILE:convolverTests> ${TEST_ARGS})
endif(CUNIT_LIBRARY)
