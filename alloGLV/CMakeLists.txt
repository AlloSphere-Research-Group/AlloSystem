cmake_minimum_required (VERSION 2.8)
project (alloGLV)

message("Building alloGLV.")
set(CMAKE_BUILD_TYPE Release)

aux_source_directory(src ALLOGLV_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(alloGLV ${ALLOGLV_SRC})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Dependencies

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")
find_package(Portaudio REQUIRED)
message("-- Portaudio headers: ${PORTAUDIO_INCLUDE_DIRS}   ${PORTAUDIO_DEFINITIONS}")
find_package(LibSndFile REQUIRED)
message("-- libsndfile headers: ${SNDFILE_INCLUDE_DIRS}")

if(ALLOSYSTEM_BUILD)
set(ALLOCORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../allocore)
set(ALLOCORE_LIBRARY allocore)
get_property(allocore_location TARGET allocore PROPERTY LOCATION)
message (STATUS "allocore_location for alloGLV == ${allocore_location}")
set(GLV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../GLV)
set(GLV_LIBRARY allocore)
get_property(glv_location TARGET GLV PROPERTY LOCATION)
message (STATUS "glv_location for alloGLV == ${glv_location}")
else(ALLOSYSTEM_BUILD)
find_package(Allocore REQUIRED)
find_package(GLV REQUIRED)
endif(ALLOSYSTEM_BUILD)

include_directories(${ALLOCORE_INCLUDE_DIR})
include_directories(${GLV_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(alloGLV ${ALLOCORE_LIBRARIES} ${GLV_LIBRARY})

#if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
## Needed by portaudio
#   find_library(AUDIOUNIT_FM AudioUnit)
#   find_library(COREAUDIO_FM CoreAudio)
#   find_library(CORESERVICES_FM CoreServices)
#   find_library(AUDIOTOOLBOX_FM AudioToolbox)
#   set(PA_FRAMEWORKS ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${CORESERVICES_FM} ${AUDIOTOOLBOX_FM})
#endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Examples\
if(NOT NO_EXAMPLES)
find_package(Gamma)
if(GAMMA_FOUND)
include_directories(${GAMMA_INCLUDE_DIR})
file(GLOB ALLOGLV_EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
foreach(example ${ALLOGLV_EXAMPLE_FILES})
        get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
        add_executable(alloGLV_${EXAMPLE_NAME} ${example})
        target_link_libraries(alloGLV_${EXAMPLE_NAME} allocore ${ALLOCORE_LINK_LIBRARIES} ${GLV_LIBRARY} alloGLV)
        if (apple)
          find_library(AUDIOUNIT_FM AudioUnit)
          target_link_libraries(alloGLV_${EXAMPLE_NAME} ${AUDIOUNIT_FM})
        endif()
endforeach(example)
else()
message("Gamma not found. not building alloGLV examples.")
endif()

endif(NOT NO_EXAMPLES)

install(DIRECTORY alloGLV/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/alloGLV)
install(TARGETS alloGLV DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
