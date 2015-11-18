cmake_minimum_required (VERSION 2.8)
project (allovtk)

message(STATUS "==== Configuring allovtk")

aux_source_directory(src ALLOVTK_SRC)

set(LIBRARY_OUTPUT_PATH "${BUILD_ROOT_DIR}/build/lib")
set(EXECUTABLE_OUTPUT_PATH "${BUILD_ROOT_DIR}/build/bin")

# Dependencies

set(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}")
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)

find_package(VTK QUIET)

if(NOT VTK_FOUND)
    message(WARNING "VTK not available, not building allovtk")
    return()
endif()

include_directories(${ALLOCORE_DEP_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  "${BUILD_ROOT_DIR}/build/include")

set(ALLOVTK_LIB "allovtk${DEBUG_SUFFIX}")
add_library(${ALLOVTK_LIB} ${ALLOVTK_SRC})

target_link_libraries(${ALLOVTK_LIB} ${ALLOCORE_LIBRARY} ${VTK_LIBRARIES})

#set_target_properties(${ALLOVTK_LIB}
#  PROPERTIES
#  ALLOVTK_INCLUDE_DIR "${BUILD_ROOT_DIR}/build/include"
#  ALLOVTK_LIBRARY ${ALLOVTK_LIB}
#  ALLOVTK_LINK_LIBRARIES "${OpenCV_LIBS}"
#)


# Copy headers to build directory (only if header has changed)
file(GLOB ALLOVTK_INSTALL_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} allovtk/*.h*)
foreach(header ${ALLOVTK_INSTALL_HEADERS})
	execute_process(COMMAND ${CMAKE_COMMAND} -E
					copy_if_different "${header}" "${BUILD_ROOT_DIR}/build/include/${header}"
					WORKING_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

# Examples
if(BUILD_EXAMPLES)
message("--------------------------- examples")
    file(GLOB ALLOVTK_EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/)
    foreach(example ${ALLOVTK_EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      set(EXAMPLE_TARGET allovtk_examples_${EXAMPLE_NAME})
      add_executable(${EXAMPLE_TARGET} ${example})
      target_link_libraries("${EXAMPLE_TARGET}" "allocore${DEBUG_SUFFIX}" ${ALLOCORE_LINK_LIBRARIES} ${ALLOVTK_LIB})
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example})
    endforeach(example)
endif(BUILD_EXAMPLES)

# Installation
install(FILES ${ALLOVTK_INSTALL_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(TARGETS ${ALLOVTK_LIB} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
