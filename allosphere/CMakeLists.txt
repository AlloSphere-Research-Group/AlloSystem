cmake_minimum_required (VERSION 2.8)
project (AlloSphere)

message(STATUS "==== Configuring allosphere")


##if(USE_CPP_11)
  message(STATUS "Using c++11..")
  if(USE_LIB_CPP)
	message(STATUS "Using libc++..")
	set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
  else(USE_LIB_CPP)
	set(CMAKE_CXX_FLAGS "-std=c++11")
  endif(USE_LIB_CPP)
##endif(USE_CPP_11)

find_package(Cuttlebone QUIET)
if(NOT CUTTLEBONE_FOUND)
  message("Cuttlebone required to build AlloSphere. Not found!")
  return()
endif(NOT CUTTLEBONE_FOUND)

set(ALLOSPHERE_SRC
  "src/al_AlloSphereApp.cpp"
)

set(ALLOSPHERE_HEADERS
  "allosphere/al_AlloSphereApp.hpp"
  )

# Dependencies ------------------------------

set(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX})
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore${DEBUG_SUFFIX} ALLOCORE_LINK_LIBRARIES)

set(ALLOUTIL_LIBRARY alloutil${DEBUG_SUFFIX})
get_target_property(ALLOUTIL_DEP_INCLUDE_DIRS alloutil${DEBUG_SUFFIX} ALLOUTIL_DEP_INCLUDE_DIRS)
get_target_property(ALLOUTIL_LINK_LIBRARIES alloutil${DEBUG_SUFFIX} ALLOUTIL_LINK_LIBRARIES)

# Build library ----------------------------------

set(ALLOSPHERE_LIB "allosphere${DEBUG_SUFFIX}")
add_library(${ALLOSPHERE_LIB} ${ALLOSPHERE_SRC} ${ALLOSPHERE_HEADERS})

# Copy headers to build directory (only if header has changed)
file(GLOB ALLOSPHERE_INSTALL_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} allosphere/*.h*)
foreach(header ${ALLOSPHERE_INSTALL_HEADERS})
	execute_process(COMMAND ${CMAKE_COMMAND} -E
					copy_if_different "${header}" "${BUILD_ROOT_DIR}/build/include/${header}"
					WORKING_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

set(ALLOSPHERE_LINK_LIBRARIES ${ALLOCORE_LIBRARY} ${ALLOCORE_LINK_LIBRARIES} ${ALLOUTIL_LIBRARY} ${ALLOUTIL_LINK_LIBRARIES} ${CUTTLBONE_LIBRARIES})
set(ALLOSPHERE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${ALLOCORE_DEP_INCLUDE_DIRS} ${ALLOUTIL_DEP_INCLUDE_DIRS} ${CUTTLEBONE_INCLUDE_DIR})


include_directories(${ALLOSPHERE_DIRS})
target_link_libraries(${ALLOSPHERE_LIB} ${ALLOSPHERE_LINK_LIBRARIES})

add_dependencies(${ALLOSPHERE_LIB} allocore${DEBUG_SUFFIX})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

#message (STATUS "allocore_location for allosphere == ${ALLOCORE_LIBRARY}")

# export properties for AlloSystem
set_target_properties(${ALLOSPHERE_LIB}
  PROPERTIES
  ALLOSPHERE_DEP_INCLUDE_DIR "${ALLOSPHERE_DIRS}"
)

set_target_properties(${ALLOSPHERE_LIB}
  PROPERTIES
  ALLOSPHERE_LINK_LIBRARIES "${ALLOSPHERE_LINK_LIBRARIES}"
)

set_target_properties(${ALLOSPHERE_LIB}
  PROPERTIES
  ALLOSPHERE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
)

# Build Examples ------------------------------------------------------
if(BUILD_EXAMPLES)
    find_package(LibSndFile REQUIRED QUIET)
  if(NOT TARGET Gamma)
    find_package(Gamma QUIET)
  endif(NOT TARGET Gamma)

  if(NOT (GAMMA_FOUND OR GAMMA_LIBRARY))
    message("Alloutil: Gamma not found. Not building examples." )
  else()

    include_directories(${GAMMA_INCLUDE_DIR})

    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/)
    foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      set(EXAMPLE_TARGET allosphere_examples_${EXAMPLE_NAME})
      add_executable(${EXAMPLE_TARGET} ${example})
      add_dependencies(${EXAMPLE_TARGET} ${ALLOCORE_LIBRARY})
      target_link_libraries("${EXAMPLE_TARGET}" ${ALLOSPHERE_LIB} ${ALLOSPHERE_LINK_LIBRARIES} ${ALLOCORE_LINK_LIBRARIES} ${GAMMA_LIBRARY} ${SNDFILE_LIBRARIES})
      if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties(${EXAMPLE_TARGET} PROPERTIES
	  LINK_FLAGS "-pagezero_size 10000 -image_base 100000000")
      endif()
      if (EXTRA_COMPILER_FLAGS)
# This is the right way, but it crashes cmake... Bug has been fixed by cmake devs but not released as of this writing...
	#set_source_files_properties(${example_src}
        #  PROPERTIES COMPILE_FLAGS "-I/usr/include/mysql")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_COMPILER_FLAGS}")
      endif(EXTRA_COMPILER_FLAGS)
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example})
    endforeach(example)
  endif(NOT (GAMMA_FOUND OR GAMMA_LIBRARY))
endif(BUILD_EXAMPLES)

# installation
install(DIRECTORY allosphere/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/allosphere)
install(TARGETS ${ALLOSPHERE_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
