
find_package(Portaudio QUIET)

set(PORTAUDIO_HEADERS
    allocore/io/al_AudioIO.hpp
    allocore/sound/al_Ambisonics.hpp
    allocore/sound/al_AudioScene.hpp
    allocore/sound/al_Crossover.hpp
    allocore/sound/al_Dbap.hpp
    allocore/sound/al_Reverb.hpp
    allocore/sound/al_Speaker.hpp
    allocore/sound/al_Vbap.hpp
    allocore/sound/al_Biquad.hpp
    allocore/sound/al_Crossover.hpp
    allocore/sound/al_Speaker.hpp
    allocore/sound/al_StereoPanner.hpp
)

if(PORTAUDIO_LIBRARY AND PORTAUDIO_INCLUDE_DIR)
message(STATUS "Building Portaudio module.")

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
    src/sound/al_AudioScene.cpp
    src/sound/al_Ambisonics.cpp
    src/sound/al_Dbap.cpp
    src/sound/al_Vbap.cpp
    src/sound/al_Biquad.cpp
)

list(APPEND ALLOCORE_HEADERS ${PORTAUDIO_HEADERS})

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${PORTAUDIO_INCLUDE_DIR}
  ${PORTAUDIO_INCLUDE_DIRS})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${PORTAUDIO_LIBRARY}
  ${PORTAUDIO_LIBRARIES})

else()
    message("NOT Building Portaudio module.")
    foreach(header ${PORTAUDIO_HEADERS})
        list(APPEND PORTAUDIO_DUMMY_HEADER_INFO "${header}::::Portaudio")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${PORTAUDIO_DUMMY_HEADER_INFO})

endif(PORTAUDIO_LIBRARY AND PORTAUDIO_INCLUDE_DIR)
