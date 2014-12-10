find_library(CUTTLEBONE_LIB
	NAMES cuttlebone
	PATHS ${CMAKE_SOURCE_DIR}/../cuttlebone/build/ /usr/local/lib)

find_path(CUTTLEBONE_INCLUDE_DIR
	NAMES Cuttlebone/Cuttlebone.hpp
	PATHS ${CMAKE_SOURCE_DIR}/../cuttlebone/ /usr/local/include)

if(NOT(CUTTLEBONE_INCLUDE_DIR AND CUTTLEBONE_LIB))
    message("Cuttlebone not found. Can't build application")
    return()
else(NOT(CUTTLEBONE_INCLUDE_DIR AND CUTTLEBONE_LIB))
	message("Cuttlebone found.")
endif(NOT(CUTTLEBONE_INCLUDE_DIR AND CUTTLEBONE_LIB))

include_directories(${CUTTLEBONE_INCLUDE_DIR})

target_link_libraries("${APP_NAME}" ${CUTTLEBONE_LIB})

# You can also use all of CMake here:

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
