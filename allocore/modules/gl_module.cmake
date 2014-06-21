
find_package(GLEW QUIET)
find_package(OpenGL QUIET)

if(GLEW_LIBRARY AND OPENGL_LIBRARY)
message("Building OpenGL module (OpenGL + GLEW).")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Graphics.cpp
  src/graphics/al_FBO.cpp
  src/graphics/al_GPUObject.cpp
  src/graphics/al_Isosurface.cpp
  src/graphics/al_Lens.cpp
  src/graphics/al_Light.cpp
  src/graphics/al_Mesh.cpp
  src/graphics/al_Shader.cpp
  src/graphics/al_Shapes.cpp
  src/graphics/al_Stereographic.cpp
  src/graphics/al_Texture.cpp
  src/io/al_App.cpp
  src/io/al_Window.cpp)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${GLEW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${GLEW_LIBRARY}
  ${OPENGL_LIBRARY})

else()
message("NOT Building OpenGL module. OpenGL and GLEW required.")
endif(GLEW_LIBRARY AND OPENGL_LIBRARY)
