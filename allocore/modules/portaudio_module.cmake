
find_package(Portaudio QUIET)

if(PORTAUDIO_LIBRARY)
message("Building Portaudio module.")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_library(AUDIOUNIT_FM AudioUnit)
  find_library(COREAUDIO_FM CoreAudio)
  find_library(COREMIDI_FM CoreMidi)
  find_library(CORESERVICES_FM CoreServices)
  find_library(AUDIOTOOLBOX_FM AudioToolbox)
  list(APPEND ALLOCORE_LINK_LIBRARIES
    ${AUDIOUNIT_FM} ${COREAUDIO_FM} ${COREMIDI_FM} ${CORESERVICES_FM}
    ${AUDIOTOOLBOX_FM})

endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND ALLOCORE_LINK_LIBRARIES asound)
endif()

list(APPEND ALLOCORE_SRC
  src/io/al_AudioIO.cpp
)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${PORTAUDIO_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${PORTAUDIO_LIBRARY}
  ${PORTAUDIO_LIBRARIES})

else()
message("NOT Building Portaudio module.")
endif(PORTAUDIO_LIBRARY)
