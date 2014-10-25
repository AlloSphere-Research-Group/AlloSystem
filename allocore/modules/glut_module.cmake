
find_package(GLUT QUIET)

set(GLUT_HEADERS
    allocore/system/al_MainLoop.hpp
)

if(GLUT_LIBRARY AND GLUT_INCLUDE_DIR)
message(STATUS "Building GLUT module.")

list(APPEND ALLOCORE_SRC
  src/system/al_MainLoop.cpp
  src/io/al_WindowGLUT.cpp)

list(APPEND ALLOCORE_HEADERS ${GLUT_HEADERS})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND ALLOCORE_SRC
    src/system/al_MainLoopOSX.mm
)
endif()

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${GLUT_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${GLUT_LIBRARY})

if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    # Avoids lots of compiler warnings for GLUT
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

else()
    message("NOT Building GLUT module.")

    foreach(header ${GLUT_HEADERS})
        list(APPEND GLUT_DUMMY_HEADER_INFO "${header}::::GLUT")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${GLUT_DUMMY_HEADER_INFO})
endif(GLUT_LIBRARY AND GLUT_INCLUDE_DIR)
