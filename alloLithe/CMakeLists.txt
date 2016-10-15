cmake_minimum_required (VERSION 2.8)
project(alloLithe)

message(STATUS "==== Configuring alloLithe")

# Depend on Lithe
set(LITHE_DONT_BUILD_TESTS true)
add_subdirectory(Lithe)

# Collate all the files
file( GLOB_RECURSE ALLOLITHE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file( GLOB_RECURSE ALLOLITHE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/alloLithe/*)

# Get Allocore dependency
set(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}")
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)

add_library( alloLithe ${ALLOLITHE_SOURCE})
target_link_libraries( alloLithe lithe_core ${ALLOCORE_LIBRARY} Gamma portaudio alloGLV)
target_include_directories(alloLithe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)