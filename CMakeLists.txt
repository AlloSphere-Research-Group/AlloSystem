project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DEBUG_SUFFIX "_debug")
  message(STATUS "====== Building Debug version")
  if((CMAKE_COMPILER_IS_GNUCC) OR (CMAKE_COMPILER_IS_GNUXX))
    message("Using gcc debug flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
  endif((CMAKE_COMPILER_IS_GNUCC) OR (CMAKE_COMPILER_IS_GNUXX))
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

set(BUILD_GAMMA 1 CACHE STRING "Try to build external Gamma")
set(BUILD_GLV 1 CACHE STRING "Try to build external GLV.")
set(BUILD_EXAMPLES 0 CACHE STRING "Build AlloSystem examples.")

# Windows custom paths
include(cmake_modules/CustomLibPaths.cmake)

# External dependencies (Gamma and GLV)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

include(cmake_modules/make_dep.cmake)

if(BUILD_GAMMA)
    make_dep(Gamma)
    set(GAMMA_LIBRARY ${Gamma_LIBRARY})
endif(BUILD_GAMMA)

if(BUILD_GLV)
    make_dep(GLV)
endif(BUILD_GLV)

# Allocore --------------------------------
add_subdirectory(allocore)
get_target_property(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}" LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIR "allocore${DEBUG_SUFFIX}" ALLOCORE_DEP_INCLUDE_DIR)
get_target_property(ALLOCORE_LINK_LIBRARIES "allocore${DEBUG_SUFFIX}" ALLOCORE_LINK_LIBRARIES)
if(BUILDING_Gamma)
  add_dependencies("allocore${DEBUG_SUFFIX}" Gamma)
endif(BUILDING_Gamma)

# Alloutil ----------------------------------
add_subdirectory(alloutil)
get_target_property(ALLOUTIL_LIBRARY alloutil${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOUTIL_DEP_INCLUDE_DIR alloutil${DEBUG_SUFFIX} ALLOUTIL_DEP_INCLUDE_DIR)
get_target_property(ALLOUTIL_LINK_LIBRARIES alloutil${DEBUG_SUFFIX} ALLOUTIL_LINK_LIBRARIES)

# alloGLV ---------------------------------------

add_subdirectory(alloGLV)
get_target_property(ALLOGLV_LIBRARY alloGLV${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOGLV_INCLUDE_DIR alloGLV${DEBUG_SUFFIX} ALLOGLV_INCLUDE_DIR)
get_target_property(ALLOGLV_LINK_LIBRARIES alloGLV${DEBUG_SUFFIX} ALLOGLV_LINK_LIBRARIES)
if(BUILDING_GLV)
    add_dependencies(alloGLV${DEBUG_SUFFIX} GLV)
endif(BUILDING_GLV)

# allocv ---------------------------------------

add_subdirectory(allocv)
get_target_property(ALLOCV_LIBRARY allocv${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOCV_INCLUDE_DIR allocv${DEBUG_SUFFIX} ALLOCV_INCLUDE_DIR)
get_target_property(ALLOCV_LINK_LIBRARIES allocv${DEBUG_SUFFIX} ALLOCV_LINK_LIBRARIES)

# alloaudio ----------------------------------
add_subdirectory(alloaudio)
get_target_property(ALLOAUDIO_LIBRARY alloaudio${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOAUDIO_INCLUDE_DIR alloaudio${DEBUG_SUFFIX} ALLOAUDIO_INCLUDE_DIR)
get_target_property(ALLOAUDIO_LINK_LIBRARIES alloaudio${DEBUG_SUFFIX} ALLOAUDIO_LINK_LIBRARIES)

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
  find_library(Gamma REQUIRED QUIET)
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
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/${dir})

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
        ${GAMMA_INCLUDE_DIRS} ${ALLOAUDIO_INCLUDE_DIR} )
      #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
      add_dependencies(${EXAMPLE_TARGET} "allocore${DEBUG_SUFFIX}" "alloutil${DEBUG_SUFFIX}")
      target_link_libraries(${EXAMPLE_TARGET}
        ${ALLOCORE_LIBRARY} ${ALLOUTIL_LIBRARY} ${GAMMA_LIBRARIES} ${ALLOAUDIO_LIBRARY}
        ${ALLOCORE_LINK_LIBRARIES} ${ALLOUTIL_LINK_LIBRARIES} ${ALLOGLV_LINK_LIBRARIES}
        ${ALLOAUDIO_LINK_LIBRARIES})
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

    foreach(FILENAME ${EXAMPLE_RES_FILES})
      set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
      set(DST "${CMAKE_SOURCE_DIR}/build/${FILENAME}")
      if(NOT (IS_DIRECTORY ${SRC}))
	configure_file(${SRC} ${DST} COPY_ONLY)
        message(STATUS "Copying: " ${SRC})
      endif(NOT (IS_DIRECTORY ${SRC}))
    endforeach(FILENAME)
  endforeach(dir)

endif(BUILD_EXAMPLES)
