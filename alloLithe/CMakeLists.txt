cmake_minimum_required (VERSION 2.8)
project(allolithe)

message(STATUS "==== Configuring allolithe")

# Depend on Lithe
set(LITHE_DONT_BUILD_TESTS true)
add_subdirectory(Lithe) # TODO: Make this dependency more modular

# Collate all the files
file( GLOB_RECURSE ALLOLITHE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
file( GLOB_RECURSE ALLOLITHE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/allolithe/*)
set(ALLOLITHE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)

# Get Allocore dependency
set(ALLOCORE_LIBRARY "allocore${DEBUG_SUFFIX}")
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)

# Create the allolithe library
add_library( allolithe ${ALLOLITHE_SOURCE})
target_link_libraries( allolithe lithe_core ${ALLOCORE_LIBRARY} Gamma portaudio alloGLV)
target_include_directories( allolithe PUBLIC ${ALLOLITHE_INCLUDE_DIR} ${ALLOCORE_DEP_INCLUDE_DIRS})


# Add the unit tests -- this is excluded from the default make all target
# (to run unit tests use the command: `make test_allolithe`)
add_subdirectory(test EXCLUDE_FROM_ALL)