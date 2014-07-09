
# Depends on Glew and oepnGl, module must be included prior to this one
if(GLUT_LIBRARY AND OPENGL_LIBRARY)

find_package(Freetype QUIET)

if(FREETYPE_LIBRARY)
message("Building font module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Font.cpp)

list(APPEND ALLOCORE_HEADERS
    allocore/graphics/al_Font.hpp
)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${FREETYPE_INCLUDE_DIRS})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${FREETYPE_LIBRARY})

else()
message("NOT Building font module. Freetype not found.")
endif(FREETYPE_LIBRARY)

else()
message("NOT Building font module. OpenGL, GLEW or GLUT not available.")
endif(GLUT_LIBRARY AND OPENGL_LIBRARY)
