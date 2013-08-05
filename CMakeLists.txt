project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
# Build release by default
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(allocore)

set(ALLOSYSTEM_BUILD 1) # Bulding all of allosystem

# check for Lua to build alloutil
find_package(Lua51)
if(LUA51_FOUND)
add_subdirectory(alloutil)
endif(LUA51_FOUND)

# check for GLV to build alloGLV
set(GLV_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../GLV)

if(EXISTS "${GLV_RELATIVE_DIR}" AND IS_DIRECTORY "${GLV_RELATIVE_DIR}")
  message("Building GLV")
  set(BUILDING_GLV 1)
  add_subdirectory(${CMAKE_SOURCE_DIR}/../GLV ${CMAKE_SOURCE_DIR}/../GLV-build)
  add_subdirectory( alloGLV )
else()
find_package(GLV)
  if(${GLV_FOUND})
    add_subdirectory( alloGLV )
  endif()
endif()


# check for vsr to build allovsr
set(VSR_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../vsr)

if(EXISTS "${VSR_RELATIVE_DIR}" AND IS_DIRECTORY "${VSR_RELATIVE_DIR}")
  message("Building VSR")
  set(BUILDING_VSR 1)
  add_subdirectory(${CMAKE_SOURCE_DIR}/../vsr ${CMAKE_SOURCE_DIR}/../vsr-build)
  add_subdirectory( allovsr )
else()
find_package(vsr)
  if(${VSR_FOUND})
    add_subdirectory( allovsr )
  endif()
endif()


# ----------------------------  Projects
if(NOT NO_PROJECTS)
# macro to get subdirectories
find_library(Gamma REQUIRED)

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
    message("Building project: " ${dir} "/" ${PROJECT_NAME})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/allocore/
                        ${CMAKE_CURRENT_SOURCE_DIR}/alloutil/
                        ${CMAKE_CURRENT_SOURCE_DIR}/alloGLV/
                        ${CMAKE_CURRENT_SOURCE_DIR}/allovsr/
                        ${GAMMA_INCLUDE_DIRs} )
    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
    target_link_libraries(${PROJECT_NAME} allocore alloutil alloGLV ${GAMMA_LIBRARIES}
                          ${PORTAUDIO_LIBRARIES} ${SNDFILE_LIBRARIES} ${PA_FRAMEWORKS})
    list(REMOVE_ITEM PROJECT_RES_FILES ${project_src})
#    add_custom_command(TARGET ${project_src} POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/build/projects/${dir}/${PROJECT_NAME})
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
