# Basic checks
#if(NOT (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})) # use only if cmake was run from the root directory
#    message(FATAL_ERROR "Error: The run script must be called from the source root directory." )
#endif(NOT (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}))

message("Using AlloSystem Run facilties.")

string(REGEX MATCH ".*\\*.*" match "${CMAKE_CURRENT_SOURCE_DIR}")
IF(NOT ${match} STREQUAL "")
  message(FATAL_ERROR "Error: Please remove '*' from path!" ) # This avoids issues with the run script
ENDIF()

if(BUILD_DIR)
  string(REGEX REPLACE "/+$" "" ALLOSYSTEM_BUILD_APP_DIR "${ALLOSYSTEM_BUILD_APP_DIR}") # remove trailing slash
  file(GLOB ALLOSYSTEM_APP_SRC "${CMAKE_SOURCE_DIR}/${ALLOSYSTEM_BUILD_APP_DIR}/*.cpp")
  file(GLOB ALLOSYSTEM_CUDA_HEADERS "${CMAKE_SOURCE_DIR}/${ALLOSYSTEM_BUILD_APP_DIR}/*.cuh")
  file(GLOB ALLOSYSTEM_CUDA_SRC "${CMAKE_SOURCE_DIR}/${ALLOSYSTEM_BUILD_APP_DIR}/*.cu")
  list(APPEND ALLOSYSTEM_CUDA_SRC ${ALLOSYSTEM_CUDA_HEADERS})
#  file(GLOB ALLOSYSTEM_APP_SRC RELATIVE "${CMAKE_SOURCE_DIR}" "${ALLOSYSTEM_BUILD_APP_DIR}/*.cpp")
  string(REPLACE "/" "_" APP_NAME "${ALLOSYSTEM_BUILD_APP_DIR}")
  string(REGEX REPLACE "_+$" "" APP_NAME "${APP_NAME}")
  set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/${ALLOSYSTEM_BUILD_APP_DIR}")
else()
  string(REPLACE "/" "_" APP_NAME "${BUILD_APP_FILE}")
  get_filename_component(APP_NAME "${APP_NAME}" NAME)
  STRING(REGEX REPLACE "\\.[^.]*\$" "" APP_NAME "${APP_NAME}")
  string(REPLACE "." "_" APP_NAME "${APP_NAME}")
  if (IS_ABSOLUTE "${BUILD_APP_FILE}")
    set(ALLOSYSTEM_APP_SRC "${BUILD_APP_FILE}")
  else()
    set(ALLOSYSTEM_APP_SRC "${CMAKE_SOURCE_DIR}/${BUILD_APP_FILE}")
  endif()
#  get_filename_component(APP_NAME ${APP_NAME} NAME_WE) # Get name w/o extension (extension is anything after first dot!)
  get_filename_component(SOURCE_DIR "${BUILD_APP_FILE}" PATH)
  set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/${SOURCE_DIR}")
endif(BUILD_DIR)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

# -------------------------- BuildAlloTarget

function(BuildAlloTarget ALLO_APP_NAME ALLO_APP_SRC ALLO_CUDA_SRC DEFINES SUFFIX)

if(NOT "${SUFFIX}" STREQUAL "")
set(ALLO_APP_NAME_SUFFIX "${${ALLO_APP_NAME}}_${SUFFIX}")
else()
set(ALLO_APP_NAME_SUFFIX "${${ALLO_APP_NAME}}")
endif()

set(BUILDING_CUDA_APP 0)

if(NOT "${ALLO_CUDA_SRC}" EQUAL "")
  message("Building CUDA application.")
  set(BUILDING_CUDA_APP 1)
  if(UNIX AND NOT APPLE)
    # CUDA version 7.5 (current) supports gcc-4.8 (not higher)
    set(CMAKE_CXX_COMPILER g++-4.8)
    set(CMAKE_C_COMPILER gcc-4.8)
    set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
    find_package(CUDA REQUIRED)
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -gencode arch=compute_30,code=sm_30 -ccbin /usr/bin/gcc-4.8 --std=c++11)
    cuda_add_executable("${ALLO_APP_NAME_SUFFIX}"
      ${${ALLO_APP_SRC}} ${ALLO_CUDA_SRC}
      OPTIONS -DSTUFF="blah blah"
      RELEASE -DNDEBUG
      DEBUG -g -DDEBUG
    )
    target_include_directories("${ALLO_APP_NAME_SUFFIX}" ""${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc/")
  else()
  # TODO add support for CUDA on OS X
    set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda")
    find_package(CUDA REQUIRED)
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -gencode arch=compute_30,code=sm_30 --std=c++11)
    cuda_add_executable("${ALLO_APP_NAME_SUFFIX}"
      ${${ALLO_APP_SRC}} ${ALLO_CUDA_SRC}
      OPTIONS -DSTUFF="blah blah"
      RELEASE -DNDEBUG
      DEBUG -g -DDEBUG
    )
    target_include_directories("${ALLO_APP_NAME_SUFFIX}" ""${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc/")
  endif()

else()
  # No CUDA sources, build regular application
  add_executable("${ALLO_APP_NAME_SUFFIX}" EXCLUDE_FROM_ALL ${${ALLO_APP_SRC}})
endif(BUILDING_CUDA_APP)



if(NOT ${DEFINES} STREQUAL "")
#message("Adding defines ${DEFINES}")
target_compile_definitions("${ALLO_APP_NAME_SUFFIX}" PUBLIC "${DEFINES}")
endif()

if(EXISTS "${SOURCE_DIR}/flags.cmake")
    include("${SOURCE_DIR}/flags.cmake")
endif()

if(COMPILER_SUPPORTS_CXX11)
	set_property(TARGET ${ALLO_APP_NAME_SUFFIX} APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++11 ")
elseif(COMPILER_SUPPORTS_CXX0X)
	set_property(TARGET ${ALLO_APP_NAME_SUFFIX} APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++0x")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(${ALLO_APP_NAME_SUFFIX} PROPERTIES
    LINK_FLAGS "-pagezero_size 10000 -image_base 100000000")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(EXISTS "${SOURCE_DIR}/flags.txt")
  file(READ "${SOURCE_DIR}/flags.txt" EXTRA_COMPILER_FLAGS)
  STRING(REGEX REPLACE "[\r\n]" " " EXTRA_COMPILER_FLAGS "${EXTRA_COMPILER_FLAGS}")
  set_target_properties(${ALLO_APP_NAME_SUFFIX} PROPERTIES
    COMPILE_FLAGS "${EXTRA_COMPILER_FLAGS}")
  message(STATUS "NOTE: Using additional flags from ${SOURCE_DIR}/flags.txt: ${EXTRA_COMPILER_FLAGS}")
endif()

# Dependencies (check if targets exist and set variables)
set(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX})
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore${DEBUG_SUFFIX} ALLOCORE_LINK_LIBRARIES)

message(STATUS "Target: ${ALLO_APP_NAME_SUFFIX}")
message(STATUS "From sources: ${${ALLO_APP_SRC}}")

add_dependencies("${ALLO_APP_NAME_SUFFIX}" allocore${DEBUG_SUFFIX})


#message("Using allocore headers from: ${ALLOCORE_DEP_INCLUDE_DIRS}")

if(BUILDING_GAMMA)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" ${GAMMA_LIBRARY})
    target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${GAMMA_LIBRARY})
    include_directories(${GAMMA_INCLUDE_DIR})
else()
  if(NOT GAMMA_FOUND)
    set(GAMMA_LIBRARY "")
    set(GAMMA_INCLUDE_DIR "")
    message("Not building GAMMA and no usable GAMMA binary found. Not linking application to GAMMA")
  endif(NOT GAMMA_FOUND)
  target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${GAMMA_LIBRARY})
  include_directories(${GAMMA_INCLUDE_DIR})
endif(BUILDING_GAMMA)

if(BUILDING_GLV)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" ${GLV_LIBRARY})
    target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${GLV_LIBRARY})
    include_directories(${GLV_INCLUDE_DIR})
else()
  if(NOT GLV_FOUND)
    set(GLV_LIBRARY "")
    set(GLV_INCLUDE_DIR "")
    message("Not building GLV and no usable GLV binary found. Not linking application to GLV")
  endif(NOT GLV_FOUND)
  target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${GLV_LIBRARY})
  include_directories(${GLV_INCLUDE_DIR})
endif(BUILDING_GLV)

if(BUILDING_CUTTLEBONE)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" ${CUTTLEBONE_LIBRARY})
    target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${CUTTLEBONE_LIBRARY})
    include_directories(${CUTTLEBONE_INCLUDE_DIR})
else()
  if(NOT CUTTLEBONE_FOUND)
    set(CUTTLEBONE_LIBRARY "")
    set(CUTTLEBONE_INCLUDE_DIR "")
    message("Not building Cuttlebone and no usable Cuttlebone binary found. Not linking application to Cuttlebone")
  endif(NOT CUTTLEBONE_FOUND)
    target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${CUTTLEBONE_LIBRARY})
    include_directories(${CUTTLEBONE_INCLUDE_DIR})
endif(BUILDING_CUTTLEBONE)

if(BUILDING_PHASESPACE)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" ${PHASESPACE_LIBRARY})
    target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${PHASESPACE_LIBRARY})
    include_directories(${PHASESPACE_INCLUDE_DIR})
else()
  if(NOT PHASESPACE_FOUND)
    set(PHASESPACE_LIBRARY "")
    set(PHASESPACE_INCLUDE_DIR "")
    message(STATUS "Not building Phasespace and no usable Phasespace binary found. Not linking application to Phasespace")
  endif(NOT PHASESPACE_FOUND)
    target_link_libraries(${ALLO_APP_NAME_SUFFIX} ${PHASESPACE_LIBRARY})
    include_directories(${PHASESPACE_INCLUDE_DIR})
endif(BUILDING_PHASESPACE)

if(TARGET alloutil${DEBUG_SUFFIX})
    set(ALLOUTIL_LIBRARY alloutil${DEBUG_SUFFIX})
    get_target_property(ALLOUTIL_DEP_INCLUDE_DIR alloutil${DEBUG_SUFFIX} ALLOUTIL_DEP_INCLUDE_DIR)
    get_target_property(ALLOUTIL_LINK_LIBRARIES alloutil${DEBUG_SUFFIX} ALLOUTIL_LINK_LIBRARIES)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" alloutil${DEBUG_SUFFIX})
    target_link_libraries("${ALLO_APP_NAME_SUFFIX}" ${ALLOUTIL_LIBRARY} ${ALLOUTIL_LINK_LIBRARIES})
    include_directories(${ALLOUTIL_DEP_INCLUDE_DIR})
else()
  if(NOT ALLOUTIL_FOUND)
    set(ALLOUTIL_LIBRARY "")
    set(ALLOUTIL_DEP_INCLUDE_DIR "")
    message("Not building ALLOUTIL and no usable ALLOUTIL binary found. Not linking application to ALLOUTIL")
  endif(NOT ALLOUTIL_FOUND)
endif(TARGET alloutil${DEBUG_SUFFIX})

if(TARGET alloGLV${DEBUG_SUFFIX})
    set(ALLOGLV_LIBRARY alloGLV${DEBUG_SUFFIX})
    get_target_property(ALLOGLV_INCLUDE_DIR alloGLV${DEBUG_SUFFIX} ALLOGLV_INCLUDE_DIR)
    get_target_property(ALLOGLV_LINK_LIBRARIES "alloGLV${DEBUG_SUFFIX}" ALLOGLV_LINK_LIBRARIES)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" alloGLV${DEBUG_SUFFIX})
    target_link_libraries("${ALLO_APP_NAME_SUFFIX}" ${ALLOGLV_LIBRARY} ${ALLOGLV_LINK_LIBRARIES})
    include_directories(${ALLOGLV_INCLUDE_DIR})
else()
  if(NOT ALLOGLV_FOUND)
    set(ALLOGLV_LIBRARY "")
    set(ALLOGLV_INCLUDE_DIR "")
    message("Not building alloGLV and no usable alloGLV binary found. Not linking application to alloGLV")
  endif(NOT ALLOGLV_FOUND)
endif(TARGET alloGLV${DEBUG_SUFFIX})

if(TARGET alloaudio${DEBUG_SUFFIX})
    set(ALLOAUDIO_LIBRARY alloaudio${DEBUG_SUFFIX})
    get_target_property(ALLOAUDIO_INCLUDE_DIR alloaudio${DEBUG_SUFFIX} ALLOAUDIO_INCLUDE_DIR)
    get_target_property(ALLOAUDIO_LINK_LIBRARIES "alloaudio${DEBUG_SUFFIX}" ALLOAUDIO_LINK_LIBRARIES)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" alloaudio${DEBUG_SUFFIX})
    target_link_libraries("${ALLO_APP_NAME_SUFFIX}" ${ALLOAUDIO_LIBRARY} ${ALLOAUDIO_LINK_LIBRARIES})
    include_directories(${ALLOAUDIO_INCLUDE_DIR})
else()
  if(NOT ALLOAUDIO_FOUND)
    set(ALLOAUDIO_LIBRARY "")
    set(ALLOAUDIO_INCLUDE_DIR "")
    message("Not building alloaudio and no usable alloaudio binary found. Not linking application to alloauadio")
  endif(NOT ALLOAUDIO_FOUND)
endif(TARGET alloaudio${DEBUG_SUFFIX})

if(TARGET allocv${DEBUG_SUFFIX})
    set(ALLOCV_LIBRARY allocv${DEBUG_SUFFIX})
    get_target_property(ALLOCV_INCLUDE_DIR allocv${DEBUG_SUFFIX} ALLOCV_INCLUDE_DIR)
    get_target_property(ALLOCV_LINK_LIBRARIES "allocv${DEBUG_SUFFIX}" ALLOCV_LINK_LIBRARIES)
    add_dependencies("${ALLO_APP_NAME_SUFFIX}" allocv${DEBUG_SUFFIX})
    target_link_libraries("${ALLO_APP_NAME_SUFFIX}" ${ALLOCV_LIBRARY} ${ALLOCV_LINK_LIBRARIES})
    include_directories(${ALLOCV_INCLUDE_DIR})
else()
  if(NOT ALLOCV_FOUND)
    set(ALLOCV_LIBRARY "")
    set(ALLOCV_INCLUDE_DIR "")
    message("Not building allocv and no usable allocv binary found. Not linking application to allocv")
  endif(NOT ALLOCV_FOUND)
endif(TARGET allocv${DEBUG_SUFFIX})

include_directories(${ALLOCORE_DEP_INCLUDE_DIRS})

target_link_libraries("${ALLO_APP_NAME_SUFFIX}"
  ${ALLOCORE_LIBRARY}
  ${ALLOCORE_LINK_LIBRARIES})


# TODO copy resources to build directory

#list(REMOVE_ITEM PROJECT_RES_FILES ${${ALLO_APP_SRC}})
endfunction(BuildAlloTarget)


function(AddRunTarget TARGET_APP_NAME ALLO_APP_NAME_SUFFIX)
if(NOT RUN_IN_DEBUGGER)
add_custom_target("${TARGET_APP_NAME}_run"
  COMMAND "${ALLO_APP_NAME_SUFFIX}"
  DEPENDS "${ALLO_APP_NAME_SUFFIX}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  SOURCES ${${ALLO_APP_SRC}}
  COMMENT "Running: ${ALLO_APP_NAME_SUFFIX}")
  option(RUN_IN_DEBUGGER 0) # For next run
else()
  if(${ALLOSYSTEM_DEBUGGER} STREQUAL "lldb")
	add_custom_target("${TARGET_APP_NAME}_run"
		COMMAND "${ALLOSYSTEM_DEBUGGER}" "-ex" "${EXECUTABLE_OUTPUT_PATH}/${ALLO_APP_NAME_SUFFIX}"
		DEPENDS "${ALLO_APP_NAME_SUFFIX}"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		SOURCES ${${ALLO_APP_SRC}}
		COMMENT "Running: ${ALLO_APP_NAME_SUFFIX}")
  else()
		add_custom_target("${TARGET_APP_NAME}_run"
		COMMAND "${ALLOSYSTEM_DEBUGGER}" "-ex" "run" "${EXECUTABLE_OUTPUT_PATH}/${ALLO_APP_NAME_SUFFIX}"
		DEPENDS "${ALLO_APP_NAME_SUFFIX}"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		SOURCES ${${ALLO_APP_SRC}}
		COMMENT "Running: ${ALLO_APP_NAME_SUFFIX}")
  endif()
endif(NOT RUN_IN_DEBUGGER)
endfunction()


# --------------- Build

if(BUILD_ALLOSPHERE_APP)

list(APPEND ALLOSPHERE_APPS "")

BuildAlloTarget(APP_NAME ALLOSYSTEM_APP_SRC "${ALLOSYSTEM_CUDA_SRC}" "ALLOSPHERE_BUILD_SIMULATOR" "simulator")
AddRunTarget("${APP_NAME}_simulator" "${APP_NAME}_simulator")
list(APPEND ALLOSPHERE_APPS "${APP_NAME}_simulator")
BuildAlloTarget(APP_NAME ALLOSYSTEM_APP_SRC "${ALLOSYSTEM_CUDA_SRC}" "ALLOSPHERE_BUILD_GRAPHICS_RENDERER" "graphics")
AddRunTarget("${APP_NAME}_graphics" "${APP_NAME}_graphics")
list(APPEND ALLOSPHERE_APPS "${APP_NAME}_graphics")
if(BUILD_ALLOSPHERE_APP_AUDIO_RENDERER)
BuildAlloTarget(APP_NAME ALLOSYSTEM_APP_SRC "${ALLOSYSTEM_CUDA_SRC}" "ALLOSPHERE_BUILD_AUDIO_RENDERER" "audio")
AddRunTarget("${APP_NAME}_audio" "${APP_NAME}_audio")
list(APPEND ALLOSPHERE_APPS "${APP_NAME}_audio")
endif()
set(RUN_CONFIG "{ \"run_dir\" : \"${BUILD_ROOT_DIR}\" , \n  \"apps\" : [ {\"type\" : \"simulator\", \"path\" : \"build/bin/${APP_NAME}_simulator\"},")

if(BUILD_ALLOSPHERE_APP_AUDIO_RENDERER)
set(RUN_CONFIG "${RUN_CONFIG} {\"type\" : \"audio\", \"path\" : \"build/bin/${APP_NAME}_audio\" }, ")
endif()

set(RUN_CONFIG "${RUN_CONFIG} {\"type\" : \"graphics\", \"path\" : \"build/bin/${APP_NAME}_graphics\"} ] \n}")

file(WRITE "${BUILD_ROOT_DIR}/build/${APP_NAME}.json" ${RUN_CONFIG})

add_custom_target("${APP_NAME}"
  COMMAND ""
  DEPENDS ${ALLOSPHERE_APPS}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  SOURCES ${${ALLO_APP_SRC}})

add_custom_target("${APP_NAME}_run"
  COMMAND "python" "tools/allorun/allorun.py" "${BUILD_ROOT_DIR}/build/${APP_NAME}.json"
  DEPENDS ${ALLOSPHERE_APPS}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  SOURCES ${${ALLO_APP_SRC}}
  COMMENT "Running: ${APP_NAME} from ${CMAKE_SOURCE_DIR}")
#  option(RUN_IN_DEBUGGER 0) # For next run

else()
BuildAlloTarget(APP_NAME ALLOSYSTEM_APP_SRC "${ALLOSYSTEM_CUDA_SRC}" "" "")
AddRunTarget("${APP_NAME}" "${APP_NAME}")
endif()


