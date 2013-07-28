project( AlloSystem )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

add_subdirectory(allocore)

set(ALLOSYSTEM_BUILD 1) # Bulding all of allosystem

# check for Lua to build alloutil
find_package(Lua51)
if(LUA51_FOUND)
add_subdirectory(alloutil)
endif(LUA51_FOUND)

# check for GLV to build alloGLV
set(GLV_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../GLV)

if(EXISTS "${GLV_RELATIVE_DIR}" AND IS_DIRECTORY "${GLV_RELATIVE_DIR}")
  message("Building GLV")
  set(BUILDING_GLV 1)
  add_subdirectory(${CMAKE_SOURCE_DIR}/../GLV ${CMAKE_SOURCE_DIR}/../GLV-build)
  add_subdirectory( alloGLV )
else()
find_package(GLV)
  if(${GLV_FOUND})
    add_subdirectory( alloGLV )
  endif()
endif()


# check for vsr to build allovsr
set(VSR_RELATIVE_DIR ${CMAKE_SOURCE_DIR}/../vsr)

if(EXISTS "${VSR_RELATIVE_DIR}" AND IS_DIRECTORY "${VSR_RELATIVE_DIR}")
  message("Building VSR")
  set(BUILDING_VSR 1)
  add_subdirectory(${CMAKE_SOURCE_DIR}/../vsr ${CMAKE_SOURCE_DIR}/../vsr-build)
  add_subdirectory( allovsr )
else()
find_package(vsr)
  if(${VSR_FOUND})
    add_subdirectory( allovsr )
  endif()
endif()

#from old Makefile

#Linux
#-lm -lstdc++

#OS X
#ARCH = 64
#CXX = @clang++-mp-3.1 -std=c++11

#Windows
#	# Needed for MinGW libs...
#	LDFLAGS  += -L/mingw/lib/
#	CPPFLAGS += -I/mingw/include/

#	# Needed for MSYS libs...
#	LDFLAGS  += -L/local/lib/
#	CPPFLAGS += -I/local/include/
#	CPPFLAGS += -I`pwd`
