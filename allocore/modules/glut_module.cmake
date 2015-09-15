
find_package(GLUT QUIET)

set(GLUT_HEADERS
    allocore/system/al_MainLoop.hpp
)

# Hack to find freeglut on MSYS2.
# Hardcoded to mingw64.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(GLUT_LIBRARY
    /mingw64/bin/libfreeglut.dll
  )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(GLUT_LIBRARY AND GLUT_INCLUDE_DIR)
  message(STATUS "Building GLUT module.")

  list(APPEND ALLOCORE_SRC
    src/system/al_MainLoop.cpp
    src/io/al_WindowGLUT.cpp
  )

  list(APPEND ALLOCORE_HEADERS
    ${GLUT_HEADERS}
  )

  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND ALLOCORE_SRC
      src/system/al_MainLoopOSX.mm
    )
  endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
    ${GLUT_INCLUDE_DIR}
  )

  list(APPEND ALLOCORE_LINK_LIBRARIES
    ${GLUT_LIBRARY}
  )

  if((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
      # Avoids lots of compiler warnings for GLUT
      set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
  endif((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
else()
  message("NOT Building GLUT module.")

  foreach(header ${GLUT_HEADERS})
      list(APPEND GLUT_DUMMY_HEADER_INFO "${header}::::GLUT")
  endforeach(header ${GLUT_HEADERS})

  list(APPEND ALLOCORE_DUMMY_HEADERS
    ${GLUT_DUMMY_HEADER_INFO}
  )
endif(GLUT_LIBRARY AND GLUT_INCLUDE_DIR)
