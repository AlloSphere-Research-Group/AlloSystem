cmake_minimum_required (VERSION 2.8)
project (alloGLV)

message("==== Configuring alloGLV")
set(CMAKE_BUILD_TYPE Release)

aux_source_directory(src ALLOGLV_SRC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(alloGLV ${ALLOGLV_SRC})

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build/bin)

# Dependencies

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake_modules")

if(ALLOSYSTEM_BUILD)
  get_target_property(ALLOCORE_LIBRARY allocore LOCATION)
  get_target_property(ALLOCORE_INCLUDE_DIR allocore ALLOCORE_INCLUDE_DIR)
  set(GLV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../GLV)
  get_property(glv_location TARGET GLV PROPERTY LOCATION)
  set(GLV_LIBRARY ${glv_location})
  message (STATUS "glv_location for alloGLV == ${glv_location}")
else()
  find_package(Allocore REQUIRED QUIET)
  find_package(GLV REQUIRED QUIET)
endif(ALLOSYSTEM_BUILD)

message (STATUS "allocore_location for alloGLV == ${allocore_location}")

include_directories(${ALLOCORE_INCLUDE_DIR}
  ${GLV_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(alloGLV ${ALLOCORE_LIBRARIES} ${GLV_LIBRARY})
set_target_properties(alloGLV 
  PROPERTIES 
  ALLOGLV_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  ALLOGLV_LIBRARY alloGLV)

set_target_properties(alloGLV 
  PROPERTIES 
  ALLOGLV_LINK_LIBRARIES "${ALLOGLV_LINK_LIBRARIES}"
  )


#if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
## Needed by portaudio
#   find_library(AUDIOUNIT_FM AudioUnit)
#   find_library(COREAUDIO_FM CoreAudio)
#   find_library(CORESERVICES_FM CoreServices)
#   find_library(AUDIOTOOLBOX_FM AudioToolbox)
#   set(PA_FRAMEWORKS ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${CORESERVICES_FM} ${AUDIOTOOLBOX_FM})
#endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Examples
if(NOT NO_EXAMPLES)
  find_package(Gamma QUIET)
  if(GAMMA_FOUND)
    include_directories(${GAMMA_INCLUDE_DIR})
    file(GLOB ALLOGLV_EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} examples/*.cpp)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/examples/)
    foreach(example ${ALLOGLV_EXAMPLE_FILES})
      get_filename_component(EXAMPLE_NAME ${example} NAME_WE) # Get name w/o extension
      if (ALLOSYSTEM_BUILD)
	set(EXAMPLE_TARGET alloGLV_examples_${EXAMPLE_NAME})
      else(ALLOSYSTEM_BUILD)
	set(EXAMPLE_TARGET examples_${EXAMPLE_NAME})
      endif(ALLOSYSTEM_BUILD)
      add_executable(${EXAMPLE_TARGET} ${example})
      target_link_libraries("${EXAMPLE_TARGET}" allocore ${ALLOCORE_LINK_LIBRARIES} ${GLV_LIBRARY} alloGLV)
      if (apple)
        find_library(AUDIOUNIT_FM AudioUnit)
        target_link_libraries(${EXAMPLE_TARGET} ${AUDIOUNIT_FM})
      endif()
      add_custom_target("${EXAMPLE_TARGET}_run"
	COMMAND "${EXAMPLE_TARGET}"
	DEPENDS "${EXAMPLE_TARGET}"
	WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
	SOURCES ${example})
    endforeach(example)
  else()
    message("Gamma not found. not building alloGLV examples.")
  endif()

endif(NOT NO_EXAMPLES)

install(DIRECTORY alloGLV/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/alloGLV)
install(TARGETS alloGLV DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
