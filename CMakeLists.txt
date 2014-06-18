project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(DEBUG_SUFFIX "_debug")
  message("====== Building Debug version")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# Windows custom paths
include(cmake_modules/CustomLibPaths.cmake)

set(BUILD_GAMMA 1 CACHE STRING "Build external Gamma")
set(BUILD_GLV 1 CACHE STRING "Build external GLV. Implies building alloGLV.")
set(BUILD_VSR 1 CACHE STRING "Build vsr. Implies building vsr.")
set(BUILD_EXAMPLES 0 CACHE STRING "Build AlloSystem examples.")

# External dependencies (Gamma, GLV and vsr)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

# check for Gamma and build ---------------------------------
if(BUILD_GAMMA)
  set(GAMMA_RELATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Gamma)
  if(EXISTS "${GAMMA_RELATIVE_DIR}" AND IS_DIRECTORY "${GAMMA_RELATIVE_DIR}")
    if (EXISTS "${GAMMA_RELATIVE_DIR}/CMakeLists.txt")
      message("Allosystem: GAMMA sources with CMakeLists found. Building Gamma.")
      set(BUILDING_GAMMA 1)
      add_subdirectory(${GAMMA_RELATIVE_DIR} ${GAMMA_RELATIVE_DIR}/AlloSystem-build)
      set(GAMMA_INCLUDE_DIR ${GAMMA_RELATIVE_DIR})
      get_target_property(GAMMA_LIBRARY Gamma LOCATION)
    else()
      execute_process(COMMAND make WORKING_DIRECTORY "${GAMMA_RELATIVE_DIR}"
        RESULT_VARIABLE make_result)
      if (NOT make_result EQUAL 0)
        message("Allosystem: Gamma make failed. Can't build Gamma.")
      else()
        find_package(Gamma)
      endif(NOT make_result EQUAL 0)
    endif()
  else()
    message("Allosystem: Gamma sources NOT found. Can't build Gamma.")
  endif()
endif(BUILD_GAMMA)

# check for GLV and build ---------------------------------
if(BUILD_GLV)
  set(GLV_RELATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../GLV)
  if(EXISTS "${GLV_RELATIVE_DIR}" AND IS_DIRECTORY "${GLV_RELATIVE_DIR}")
    if (EXISTS "${GLV_RELATIVE_DIR}/CMakeLists.txt")
      message("Allosystem: GLV sources with CMakeLists.txt found. Building GLV.")
      set(BUILDING_GLV 1)
      add_subdirectory(${GLV_RELATIVE_DIR} ${GLV_RELATIVE_DIR}/AlloSystem-build)
      set(GLV_INCLUDE_DIR ${GLV_RELATIVE_DIR})
      get_target_property(GLV_LIBRARY GLV LOCATION)
    else()
      execute_process(COMMAND make WORKING_DIRECTORY "${GLV_RELATIVE_DIR}"
        RESULT_VARIABLE make_result)
      if (NOT make_result EQUAL 0)
        message("Allosystem: GLV make failed. Can't build GLV.")
      else()
        find_package(GLV) # To find built GLV
      endif(NOT make_result EQUAL 0)
    endif()
  else()
    message("Allosystem: GLV sources NOT found. Can't build GLV.")
  endif()
endif(BUILD_GLV)

# check for vsr and build ----------------------------
if(BUILD_VSR)
  set(VSR_RELATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vsr)
  if((EXISTS "${VSR_RELATIVE_DIR}" AND IS_DIRECTORY "${VSR_RELATIVE_DIR}"))
    set(BUILDING_VSR 1)
    add_subdirectory(${VSR_RELATIVE_DIR} ${VSR_RELATIVE_DIR}/Allosystem-build)
    set_target_properties(vsr PROPERTIES GLV_INCLUDE_DIR "${GLV_RELATIVE_DIR}"
      GLV_LIBRARY "${GLV_LIBRARY}")
    set(VSR_INCLUDE_DIR ${VSR_RELATIVE_DIR})
    get_target_property(VSR_LIBRARY vsr LOCATION)
  else()
    message("Allosystem: vsr sources NOT found. Can't build vsr.")
  endif()
endif(BUILD_VSR)

# Allocore --------------------------------
add_subdirectory(allocore)
get_target_property(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}" LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIR "allocore${DEBUG_SUFFIX}" ALLOCORE_DEP_INCLUDE_DIR)
get_target_property(ALLOCORE_LINK_LIBRARIES "allocore${DEBUG_SUFFIX}" ALLOCORE_LINK_LIBRARIES)

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
    add_dependencies(alloGLV GLV)
endif(BUILDING_GLV)

# allovsr ----------------------------------
add_subdirectory(allovsr)
if(BUILDING_VSR)
  add_dependencies(allovsr vsr)
endif(BUILDING_VSR)

# Run from path (Used in run.sh script)
if((BUILD_DIR) OR (BUILD_APP_FILE))
  include(cmake_modules/CMakeRunTargets.cmake)
endif((BUILD_DIR) OR (BUILD_APP_FILE))

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
      include_directories(${ALLOCORE_INCLUDE_DIR} ${ALLOUTIL_INCLUDE_DIR} ${ALLOGLV_INCLUDE_DIR} ${ALLOVSR_INCLUDE_DIR}
        ${GAMMA_INCLUDE_DIRS} )
      #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
      add_dependencies(${EXAMPLE_TARGET} "allocore${DEBUG_SUFFIX}" "alloutil${DEBUG_SUFFIX}")
      target_link_libraries(${EXAMPLE_TARGET}
	${ALLOCORE_LIBRARY} ${ALLOUTIL_LIBRARY} ${GAMMA_LIBRARIES}
        ${ALLOCORE_LINK_LIBRARIES} ${ALLOUTIL_LINK_LIBRARIES} ${ALLOGLV_LINK_LIBRARIES})
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
	message("Copying: " ${SRC})
      endif(NOT (IS_DIRECTORY ${SRC}))
    endforeach(FILENAME)
  endforeach(dir)

endif(BUILD_EXAMPLES)
