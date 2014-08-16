
# Depends on Glew and oepnGl, module must be included prior to this one
if(GLUT_LIBRARY AND OPENGL_LIBRARY)
find_package(FreeImage QUIET)

if(FREEIMAGE_LIBRARY AND FREEIMAGE_INCLUDE_PATH)
message("Building freeimage module.")

list(APPEND ALLOCORE_SRC
    src/graphics/al_Image.cpp)

list(APPEND ALLOCORE_HEADERS
    allocore/graphics/al_Image.hpp
)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${FREEIMAGE_INCLUDE_PATH})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${FREEIMAGE_LIBRARY})

else()
message("NOT Building freeimage module.")
endif(FREEIMAGE_LIBRARY)

else()
message("NOT Building freeimage module. OpenGL, GLEW or GLUT not available.")

endif(GLUT_LIBRARY AND OPENGL_LIBRARY)
