cmake_minimum_required (VERSION 2.8)
project (Allovsr)

set(ALLOVSR_BUILD_EXAMPLES 1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")

# --- Check for allocore
GET_DIRECTORY_PROPERTY(ALLOSYSTEM_BUILD DIRECTORY ${CMAKE_SOURCE_DIR}
DEFINITION ALLOSYSTEM_BUILD)

if(ALLOSYSTEM_BUILD)
message("Configuring allovsr")
set(ALLOCORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../allocore)
set(ALLOCORE_LIBRARY allocore)
set(ALLOGLV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../alloGLV)
set(ALLOGLV_LIBRARY alloGLV)
get_property(allocore_location TARGET allocore PROPERTY LOCATION)
message (STATUS "allocore_location for allovsr == ${allocore_location}")
else()
find_package(Allocore REQUIRED)
find_package(AlloGLV REQUIRED)
endif(ALLOSYSTEM_BUILD)


# --- Check for vsr
find_package(vsr REQUIRED)
message (STATUS "vsr_location for allovsr == ${VSR_INCLUDE_DIR}")

# --- configure
message ("${ALLOGLV_INCLUDE_DIR}")
include_directories(${ALLOCORE_INCLUDE_DIR} ${ALLOGLV_INCLUDE_DIR} ${VSR_INCLUDE_DIR})
set(ALLOVSR_LINK_LIBRARIES ${ALLOCORE_LIBRARY} ${ALLOGLV_LIBRARY} ${VSR_LIBRARY} ${GLV_LIBRARY})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Examples
if(NOT NO_EXAMPLES)
find_package(APR REQUIRED)
find_package(Assimp REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(LibSndFile REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Portaudio REQUIRED)
find_package(Gamma REQUIRED)
find_package(GLV REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# Needed by portaudio
add_definitions(-DAL_OSX)
   find_library(AUDIOUNIT_FM AudioUnit)
   find_library(COREAUDIO_FM CoreAudio)
   find_library(COREMIDI_FM CoreMidi)
   find_library(CORESERVICES_FM CoreServices)
   find_library(AUDIOTOOLBOX_FM AudioToolbox)
   set(PA_FRAMEWORKS ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${COREMIDI_FM} ${CORESERVICES_FM} ${AUDIOTOOLBOX_FM})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(EXAMPLE_LINUX_LIBS asound pthread)

add_definitions(-DAL_LINUX)
set(ALLOCORE_LINK_LIBRARIES ${ALLOCORE_LIBRARIES} avahi-common avahi-client)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
add_definitions(-DAL_WINDOWS)
endif()

set(ALLOCORE_LINK_LIBRARIES
${APR_LIBRARY}
${ASSIMP_LIBRARY}
${FREEIMAGE_LIBRARY}
${FREETYPE_LIBRARY}
${GLEW_LIBRARY}
${GLUT_LIBRARY}
${LIBSNDFILE_LIBRARY}
${OPENGL_LIBRARY}
${PORTAUDIO_LIBRARY}
${PORTAUDIO_LIBRARIES}
${GAMMA_LIBRARIES}
${SNDFILE_LIBRARIES}
${GLV_LIBRARY}
${PA_FRAMEWORKS}
${LINUX_LIBS}
${EXAMPLE_LINUX_LIBS}
${ALLOCORE_LINK_LIBRARIES}
)

include_directories(${GAMMA_INCLUDE_DIRS} ${GLV_INCLUDE_DIRS})

file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
foreach(example ${EXAMPLE_FILES})
get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
add_executable(allovsr_${EXAMPLE_NAME} ${example})
target_link_libraries(allovsr_${EXAMPLE_NAME} ${ALLOVSR_LINK_LIBRARIES} ${ALLOCORE_LINK_LIBRARIES})
endforeach(example)
endif(NOT NO_EXAMPLES)


# installation
install(DIRECTORY allovsr/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/allovsr)
#install(TARGETS allovsr DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)


