set(BUILD_OSC 1)


if(BUILD_OSC)
message("Building OSC module.")

# for oscpack (oscpack sources are included with AlloSystem)
include(TestBigEndian)
test_big_endian(TEST_ENDIAN)
if (TEST_ENDIAN MATCHES 0)
  add_definitions(-DOSC_HOST_LITTLE_ENDIAN)
else()
  add_definitions(-DOSC_HOST_BIG_ENDIAN)
endif()

list(APPEND ALLOCORE_SRC
  src/protocol/oscpack/osc/OscOutboundPacketStream.cpp
  src/protocol/oscpack/osc/OscPrintReceivedElements.cpp
  src/protocol/oscpack/osc/OscReceivedElements.cpp
  src/protocol/oscpack/osc/OscTypes.cpp
  src/protocol/al_OSC.cpp)

#list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
#  ${GLUT_INCLUDE_DIR})

#list(APPEND ALLOCORE_LINK_LIBRARIES
#  ${GLUT_LIBRARY})

else()
message("NOT Building OSC module.")
endif(BUILD_OSC)
