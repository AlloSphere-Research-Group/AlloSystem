
find_package(Freetype QUIET)

set(FREETYPE_HEADERS
    allocore/graphics/al_Font.hpp
)

if(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)
# Depends on Glew and oepnGl, module must be included prior to this one
if(GLUT_LIBRARY AND OPENGL_LIBRARY)
message(STATUS "Building font module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Font.cpp)

list(APPEND ALLOCORE_HEADERS ${FREETYPE_HEADERS})

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${FREETYPE_INCLUDE_DIRS})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${FREETYPE_LIBRARY})

else()
    message("NOT Building font module. OpenGL, GLEW or GLUT not available.")
    foreach(header ${FREETYPE_HEADERS})
        list(APPEND FREETYPE_DUMMY_HEADER_INFO "${header}::::Freetype")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${FREETYPE_DUMMY_HEADER_INFO})
endif(GLUT_LIBRARY AND OPENGL_LIBRARY)

else()
    message("NOT Building font module. Freetype not found.")

    foreach(header ${FREETYPE_HEADERS})
        list(APPEND FREETYPE_DUMMY_HEADER_INFO "${header}::::Freetype")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${FREETYPE_DUMMY_HEADER_INFO})
endif(FREETYPE_LIBRARY AND FREETYPE_INCLUDE_DIRS)
