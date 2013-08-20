project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release) # Build release by default

set(ALLOSYSTEM_BUILD 1) # Bulding all of allosystem (do not change this manually)

include(CustomLibPaths.cmake)

# default build set
set(BUILD_GAMMA 0 CACHE STRING "Build external Gamma")
set(BUILD_GLV 0 CACHE STRING "Build external GLV")
set(BUILD_ALLOGLV 1 CACHE STRING "Build AlloGLV")
set(BUILD_ALLOUTIL 1 CACHE STRING "Build alloutil")
set(BUILD_ALLOVSR 0 CACHE STRING "Build allovsr")
set(BUILD_VSR 1 CACHE STRING "Build vsr (only if allovsr is being built)")

# External dependencies (Gamma, GLV and vsr)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

# check for Gamma and build ---------------------------------
if(BUILD_GAMMA)
  set(GAMMA_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../Gamma)
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
  set(GLV_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../GLV)
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
  set(VSR_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../vsr)
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

# Alloutil ----------------------------------
if(BUILD_ALLOUTIL)
  find_package(Lua51 QUIET) # check for Lua to build alloutil
  if(LUA51_FOUND)
    message("Allosystem: LUA found. Building alloutil.")
    add_subdirectory(alloutil)
  else()
    message(FATAL_ERROR "Allosystem: LUA NOT found. NOT Building alloutil.")
  endif(LUA51_FOUND)
  get_target_property(ALLOUTIL_LIBRARY alloutil LOCATION)
  get_target_property(ALLOUTIL_INCLUDE_DIR alloutil ALLOUTIL_INCLUDE_DIR)
else()
  find_package(Alloutil)
endif(BUILD_ALLOUTIL)

# alloGLV ---------------------------------------
if(BUILD_ALLOGLV)
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
else()
  find_package(alloGLV)
endif(BUILD_ALLOGLV)

# allovsr ----------------------------------
if(BUILD_ALLOVSR)
  if(VSR_LIBRARY AND GLV_LIBRARY AND ALLOGLV_LIBRARY)
    add_subdirectory(allovsr)
    message("Allosystem: vsr, GLV and alloGLV libraries found. Building allovsr.")
  else()
    message(FATAL_ERROR "Allosystem: vsr, GLV or alloGLV  NOT found. NOT building allovsr.")
  endif(VSR_LIBRARY AND GLV_LIBRARY AND ALLOGLV_LIBRARY)
endif(BUILD_ALLOVSR)

# ----------------------------  Projects
if(NOT NO_PROJECTS)
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
      add_executable(${PROJECT_NAME} ${project_src})
      message("Building project: projects_${PROJECT_NAME}")
      include_directories(${CMAKE_CURRENT_SOURCE_DIR}/allocore/
        ${CMAKE_CURRENT_SOURCE_DIR}/alloutil/
        ${CMAKE_CURRENT_SOURCE_DIR}/alloGLV/
        ${CMAKE_CURRENT_SOURCE_DIR}/allovsr/
        ${GAMMA_INCLUDE_DIRs} )
      #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
      target_link_libraries(${PROJECT_NAME} allocore alloutil alloGLV ${GAMMA_LIBRARIES}
        ${PORTAUDIO_LIBRARIES} ${SNDFILE_LIBRARIES} ${PA_FRAMEWORKS})
      list(REMOVE_ITEM PROJECT_RES_FILES ${project_src})
      add_custom_target("projects_${PROJECT_NAME}" DEPENDS ${PROJECT_NAME} COMMAND "${CMAKE_SOURCE_DIR}/build/projects/${dir}/${PROJECT_NAME}")
      #    add_custom_command(TARGET "projects_${PROJE${PROJECT_NAME}CT_NAME}" POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/build/projects/${dir}/${PROJECT_NAME})
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
