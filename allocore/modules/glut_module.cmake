
find_package(GLUT QUIET)

if(GLUT_LIBRARY)
message("Building GLUT module.")

list(APPEND ALLOCORE_SRC
  src/system/al_MainLoop.cpp
  src/io/al_WindowGLUT.cpp)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND ALLOCORE_SRC
    src/system/al_MainLoopOSX.mm
)
endif()

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${GLUT_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${GLUT_LIBRARY})

else()
message("NOT Building GLUT module.")
endif(GLUT_LIBRARY)
