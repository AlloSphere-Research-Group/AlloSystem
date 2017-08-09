cmake_minimum_required (VERSION 2.8)
project (Alloutil)

message(STATUS "==== Configuring alloutil")

set(ALLOUTIL_SRC
  src/al_FileWatcher.cpp
  src/al_OmniStereo.cpp
  src/al_ResourceManager.cpp
  src/al_WarpBlend.cpp
  src/al_RayStereo.cpp
  src/al_InterfaceServerClient.cpp
  src/al_Simulator.cpp
  src/al_SliceViewer.cpp
  )

# Dependencies ------------------------------
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(NOT (COMPILER_SUPPORTS_CXX11 OR COMPILER_SUPPORTS_CXX0X))
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

find_package(LuaJIT)

if(LUAJIT_FOUND)
  set(LUA_INCLUDE_DIR ${LUAJIT_INCLUDE_DIR})
  set(LUA_LIBRARIES ${LUAJIT_LIBRARIES})
  set(LUA_FOUND 1)
else()
  find_package(Lua51)
  if (LUA_LIBRARY)
    set(LUA_FOUND 1)
  endif(LUA_LIBRARY)
endif(LUAJIT_FOUND)

if(NOT LUA_FOUND)
  message("Lua 5.1 or LuaJIT required to build AlloUtil. Neither found!")
  return()
endif(NOT LUA_FOUND)

find_package(Jsoncpp)

if(NOT Jsoncpp_FOUND)
  message("Jsoncpp required to build AlloUtil. Not found!")
  return()
endif(NOT Jsoncpp_FOUND)

find_path(ALLOCORE_HEADER allocore/graphics/al_Image.hpp ${BUILD_ROOT_DIR}/build/include)

if(NOT ALLOCORE_HEADER)
    message("Allocore not built with freeimage support. Can't build Alloutil.")
    return()
endif(NOT ALLOCORE_HEADER)

set(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}")
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore${DEBUG_SUFFIX} ALLOCORE_LINK_LIBRARIES)

# Copy headers to build directory (only if header has changed)
file(GLOB ALLOUTIL_INSTALL_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} alloutil/*.h*)
foreach(header ${ALLOUTIL_INSTALL_HEADERS})
	execute_process(COMMAND ${CMAKE_COMMAND} -E
					copy_if_different "${header}" "${BUILD_ROOT_DIR}/build/include/${header}"
					WORKING_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

# Build library ----------------------------------

set(ALLOUTIL_LIB "alloutil${DEBUG_SUFFIX}")
add_library(${ALLOUTIL_LIB} ${ALLOUTIL_SRC} ${ALLOUTIL_INSTALL_HEADERS})

set(ALLOUTIL_LINK_LIBRARIES ${LUA_LIBRARIES} ${Jsoncpp_LIBRARIES} ${ALLOCORE_LIBRARY})
set(ALLOUTIL_DIRS "${BUILD_ROOT_DIR}/build/include" ${LUA_INCLUDE_DIR} ${Jsoncpp_INCLUDE_DIRS} ${ALLOCORE_DEP_INCLUDE_DIRS})

include_directories(${ALLOUTIL_DIRS})
target_link_libraries(${ALLOUTIL_LIB} ${ALLOUTIL_LINK_LIBRARIES})

add_dependencies(${ALLOUTIL_LIB} allocore${DEBUG_SUFFIX})

set(LIBRARY_OUTPUT_PATH "${BUILD_ROOT_DIR}/build/lib")
set(EXECUTABLE_OUTPUT_PATH "${BUILD_ROOT_DIR}/build/bin")

#message (STATUS "allocore_location for alloutil == ${ALLOCORE_LIBRARY}")

# export properties for AlloSystem
set_target_properties(${ALLOUTIL_LIB}
  PROPERTIES
  ALLOUTIL_DEP_INCLUDE_DIRS "${ALLOUTIL_DIRS}"
  )
set_target_properties(${ALLOUTIL_LIB}
  PROPERTIES
  ALLOUTIL_LINK_LIBRARIES "${ALLOUTIL_LINK_LIBRARIES}"
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
	list(REMOVE_ITEM EXAMPLE_FILES "examples/VCR.cpp") #FIXME put back example when fixed
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_ROOT_DIR}/build/bin/examples/)
    foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      set(EXAMPLE_TARGET alloutil_examples_${EXAMPLE_NAME})
      add_executable(${EXAMPLE_TARGET} ${example})
      add_dependencies(${EXAMPLE_TARGET} ${ALLOCORE_LIBRARY})
      target_link_libraries("${EXAMPLE_TARGET}" ${ALLOUTIL_LIB} ${ALLOUTIL_LINK_LIBRARIES} ${ALLOCORE_LINK_LIBRARIES} ${GAMMA_LIBRARY} ${SNDFILE_LIBRARIES})
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
install(FILES ${ALLOUTIL_INSTALL_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(TARGETS ${ALLOUTIL_LIB} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
