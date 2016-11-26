set(BUILD_OSC 1)


if(BUILD_OSC)
message(STATUS "Building OSC module.")

set(OSC_HEADERS
    allocore/protocol/al_OSC.hpp
    allocore/ui/al_Parameter.hpp
	allocore/ui/al_Preset.hpp
	allocore/ui/al_HtmlInterfaceServer.hpp
	allocore/ui/al_ParameterMIDI.hpp
	allocore/ui/al_PresetMIDI.hpp
	allocore/ui/al_PresetSequencer.hpp
	allocore/ui/al_SequenceRecorder.hpp
	allocore/ui/al_Composition.hpp
	allocore/ui/al_PresetMapper.hpp
)

# for oscpack (oscpack sources are included with AlloSystem)
include(TestBigEndian)
test_big_endian(TEST_ENDIAN)
if (TEST_ENDIAN MATCHES 0)
  add_definitions(-DOSC_HOST_LITTLE_ENDIAN)
else()
  add_definitions(-DOSC_HOST_BIG_ENDIAN)
endif()

set(OSCPACK_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/")

list(APPEND ALLOCORE_SRC
  ${OSCPACK_ROOT_DIR}/oscpack/osc/OscOutboundPacketStream.cpp
  ${OSCPACK_ROOT_DIR}/oscpack/osc/OscPrintReceivedElements.cpp
  ${OSCPACK_ROOT_DIR}/oscpack/osc/OscReceivedElements.cpp
  ${OSCPACK_ROOT_DIR}/oscpack/osc/OscTypes.cpp
  src/protocol/al_OSC.cpp
  src/ui/al_Parameter.cpp
  src/ui/al_Preset.cpp
  src/ui/al_PresetMIDI.cpp
  src/ui/al_HtmlInterfaceServer.cpp
  src/ui/al_PresetSequencer.cpp
  src/ui/al_SequenceRecorder.cpp
  src/ui/al_Composition.cpp
  src/ui/al_PresetMapper.cpp
  )

INCLUDE_DIRECTORIES("${OSCPACK_ROOT_DIR}")

list(APPEND ALLOCORE_HEADERS ${OSC_HEADERS})

#list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
#  ${GLUT_INCLUDE_DIR})

#list(APPEND ALLOCORE_LINK_LIBRARIES
#  ${GLUT_LIBRARY})

else()
    message("NOT Building OSC module.")
    foreach(header ${OSC_HEADERS})
        list(APPEND OSC_DUMMY_HEADER_INFO "${header}::::OSC")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${OSC_DUMMY_HEADER_INFO})

endif(BUILD_OSC)
