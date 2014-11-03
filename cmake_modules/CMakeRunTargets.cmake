# Basic checks
#if(NOT (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})) # use only if cmake was run from the root directory
#    message(FATAL_ERROR "Error: The run script must be called from the source root directory." )
#endif(NOT (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR}))

string(REGEX MATCH ".*\\*.*" match "${CMAKE_CURRENT_SOURCE_DIR}")
IF(NOT ${match} STREQUAL "")
  message(FATAL_ERROR "Error: Please remove '*' from path!" ) # This avoids issues with the run script
ENDIF()

if(BUILD_DIR)
  string(REGEX REPLACE "/+$" "" ALLOSYSTEM_BUILD_APP_DIR "${ALLOSYSTEM_BUILD_APP_DIR}") # remove trailing slash
  file(GLOB ALLOPROJECT_APP_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ALLOSYSTEM_BUILD_APP_DIR}/*.cpp)
  string(REPLACE "/" "_" APP_NAME "${ALLOSYSTEM_BUILD_APP_DIR}")
  string(REGEX REPLACE "_+$" "" APP_NAME "${APP_NAME}")
  set(SOURCE_DIR "${ALLOSYSTEM_BUILD_APP_DIR}")
else()
  set(ALLOPROJECT_APP_SRC "${BUILD_APP_FILE}")
  string(REPLACE "/" "_" APP_NAME "${BUILD_APP_FILE}")
  get_filename_component(APP_NAME "${APP_NAME}" NAME)
  STRING(REGEX REPLACE "\\.[^.]*\$" "" APP_NAME "${APP_NAME}")
  string(REPLACE "." "_" APP_NAME "${APP_NAME}")
#  get_filename_component(APP_NAME ${APP_NAME} NAME_WE) # Get name w/o extension (extension is anything after first dot!)
  get_filename_component(SOURCE_DIR "${BUILD_APP_FILE}" PATH)
endif(BUILD_DIR)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

if(EXISTS "${SOURCE_DIR}/flags.cmake")
    include("${SOURCE_DIR}/flags.cmake")
endif()

add_executable("${APP_NAME}" EXCLUDE_FROM_ALL ${ALLOPROJECT_APP_SRC})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(${APP_NAME} PROPERTIES
    LINK_FLAGS "-pagezero_size 10000 -image_base 100000000")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(EXISTS "${SOURCE_DIR}/flags.txt")
  file(READ "${SOURCE_DIR}/flags.txt" EXTRA_COMPILER_FLAGS)
  STRING(REGEX REPLACE "[\r\n]" " " EXTRA_COMPILER_FLAGS "${EXTRA_COMPILER_FLAGS}")
  set_target_properties(${APP_NAME} PROPERTIES
    COMPILE_FLAGS "${EXTRA_COMPILER_FLAGS}")
  message(STATUS "NOTE: Using additional flags from ${SOURCE_DIR}/flags.txt: ${EXTRA_COMPILER_FLAGS}")
endif()

message(STATUS "Target: ${APP_NAME}")
message(STATUS "From sources: ${ALLOPROJECT_APP_SRC}")

# Dependencies (check if targets exist and set variables)
get_target_property(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore${DEBUG_SUFFIX} ALLOCORE_LINK_LIBRARIES)
add_dependencies("${APP_NAME}" allocore${DEBUG_SUFFIX})

#message("Using allocore headers from: ${ALLOCORE_DEP_INCLUDE_DIRS}")

if(BUILDING_Gamma)
    get_target_property(GAMMA_LIBRARY Gamma LOCATION)
    add_dependencies(${APP_NAME} Gamma)
    target_link_libraries(${APP_NAME} ${GAMMA_LIBRARY})
    include_directories(${GAMMA_INCLUDE_DIR})
else()
  if(NOT GAMMA_FOUND)
    set(GAMMA_LIBRARY "")
    set(GAMMA_INCLUDE_DIR "")
    message("Not building GAMMA and no usable GAMMA binary found. Not linking application to GAMMA")
  endif(NOT GAMMA_FOUND)  
endif(BUILDING_Gamma)

if(BUILDING_GLV)
    get_target_property(GLV_LIBRARY GLV LOCATION)
    add_dependencies(${APP_NAME} GLV)
    target_link_libraries(${APP_NAME} ${GLV_LIBRARY})
    include_directories(${GLV_INCLUDE_DIR})
else()
  if(NOT GLV_FOUND)
    set(GLV_LIBRARY "")
    set(GLV_INCLUDE_DIR "")
    message("Not building GLV and no usable GLV binary found. Not linking application to GLV")
  endif(NOT GLV_FOUND)  
endif(BUILDING_GLV)

if(TARGET alloutil${DEBUG_SUFFIX})
    get_target_property(ALLOUTIL_LIBRARY alloutil${DEBUG_SUFFIX} LOCATION)
    get_target_property(ALLOUTIL_DEP_INCLUDE_DIR alloutil${DEBUG_SUFFIX} ALLOUTIL_DEP_INCLUDE_DIR)
    get_target_property(ALLOUTIL_LINK_LIBRARIES alloutil${DEBUG_SUFFIX} ALLOUTIL_LINK_LIBRARIES)
    add_dependencies("${APP_NAME}" alloutil${DEBUG_SUFFIX})
    target_link_libraries("${APP_NAME}" ${ALLOUTIL_LIBRARY} ${ALLOUTIL_LINK_LIBRARIES})
    include_directories(${ALLOUTIL_DEP_INCLUDE_DIR})
else()
  if(NOT ALLOUTIL_FOUND)
    set(ALLOUTIL_LIBRARY "")
    set(ALLOUTIL_DEP_INCLUDE_DIR "")
    message("Not building ALLOUTIL and no usable ALLOUTIL binary found. Not linking application to ALLOUTIL")
  endif(NOT ALLOUTIL_FOUND) 
endif(TARGET alloutil${DEBUG_SUFFIX})

if(TARGET alloGLV${DEBUG_SUFFIX})
    get_target_property(ALLOGLV_LIBRARY alloGLV${DEBUG_SUFFIX} LOCATION)
    get_target_property(ALLOGLV_INCLUDE_DIR alloGLV${DEBUG_SUFFIX} ALLOGLV_INCLUDE_DIR)
    get_target_property(ALLOGLV_LINK_LIBRARIES "alloGLV${DEBUG_SUFFIX}" ALLOGLV_LINK_LIBRARIES)
    add_dependencies("${APP_NAME}" alloGLV${DEBUG_SUFFIX})
    target_link_libraries("${APP_NAME}" ${ALLOGLV_LIBRARY} ${ALLOGLV_LINK_LIBRARIES})
    include_directories(${ALLOGLV_INCLUDE_DIR})
else()
  if(NOT ALLOGLV_FOUND)
    set(ALLOGLV_LIBRARY "")
    set(ALLOGLV_INCLUDE_DIR "")
    message("Not building alloGLV and no usable alloGLV binary found. Not linking application to alloGLV")
  endif(NOT ALLOGLV_FOUND) 
endif(TARGET alloGLV${DEBUG_SUFFIX})


include_directories(${ALLOCORE_DEP_INCLUDE_DIRS})

# TODO copy resources to build directory

#    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
target_link_libraries("${APP_NAME}"
  ${ALLOCORE_LIBRARY}
  ${ALLOCORE_LINK_LIBRARIES})

#list(REMOVE_ITEM PROJECT_RES_FILES ${ALLOPROJECT_APP_SRC})

if(NOT RUN_IN_DEBUGGER)
add_custom_target("${APP_NAME}_run"
  COMMAND "${APP_NAME}"
  DEPENDS "${APP_NAME}"
  WORKING_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}"
  SOURCES ${ALLOPROJECT_APP_SRC}
  COMMENT "Running: ${APP_NAME}")
  option(RUN_IN_DEBUGGER 0) # For next run
else()
add_custom_target("${APP_NAME}_run"
  COMMAND "${ALLOSYSTEM_DEBUGGER}" "-ex" "run" "${EXECUTABLE_OUTPUT_PATH}/${APP_NAME}"
  DEPENDS "${APP_NAME}"
  WORKING_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}"
  SOURCES ${ALLOPROJECT_APP_SRC}
  COMMENT "Running: ${APP_NAME}")

endif(NOT RUN_IN_DEBUGGER)
