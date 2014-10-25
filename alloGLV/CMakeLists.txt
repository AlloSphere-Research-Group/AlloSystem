cmake_minimum_required (VERSION 2.8)
project (alloGLV)

message(STATUS "==== Configuring alloGLV")

aux_source_directory(src ALLOGLV_SRC)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Dependencies

get_target_property(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)

find_package(APR QUIET)

if(NOT (APR_LIBRARY AND APR_INCLUDE_DIR))
    message(WARNING "APR not available, not building alloGLV")
    return()
endif()

if(BUILDING_GLV)
    set(GLV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../GLV)
    get_target_property(glv_location GLV LOCATION)
    get_target_property(GLV_LIBRARY GLV GLV_LIBRARY)
    set(GLV_LIBRARY ${glv_location})
    message (STATUS "glv_location for alloGLV == ${glv_location}")
else()
    find_package(GLV QUIET)
    if(NOT GLV_FOUND)
        message("Allosystem: GLV Library NOT found. Not building alloGLV.")
        return()
    endif()
endif()

include_directories(${ALLOCORE_DEP_INCLUDE_DIRS}
  ${GLV_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR})

set(ALLOGLV_LIB "alloGLV${DEBUG_SUFFIX}")
add_library(${ALLOGLV_LIB} ${ALLOGLV_SRC})

target_link_libraries(${ALLOGLV_LIB} ${ALLOCORE_LIBRARIES} ${GLV_LIBRARY})
set_target_properties(${ALLOGLV_LIB}
  PROPERTIES 
  ALLOGLV_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  ALLOGLV_LIBRARY ${ALLOGLV_LIB})

set_target_properties(${ALLOGLV_LIB}
  PROPERTIES 
  ALLOGLV_LINK_LIBRARIES "${GLV_LIBRARY}"
  )

# Examples
if(BUILD_EXAMPLES)
  find_package(Gamma QUIET)
  if(GAMMA_FOUND)
    include_directories(${GAMMA_INCLUDE_DIR})
    file(GLOB ALLOGLV_EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/)
    foreach(example ${ALLOGLV_EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      set(EXAMPLE_TARGET alloGLV_examples_${EXAMPLE_NAME})
      add_executable(${EXAMPLE_TARGET} ${example})
      target_link_libraries("${EXAMPLE_TARGET}" "allocore${DEBUG_SUFFIX}" ${ALLOCORE_LINK_LIBRARIES} ${GLV_LIBRARY} ${ALLOGLV_LIB})
      if (apple)
        find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries(${EXAMPLE_TARGET} ${AUDIOUNIT_FM})
      endif()
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example})
    endforeach(example)
  else()
    message("Gamma not found. not building alloGLV examples.")
  endif()

endif(BUILD_EXAMPLES)

install(DIRECTORY alloGLV/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/alloGLV)
install(TARGETS ${ALLOGLV_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
