cmake_minimum_required (VERSION 2.8)
project (Alloutil)

message("==== Configuring alloutil.")
set(CMAKE_BUILD_TYPE Release)

set(ALLOUTIL_BUILD_EXAMPLES 1)

set(ALLOUTIL_SRC
  src/al_FileWatcher.cpp
  src/al_OmniStereo.cpp
  src/al_ResourceManager.cpp
  src/al_WarpBlend.cpp
  )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(alloutil ${ALLOUTIL_SRC})
set(ALLOUTIL_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${LUA_INCLUDE_DIR})
set_target_properties(alloutil 
  PROPERTIES 
  ALLOUTIL_INCLUDE_DIR "${ALLOUTIL_DIRS}"
  )

find_package(Lua51 REQUIRED)

GET_DIRECTORY_PROPERTY(ALLOSYSTEM_BUILD DIRECTORY ${CMAKE_SOURCE_DIR}
  DEFINITION ALLOSYSTEM_BUILD)

if(ALLOSYSTEM_BUILD)
  #message("Allosystem build")
  get_target_property(ALLOCORE_LIBRARY allocore LOCATION)
  get_target_property(ALLOCORE_INCLUDE_DIR allocore ALLOCORE_INCLUDE_DIR)
else(ALLOSYSTEM_BUILD)
  find_package(Allocore REQUIRED)
endif(ALLOSYSTEM_BUILD)


include_directories(${LUA_INCLUDE_DIR} ${ALLOCORE_INCLUDE_DIR})
set(ALLOUTIL_LINK_LIBRARIES ${LUA_LIBRARIES} ${ALLOCORE_LIBRARY})


set_target_properties(alloutil 
  PROPERTIES 
  ALLOUTIL_LINK_LIBRARIES "${ALLOUTIL_LINK_LIBRARIES}"
  )

target_link_libraries(alloutil ${ALLOUTIL_LINK_LIBRARIES})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

message (STATUS "allocore_location for alloutil == ${ALLOCORE_LIBRARY}")

# Examples
if(NOT NO_EXAMPLES)
    find_package(APR REQUIRED QUIET)
    find_package(Assimp REQUIRED QUIET)
    find_package(FreeImage REQUIRED QUIET)
    find_package(Freetype REQUIRED QUIET)
    find_package(GLEW REQUIRED QUIET)
    find_package(GLUT REQUIRED QUIET)
    find_package(LibSndFile REQUIRED QUIET)
    find_package(OpenGL REQUIRED QUIET)
    find_package(Portaudio REQUIRED QUIET)
  if(NOT ALLOSYSTEM_BUILD)
    find_package(Gamma QUIET)
  endif(NOT ALLOSYSTEM_BUILD)
  
  if(NOT (GAMMA_FOUND OR GAMMA_LIBRARY))
    message("Alloutil: Gamma not found. Not building examples." )
  else()

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

    set(ALLOCORE_LINK_LIBRARIES ${ALLOCORE_LINK_LIBRARIES}
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
      ${GAMMA_LIBRARY}
      ${SNDFILE_LIBRARIES}
      ${PA_FRAMEWORKS}
      ${LINUX_LIBS}
      ${EXAMPLE_LINUX_LIBS})

    include_directories(${GAMMA_INCLUDE_DIR})

    file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/)
    foreach(example ${EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      if (ALLOSYSTEM_BUILD)
	set(EXAMPLE_TARGET alloutil_examples_${EXAMPLE_NAME})
      else(ALLOSYSTEM_BUILD)
	set(EXAMPLE_TARGET examples_${EXAMPLE_NAME})
      endif(ALLOSYSTEM_BUILD)
      add_executable(${EXAMPLE_TARGET} ${example})
      target_link_libraries("${EXAMPLE_TARGET}" alloutil ${ALLOUTIL_LINK_LIBRARIES} ${ALLOCORE_LINK_LIBRARIES})
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example})
    endforeach(example)
  endif(NOT (GAMMA_FOUND OR GAMMA_LIBRARY))
endif(NOT NO_EXAMPLES)


# installation
install(DIRECTORY alloutil/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/alloutil)
install(TARGETS alloutil DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)


