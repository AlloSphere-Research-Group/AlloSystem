cmake_minimum_required (VERSION 2.8)
project (allocv)

message(STATUS "==== Configuring allocv")

aux_source_directory(src ALLOCV_SRC)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Dependencies

get_target_property(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)

find_package(OpenCV QUIET)

if(NOT (OpenCV_LIBS AND OpenCV_INCLUDE_DIRS))
    message(WARNING "OpenCV not available, not building allocv")
    return()
endif()

include_directories(${ALLOCORE_DEP_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR})

set(ALLOCV_LIB "allocv${DEBUG_SUFFIX}")
add_library(${ALLOCV_LIB} ${ALLOCV_SRC})

target_link_libraries(${ALLOCV_LIB} ${ALLOCORE_LIBRARY} ${OpenCV_LIBS})

set_target_properties(${ALLOCV_LIB}
  PROPERTIES
  ALLOCV_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  ALLOCV_LIBRARY ${ALLOCV_LIB}
  ALLOCV_LINK_LIBRARIES "${OpenCV_LIBS}"
)

# Examples
#if(BUILD_EXAMPLES)
#  find_package(Gamma QUIET)
#  if(GAMMA_FOUND)
#    include_directories(${GAMMA_INCLUDE_DIR})
#    file(GLOB ALLOGLV_EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
#    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/)
#    foreach(example ${ALLOGLV_EXAMPLE_FILES})
#      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
#      set(EXAMPLE_TARGET alloGLV_examples_${EXAMPLE_NAME})
#      add_executable(${EXAMPLE_TARGET} ${example})
#      target_link_libraries("${EXAMPLE_TARGET}" "allocore${DEBUG_SUFFIX}" ${ALLOCORE_LINK_LIBRARIES} ${GLV_LIBRARY} ${ALLOGLV_LIB})
#      if (apple)
#        find_library(AUDIOUNIT_FM AudioUnit)
#        target_link_libraries(${EXAMPLE_TARGET} ${AUDIOUNIT_FM})
#      endif()
#      add_custom_target("${EXAMPLE_TARGET}_run"
#	COMMAND "${EXAMPLE_TARGET}"
#	DEPENDS "${EXAMPLE_TARGET}"
#	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
#	SOURCES ${example})
#    endforeach(example)
#  else()
#    message("Gamma not found. not building alloGLV examples.")
#  endif()

#endif(BUILD_EXAMPLES)

install(DIRECTORY allocv/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/allocv)
install(TARGETS ${ALLOCV_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
