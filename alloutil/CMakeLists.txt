cmake_minimum_required (VERSION 2.8)
project (Alloutil)

set(ALLOUTIL_SRC
  src/al_FileWatcher.cpp
  src/al_OmniStereo.cpp
  src/al_ResourceManager.cpp
  src/al_WarpBlend.cpp
)

include_directories(${CMAKE_SOURCE_DIR})
add_library(alloutil ${ALLOUTIL_SRC})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../build/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")
find_package(Lua51 REQUIRED) 
# FIXME find_package(Allocore REQUIRED) 
# FIXME include_directories(${LUA_INCLUDE_DIR} ${ALLOCORE_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/../allocore)
set(ALLOUTIL_LINK_LIBRARIES ${ALLOUTIL_LIBRARIES})
target_link_libraries(alloutil ${ALLOUTIL_LINK_LIBRARIES})

# not ready... FIXME
#file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
#foreach(example ${EXAMPLE_FILES})
#get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
#add_executable(${EXAMPLE_NAME} ${example})
#target_link_libraries(${EXAMPLE_NAME} alloutil ${ALLOUTIL_LINK_LIBRARIES})
#endforeach(example)
