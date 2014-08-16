
# Depends on Glew and oepnGl, module must be included prior to this one
if(GLUT_LIBRARY AND OPENGL_LIBRARY)

find_package(Assimp QUIET)

if(ASSIMP_LIBRARY AND ASSIMP_INCLUDE_DIR)
message("Building assimp module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Asset.cpp)

list(APPEND ALLOCORE_HEADERS
    allocore/graphics/al_Asset.hpp
)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${ASSIMP_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${ASSIMP_LIBRARY})

else()
message("NOT Building assimp module. Assimp not found.")
endif(ASSIMP_LIBRARY)
else()
message("NOT Building assimp module. OpenGL, GLEW or GLUT not available.")
endif(GLUT_LIBRARY AND OPENGL_LIBRARY)
