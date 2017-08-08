project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

# For unit testing
ENABLE_TESTING() # Must be here to have tests for the subprojects
find_program( MEMORYCHECK_COMMAND valgrind )
set( MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck --trace-children=yes --leak-check=full" )
set( MEMORYCHECK_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/valgrind_suppress.txt" )

# ------ Helper functions

function(add_memcheck_test TEST_NAME)
  add_test("memcheck_${TEST_NAME}" ${MEMORYCHECK_COMMAND} "${BUILD_ROOT_DIR}/build/bin/${TEST_NAME}" ${MEMORYCHECK_COMMAND_OPTIONS} "--suppressions=${MEMORYCHECK_SUPPRESSIONS_FILE}")
endfunction(add_memcheck_test)

# Write dummy headers
function(write_dummy_headers DUMMY_HEADERS HEADERS)
foreach(HEADER_INFO ${DUMMY_HEADERS})
	string(REPLACE "::::" ";" HEADER_PARTS ${HEADER_INFO})
	list(GET HEADER_PARTS 0 header) # First element is header
	list(GET HEADER_PARTS 1 dependency) # Second element is dependency

	set(DUMMY_HEADER_TEXT "#error \"ERROR: Header not supported. Dependency '${dependency}' not met.\"\n")
	file(WRITE "${BUILD_ROOT_DIR}/build/include/${header}" ${DUMMY_HEADER_TEXT})
	message("Writing dummy header: ${header} ---- Unmet dependency: ${dependency}")
	list(APPEND HEADERS "${BUILD_ROOT_DIR}/build/include/${header}")
endforeach()
endfunction()

# If root directory has not been specified, use default
if(NOT DEFINED BUILD_ROOT_DIR)
  set(BUILD_ROOT_DIR "${CMAKE_SOURCE_DIR}")
endif(NOT DEFINED BUILD_ROOT_DIR)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DEBUG_SUFFIX "_debug")
  message(STATUS "====== Building Debug version")
  if((CMAKE_COMPILER_IS_GNUCC) OR (CMAKE_COMPILER_IS_GNUXX))
    message("Using gcc debug flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
  endif((CMAKE_COMPILER_IS_GNUCC) OR (CMAKE_COMPILER_IS_GNUXX))
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# Other defaults

if(NOT DEFINED BUILD_GAMMA)
  set(BUILD_GAMMA 1 CACHE STRING "Try to build external Gamma")
endif(NOT DEFINED BUILD_GAMMA)

if(NOT DEFINED BUILD_GLV)
  set(BUILD_GLV 1 CACHE STRING "Try to build external GLV.")
endif(NOT DEFINED BUILD_GLV)

if(NOT DEFINED BUILD_CUTTLEBONE)
  set(BUILD_CUTTLEBONE 1 CACHE STRING "Try to build external Cuttlebone.")
endif(NOT DEFINED BUILD_CUTTLEBONE)

if(NOT DEFINED BUILD_PHASESPACE)
  set(BUILD_PHASESPACE 1 CACHE STRING "Try to build external phasespace.")
endif(NOT DEFINED BUILD_PHASESPACE)

option(BUILD_ALLOCV "Build the AlloCV module (deprecated)")

set(BUILD_EXAMPLES 0 CACHE STRING "Build AlloSystem examples.")


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DAL_OSX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DAL_LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_definitions(-DAL_WINDOWS)
endif()


# External dependencies (Gamma and GLV)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)


if(NOT (COMPILER_SUPPORTS_CXX11 OR COMPILER_SUPPORTS_CXX0X))
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

include(cmake_modules/make_dep.cmake) # Functions to build some dependencies like Gamma and GLV

if(BUILD_GAMMA AND NOT GAMMA_LIBRARY)
    make_dep(Gamma)
	#set(GAMMA_LIBRARY ${Gamma_LIBRARY})
	#include_directories(${Gamma_INCLUDE_DIR})
endif(BUILD_GAMMA AND NOT GAMMA_LIBRARY)

if(BUILD_GLV AND NOT GLV_LIBRARY)
    make_dep(GLV)
endif(BUILD_GLV AND NOT GLV_LIBRARY)

if(BUILD_CUTTLEBONE AND NOT CUTTLEBONE_LIBRARY)
    make_dep(cuttlebone)
endif(BUILD_CUTTLEBONE AND NOT CUTTLEBONE_LIBRARY)

if(BUILD_PHASESPACE AND NOT PHASESPACE_LIBRARY)
    make_dep(Phasespace)
endif(BUILD_PHASESPACE AND NOT PHASESPACE_LIBRARY)

# Allocore --------------------------------
add_subdirectory(allocore)
set(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}")
get_target_property(ALLOCORE_DEP_INCLUDE_DIR "allocore${DEBUG_SUFFIX}" ALLOCORE_DEP_INCLUDE_DIR)
get_target_property(ALLOCORE_LINK_LIBRARIES "allocore${DEBUG_SUFFIX}" ALLOCORE_LINK_LIBRARIES)
if(BUILDING_GAMMA)
  add_dependencies("allocore${DEBUG_SUFFIX}" Gamma)
endif(BUILDING_GAMMA)

# Alloutil ----------------------------------
add_subdirectory(alloutil)
set(ALLOUTIL_LIBRARY "alloutil${DEBUG_SUFFIX}")
get_target_property(ALLOUTIL_DEP_INCLUDE_DIRS alloutil${DEBUG_SUFFIX} ALLOUTIL_DEP_INCLUDE_DIRS)
get_target_property(ALLOUTIL_LINK_LIBRARIES alloutil${DEBUG_SUFFIX} ALLOUTIL_LINK_LIBRARIES)

# alloGLV ---------------------------------------
add_subdirectory(alloGLV)
set(ALLOGLV_LIBRARY "alloGLV${DEBUG_SUFFIX}")
get_target_property(ALLOGLV_INCLUDE_DIR alloGLV${DEBUG_SUFFIX} ALLOGLV_INCLUDE_DIR)
get_target_property(ALLOGLV_LINK_LIBRARIES alloGLV${DEBUG_SUFFIX} ALLOGLV_LINK_LIBRARIES)
if(BUILDING_GLV)
    add_dependencies(alloGLV${DEBUG_SUFFIX} GLV)
endif(BUILDING_GLV)

# allocv ---------------------------------------
if(BUILD_ALLOCV)
add_subdirectory(allocv)
set(ALLOCV_LIBRARY "allocv${DEBUG_SUFFIX}")
get_target_property(ALLOCV_INCLUDE_DIR allocv${DEBUG_SUFFIX} ALLOCV_INCLUDE_DIR)
get_target_property(ALLOCV_LINK_LIBRARIES allocv${DEBUG_SUFFIX} ALLOCV_LINK_LIBRARIES)
endif(BUILD_ALLOCV)

# alloaudio ----------------------------------
add_subdirectory(alloaudio)
get_target_property(ALLOAUDIO_LIBRARY "alloaudio${DEBUG_SUFFIX}" ALLOAUDIO_LIBRARY)
get_target_property(ALLOAUDIO_INCLUDE_DIR alloaudio${DEBUG_SUFFIX} ALLOAUDIO_INCLUDE_DIR)
get_target_property(ALLOAUDIO_LINK_LIBRARIES alloaudio${DEBUG_SUFFIX} ALLOAUDIO_LINK_LIBRARIES)
if(ALLOAUDIO_LIBRARY)
  add_dependencies("alloaudio${DEBUG_SUFFIX}" "${ALLOCORE_LIBRARY}")
endif(ALLOAUDIO_LIBRARY)

# allosphere ----------------------------------
add_subdirectory(allosphere)
get_target_property(ALLOSPHERE_LIBRARY "allosphere${DEBUG_SUFFIX}" ALLOSPHERE_LIBRARY)
get_target_property(ALLOSPHERE_INCLUDE_DIR allosphere${DEBUG_SUFFIX} ALLOSPHERE_INCLUDE_DIR)
get_target_property(ALLOSPHERE_LINK_LIBRARIES allosphere${DEBUG_SUFFIX} ALLOSPHERE_LINK_LIBRARIES)
if(ALLOSPHERE_LIBRARY)
  add_dependencies("allosphere${DEBUG_SUFFIX}" "${ALLOCORE_LIBRARY}")
  set(USE_CPP_11 1) # To trigger usage of C++11 in run script below
endif(ALLOSPHERE_LIBRARY)

# Run from path (Used in run.sh script)
if((ALLOSYSTEM_BUILD_DIR) OR (ALLOSYSTEM_BUILD_APP_FILE))
    set(BUILD_DIR ${ALLOSYSTEM_BUILD_DIR})
    set(BUILD_APP_FILE ${ALLOSYSTEM_BUILD_APP_FILE})
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/CMakeRunTargets.cmake")
    set(BUILD_EXAMPLES 0)
endif((ALLOSYSTEM_BUILD_DIR) OR (ALLOSYSTEM_BUILD_APP_FILE))

# ----------------------------  Examples
# macro to get subdirectories
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

if(BUILD_EXAMPLES)
  if (NOT GAMMA_LIBRARY)
	find_package(Gamma REQUIRED QUIET)
  endif (NOT GAMMA_LIBRARY)
  find_library(CUTTLEBONE_LIBRARY cuttlebone)
  find_library(MYSQLPP_LIBRARY mysqlpp)
  if (CUTTLEBONE_LIBRARY AND MYSQLPP_LIBRARY AND OPENCL_LIBRARY)
  subdirlist(EXAMPLE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
  #include root projects directory
  list(APPEND EXAMPLE_DIRS ".")
  foreach(dir ${EXAMPLE_DIRS})
    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*.cpp)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt)
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt EXTRA_COMPILER_FLAGS)
      #message("NOTE: Using additional flags from examples/${dir}/flags.txt: " ${EXTRA_COMPILER_FLAGS})
      STRING(REGEX REPLACE "[\r\n]" " " EXTRA_COMPILER_FLAGS "${EXTRA_COMPILER_FLAGS}")
    else()
      set(EXTRA_COMPILER_FLAGS "")
    endif()

    # Get a list of all files, later remove all .cpp and .h and you're left with the resources to copy
    file(GLOB EXAMPLE_RES_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*)

    foreach(example_src ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example_src} NAME_WE) # Get name w/o extension
	  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD_ROOT_DIR}/build/bin/examples/${dir}")

      if ("${dir}" STREQUAL ".")
	set(EXAMPLE_TARGET examples_${EXAMPLE_NAME})
      else()
	set(EXAMPLE_TARGET examples_${dir}_${EXAMPLE_NAME})
      endif()
      add_executable(${EXAMPLE_TARGET} ${example_src})

      if(APPLE)
	#These are needed for Luajit on OS X 64-bit binaries
	set_target_properties(${EXAMPLE_TARGET} PROPERTIES
	  LINK_FLAGS "-pagezero_size 10000 -image_base 100000000")
      endif()
      if (EXTRA_COMPILER_FLAGS)
# This is the right way, but it crashes cmake... Bug has been fixed by cmake devs but not released as of this writing...
	#set_source_files_properties(${example_src}
        #  PROPERTIES COMPILE_FLAGS "-I/usr/include/mysql")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILER_FLAGS}")
      endif(EXTRA_COMPILER_FLAGS)
      #message("Adding target for example: ${example_src}")
      include_directories(${ALLOCORE_INCLUDE_DIR} ${ALLOUTIL_INCLUDE_DIR} ${ALLOGLV_INCLUDE_DIR}
        ${GLV_INCLUDE_DIRS} ${GAMMA_INCLUDE_DIRS} ${ALLOAUDIO_INCLUDE_DIR} ${ALLOSPHERE_INCLUDE_DIR})
      #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
      add_dependencies(${EXAMPLE_TARGET} "allocore${DEBUG_SUFFIX}" "alloutil${DEBUG_SUFFIX}" "allosphere${DEBUG_SUFFIX}")
      target_link_libraries(${EXAMPLE_TARGET}
        ${ALLOCORE_LIBRARY} ${ALLOUTIL_LIBRARY} ${GAMMA_LIBRARIES} ${ALLOAUDIO_LIBRARY} ${ALLOSPHERE_LIBRARY}
        ${ALLOCORE_LINK_LIBRARIES} ${ALLOUTIL_LINK_LIBRARIES} ${ALLOGLV_LINK_LIBRARIES}
        ${ALLOAUDIO_LINK_LIBRARIES} ${ALLOSPHERE_LINK_LIBRARIES})
      if(ALLOGLV_LIBRARY)
        add_dependencies(${EXAMPLE_TARGET} ${ALLOGLV_LIBRARY})
        target_link_libraries(${EXAMPLE_TARGET} ${ALLOGLV_LIBRARY} ${ALLOGLV_LINK_LIBRARIES})
      endif(ALLOGLV_LIBRARY)
      list(REMOVE_ITEM EXAMPLE_RES_FILES ${example_src})
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example_src}
	COMMENT "Running: ${EXAMPLE_TARGET}")

    endforeach(example_src)

# TODO: solve asset copying
	foreach(FILENAME ${EXAMPLE_RES_FILES})
      set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
	  set(DST "${BUILD_ROOT_DIR}/build/${FILENAME}")
      if(NOT (IS_DIRECTORY ${SRC}))
	configure_file(${SRC} ${DST} COPY_ONLY)
        message(STATUS "Copying: " ${SRC})
      endif(NOT (IS_DIRECTORY ${SRC}))
    endforeach(FILENAME)
  endforeach(dir)

  endif (CUTTLEBONE_LIBRARY AND MYSQLPP_LIBRARY)

endif(BUILD_EXAMPLES)
