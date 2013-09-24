project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release) # Build release by default

set(ALLOSYSTEM_BUILD 1) # Bulding all of allosystem (do not change this manually)

include(CustomLibPaths.cmake)

# default build set
set(BUILD_EVERYTHING 0 CACHE STRING "Build everything")

set(BUILD_GAMMA 0 CACHE STRING "Build external Gamma")
set(BUILD_GLV 0 CACHE STRING "Build external GLV. Implies building alloGLV.")
set(BUILD_ALLOGLV 0 CACHE STRING "Build AlloGLV")
set(BUILD_ALLOUTIL 1 CACHE STRING "Build alloutil")
set(BUILD_VSR 0 CACHE STRING "Build vsr. Implies building vsr.")
set(BUILD_ALLOVSR 0 CACHE STRING "Build allovsr")

if(BUILD_EVERYTHING OR BUILD_ALL)
  set(BUILD_GAMMA 1)
  set(BUILD_GLV 1)
  set(BUILD_ALLOGLV 1)
  set(BUILD_ALLOUTIL 1)
  set(BUILD_VSR 1)
  set(BUILD_ALLOVSR 1)
endif(BUILD_EVERYTHING OR BUILD_ALL)

# External dependencies (Gamma, GLV and vsr)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

# check for Gamma and build ---------------------------------
if(BUILD_GAMMA)
  set(GAMMA_RELATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Gamma)
  if(EXISTS "${GAMMA_RELATIVE_DIR}" AND IS_DIRECTORY "${GAMMA_RELATIVE_DIR}")
    message("Allosystem: GAMMA sources found. Building Gamma.")
    set(BUILDING_GAMMA 1)
    add_subdirectory(${GAMMA_RELATIVE_DIR} ${GAMMA_RELATIVE_DIR}/AlloSystem-build)
    set(GAMMA_INCLUDE_DIR ${GAMMA_RELATIVE_DIR})
    get_target_property(GAMMA_LIBRARY Gamma LOCATION)
    else()
    message(FATAL_ERROR "Allosystem: Gamma sources NOT found. Can't build Gamma.")
  endif()
else()
  find_package(Gamma)
endif(BUILD_GAMMA)

# check for GLV and build ---------------------------------
if(BUILD_GLV)
  set(GLV_RELATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../GLV)
  if(EXISTS "${GLV_RELATIVE_DIR}" AND IS_DIRECTORY "${GLV_RELATIVE_DIR}")
    message("Allosystem: GLV sources found. Building GLV.")
    set(BUILDING_GLV 1)
    add_subdirectory(${GLV_RELATIVE_DIR} ${GLV_RELATIVE_DIR}/AlloSystem-build)
    set(GLV_INCLUDE_DIR ${GLV_RELATIVE_DIR})
    get_target_property(GLV_LIBRARY GLV LOCATION)
  else()
    message(FATAL_ERROR "Allosystem: GLV sources NOT found. Can't build GLV.")
  endif()
else()
  find_package(GLV)
endif(BUILD_GLV)

# check for vsr and build ----------------------------
if(BUILD_VSR AND BUILD_ALLOVSR)
  set(VSR_RELATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vsr)
  if((EXISTS "${VSR_RELATIVE_DIR}" AND IS_DIRECTORY "${VSR_RELATIVE_DIR}"))
    set(BUILDING_VSR 1)
    add_subdirectory(${VSR_RELATIVE_DIR} ${VSR_RELATIVE_DIR}/Allosystem-build)
    set_target_properties(vsr PROPERTIES GLV_INCLUDE_DIR "${GLV_RELATIVE_DIR}"
      GLV_LIBRARY "${GLV_LIBRARY}")
    set(VSR_INCLUDE_DIR ${VSR_RELATIVE_DIR})
    get_target_property(VSR_LIBRARY vsr LOCATION)
  else()
    message(FATAL_ERROR "Allosystem: vsr sources NOT found. Can't build vsr.")
  endif()
endif(BUILD_VSR AND BUILD_ALLOVSR)

# Allocore --------------------------------
add_subdirectory(allocore)
get_target_property(ALLOCORE_LIBRARY allocore LOCATION)
get_target_property(ALLOCORE_INCLUDE_DIR allocore ALLOCORE_INCLUDE_DIR)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore ALLOCORE_LINK_LIBRARIES)

# Alloutil ----------------------------------
if(BUILD_ALLOUTIL)
  find_package(Lua51 QUIET) # check for Lua to build alloutil
  if(LUA51_FOUND)
    message("Allosystem: LUA found. Building alloutil.")
    add_subdirectory(alloutil)
    add_dependencies(alloutil allocore)
  else()
    message(FATAL_ERROR "Allosystem: LUA NOT found. NOT Building alloutil.")
  endif(LUA51_FOUND)
  get_target_property(ALLOUTIL_LIBRARY alloutil LOCATION)
  get_target_property(ALLOUTIL_INCLUDE_DIR alloutil ALLOUTIL_INCLUDE_DIR)
  get_target_property(ALLOUTIL_LINK_LIBRARIES alloutil ALLOUTIL_LINK_LIBRARIES)
else()
  find_package(Alloutil QUIET)
endif(BUILD_ALLOUTIL)

# alloGLV ---------------------------------------
if(BUILD_ALLOGLV OR BUILD_GLV)
  if(BUILDING_GLV)
    message("Allosystem: GLV sources found. Building GLV and alloGLV.")
    add_subdirectory(alloGLV)
    add_dependencies(alloGLV GLV)
  else()
    find_package(GLV QUIET)
    if(${GLV_FOUND})
      message("Allosystem: GLV found. Building alloGLV.")
      add_subdirectory(alloGLV)
      get_target_property(GLV_LIBRARY GLV GLV_LIBRARY)
      set(GLV_INCLUDE_DIR ${GLV_RELATIVE_DIR})
    else()
      message(FATAL_ERROR "Allosystem: GLV NOT found. Can't build alloGLV.")
    endif()
  endif(BUILDING_GLV)
    get_target_property(ALLOGLV_LIBRARY alloGLV LOCATION)
    get_target_property(ALLOGLV_INCLUDE_DIR alloGLV ALLOGLV_INCLUDE_DIR)
    get_target_property(ALLOGLV_LINK_LIBRARIES alloGLV ALLOGLV_LINK_LIBRARIES)
else()
  find_package(AlloGLV QUIET)
endif(BUILD_ALLOGLV OR BUILD_GLV)

# allovsr ----------------------------------
if(BUILD_ALLOVSR OR BUILD_VSR)
  if(VSR_LIBRARY AND GLV_LIBRARY AND ALLOGLV_LIBRARY)
    add_subdirectory(allovsr)
    message("Allosystem: vsr, GLV and alloGLV libraries found. Building allovsr.")
  else()
    message(FATAL_ERROR "Allosystem: vsr, GLV or alloGLV  NOT found. NOT building allovsr.")
  endif(VSR_LIBRARY AND GLV_LIBRARY AND ALLOGLV_LIBRARY)
endif(BUILD_ALLOVSR OR BUILD_VSR)

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

if(NOT NO_EXAMPLES)
  find_library(Gamma REQUIRED QUIET)
  subdirlist(EXAMPLE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
  #include root projects directory
  list(APPEND EXAMPLE_DIRS ".")
  foreach(dir ${EXAMPLE_DIRS})
    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*.cpp)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt)
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/examples/${dir}/flags.txt EXTRA_COMPILER_FLAGS)
      message("WARNING: Using additional flags from examples/${dir}/flags.txt: " ${EXTRA_COMPILER_FLAGS})
    endif()

    # Get a list of all files, later remove all .cpp and .h and you're left with the resources to copy
    file(GLOB EXAMPLE_RES_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/${dir}/*)

    foreach(example_src ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example_src} NAME_WE) # Get name w/o extension
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/examples/${dir})

      if ("${dir}" STREQUAL ".")
	set(EXAMPLE_TARGET examples_${EXAMPLE_NAME})
      else()
	set(EXAMPLE_TARGET examples_${dir}_${EXAMPLE_NAME})
      endif()
      add_executable(${EXAMPLE_TARGET} EXCLUDE_FROM_ALL ${example_src})
      message("Adding target for example: ${example_src}")
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/allocore/
        ${CMAKE_CURRENT_SOURCE_DIR}/alloutil/
        ${CMAKE_CURRENT_SOURCE_DIR}/alloGLV/
        ${CMAKE_CURRENT_SOURCE_DIR}/allovsr/
        ${GAMMA_INCLUDE_DIRs} )
      #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
      target_link_libraries(${EXAMPLE_TARGET} allocore alloutil alloGLV ${GAMMA_LIBRARIES}
        ${ALLOCORE_LINK_LIBRARIES} ${ALLOUTIL_LINK_LIBRARIES} ${ALLOGLV_LINK_LIBRARIES})
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

endif(NOT NO_EXAMPLES)


# ----------------------------  Projects
if(NOT NO_PROJECTS)
  find_library(Gamma REQUIRED QUIET)
  subdirlist(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/projects)
  #include root projects directory
  list(APPEND PROJECT_DIRS ".")
  foreach(dir ${PROJECT_DIRS})
    file(GLOB PROJECT_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} projects/${dir}/*.cpp)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/projects/${dir}/flags.txt)
      file(READ ${CMAKE_CURRENT_SOURCE_DIR}/projects/${dir}/flags.txt EXTRA_COMPILER_FLAGS)
      message("WARNING: Using additional flags from /projects/${dir}/flags.txt: " ${EXTRA_COMPILER_FLAGS})
    endif()

    # Get a list of all files, later remove all .cpp and .h and you're left with the resources to copy
    file(GLOB PROJECT_RES_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} projects/${dir}/*)

    foreach(project_src ${PROJECT_FILES})
      get_filename_component(PROJECT_NAME ${project_src} NAME_WE) # Get name w/o extension
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/projects/${dir})

      if ("${dir}" STREQUAL ".")
	set(PROJECT_TARGET projects_${PROJECT_NAME})
      else()
	set(PROJECT_TARGET projects_${dir}_${PROJECT_NAME})
      endif()
      add_executable(${PROJECT_TARGET} EXCLUDE_FROM_ALL ${project_src})
      message("Adding target for project: ${project_src}")
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/allocore/
        ${CMAKE_CURRENT_SOURCE_DIR}/alloutil/
        ${CMAKE_CURRENT_SOURCE_DIR}/alloGLV/
        ${CMAKE_CURRENT_SOURCE_DIR}/allovsr/
        ${GAMMA_INCLUDE_DIRs} )
      #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
      target_link_libraries(${PROJECT_TARGET} allocore alloutil alloGLV ${GAMMA_LIBRARIES}
        ${ALLOCORE_LINK_LIBRARIES} ${ALLOUTIL_LINK_LIBRARIES} ${ALLOGLV_LINK_LIBRARIES})
      list(REMOVE_ITEM PROJECT_RES_FILES ${project_src})
      add_custom_target("${PROJECT_TARGET}_run"
	COMMAND "${PROJECT_TARGET}"
	DEPENDS "${PROJECT_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${project_src}
	COMMENT "Running: ${PROJECT_TARGET}")

    endforeach(project_src)

    foreach(FILENAME ${PROJECT_RES_FILES})
      set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
      set(DST "${CMAKE_SOURCE_DIR}/build/${FILENAME}")
      if(NOT (IS_DIRECTORY ${SRC}))
	configure_file(${SRC} ${DST} COPY_ONLY)
	message("Copying: " ${SRC})
      endif(NOT (IS_DIRECTORY ${SRC}))
    endforeach(FILENAME)
  endforeach(dir)

endif(NOT NO_PROJECTS)
