
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

# empty  allocore/graphics/al_Slab.hpp, remove?


list(APPEND ALLOCORE_HEADERS
    allocore/graphics/al_BufferObject.hpp
    allocore/graphics/al_DisplayList.hpp
    allocore/graphics/al_FBO.hpp
    allocore/graphics/al_GPUObject.hpp
    allocore/graphics/al_Graphics.hpp
    allocore/graphics/al_Isosurface.hpp
    allocore/graphics/al_Lens.hpp
    allocore/graphics/al_Light.hpp
    allocore/graphics/al_OpenGL.hpp
    allocore/graphics/al_Shader.hpp
    allocore/graphics/al_Slab.hpp
    allocore/graphics/al_Stereographic.hpp
    allocore/graphics/al_Texture.hpp
    allocore/io/al_App.hpp
    allocore/io/al_ControlNav.hpp
    allocore/io/al_Window.hpp
)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${GLEW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${GLEW_LIBRARY}
  ${OPENGL_LIBRARY})

else()
message("NOT Building OpenGL module. OpenGL and GLEW required.")
endif(GLEW_LIBRARY AND OPENGL_LIBRARY)
