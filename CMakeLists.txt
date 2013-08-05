project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
# Build release by default
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(allocore)

set(ALLOSYSTEM_BUILD 1) # Bulding all of allosystem
set(BUILD_GAMMA 1)
set(BUILD_GLV 1)

# check for Lua to build alloutil --------------------------
find_package(Lua51 QUIET)
if(LUA51_FOUND)
message("Allosystem: LUA found. Building alloutil.")
add_subdirectory(alloutil)
endif(LUA51_FOUND)

# check for Gamma and build ---------------------------------
if(BUILD_GAMMA)
set(GAMMA_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../Gamma)
if(EXISTS "${GAMMA_RELATIVE_DIR}" AND IS_DIRECTORY "${GAMMA_RELATIVE_DIR}")
  message("Allosystem: GAMMA sources found. Building ammaA.")
  set(BUILDING_GAMMA 1)
  execute_process(COMMAND "${GAMMA_RELATIVE_DIR}/distclean"
                  WORKING_DIRECTORY "${GAMMA_RELATIVE_DIR}")
  add_subdirectory(${GAMMA_RELATIVE_DIR} ${GAMMA_RELATIVE_DIR})
  set(GAMMA_INCLUDE_DIR ${GAMMA_RELATIVE_DIR})
endif()
endif(BUILD_GAMMA)

# check for GLV and build ---------------------------------
if(BUILD_GLV)
set(GLV_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../GLV)
if(EXISTS "${GLV_RELATIVE_DIR}" AND IS_DIRECTORY "${GLV_RELATIVE_DIR}")
  message("Allosystem: GLV sources found. Building GLV.")
  set(BUILDING_GLV 1)
  execute_process(COMMAND "${GLV_RELATIVE_DIR}/distclean"
                  WORKING_DIRECTORY "${GLV_RELATIVE_DIR}")
  add_subdirectory(${GLV_RELATIVE_DIR} ${GLV_RELATIVE_DIR})
  set(GLV_INCLUDE_DIR ${GLV_RELATIVE_DIR})
endif()
endif(BUILD_GLV)

# Check and build alloGLV ---------------------------------------
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
  endif()
endif()

# check for vsr to build allovsr ----------------------------------
set(VSR_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../vsr)
if((EXISTS "${VSR_RELATIVE_DIR}" AND IS_DIRECTORY "${VSR_RELATIVE_DIR}"))
  set(BUILDING_VSR 1)
  execute_process(COMMAND "${VSR_RELATIVE_DIR}/distclean"
                  WORKING_DIRECTORY "${VSR_RELATIVE_DIR}")
  add_subdirectory(${VSR_RELATIVE_DIR} ${VSR_RELATIVE_DIR})
  set_target_properties(vsr PROPERTIES GLV_INCLUDE_DIR ${GLV_RELATIVE_DIR}
                                       GLV_LIBRARY "${GLV_LIBRARY}")
  set(VSR_INCLUDE_DIR ${VSR_RELATIVE_DIR})
  set(VSR_LIBRARY vsr)
  add_subdirectory(allovsr)
else()
  find_package(vsr QUIET)
  find_package(GLV QUIET)
  if(VSR_FOUND AND GLV_FOUND)
    message("Allosystem: vsr and GLV libraries found. Building allovsr.")
    add_subdirectory(allovsr)
  endif()
endif()


# ----------------------------  Projects
if(NOT NO_PROJECTS)
# macro to get subdirectories
find_library(Gamma REQUIRED QUIET)

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
    add_custom_target("project_${PROJECT_NAME}" DEPENDS ${PROJECT_NAME} COMMAND "${CMAKE_SOURCE_DIR}/build/projects/${dir}/${PROJECT_NAME}")
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

#from old Makefile

#Linux
#-lm -lstdc++

#OS X
#ARCH = 64
#CXX = @clang++-mp-3.1 -std=c++11
