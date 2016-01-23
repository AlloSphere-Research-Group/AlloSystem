
# glew only for linux and windows?
# currently glfw branch only on osx
# find_package(GLEW QUIET)

find_package(OpenGL QUIET)

# graphics things

# al_Asset included at assimp module
# al_Font at font module
# al_Image at image module

# easyFbo: later
# slab: currently empty

set(GL_HEADERS
  # no dep
  allocore/graphics/al_OpenGL.hpp
  allocore/graphics/al_OpenGLGLFW.hpp
  allocore/graphics/al_GPUObject.hpp
  allocore/graphics/al_Graphics.hpp
  # 1 tier dep
  allocore/graphics/al_BufferObject.hpp
  allocore/graphics/al_DisplayList.hpp
  allocore/graphics/al_FBO.hpp
  allocore/graphics/al_Lens.hpp
  allocore/graphics/al_Light.hpp
  allocore/graphics/al_Mesh.hpp
  allocore/graphics/al_Shader.hpp
  allocore/graphics/al_Texture.hpp
  # 2 tier dep
  allocore/graphics/al_Isosurface.hpp
  allocore/graphics/al_MeshVBO.hpp
  allocore/graphics/al_Shapes.hpp
  allocore/graphics/al_Stereographic.hpp
  
  # others
  allocore/io/al_Window.hpp
)

# if(GLEW_LIBRARY AND OPENGL_LIBRARY)
if(OPENGL_LIBRARY)
# message(STATUS "Building OpenGL module (OpenGL + GLEW).")
message(STATUS "Building OpenGL module.")

list(APPEND ALLOCORE_SRC
  # no dep
  src/graphics/al_GPUObject.cpp
  src/graphics/al_Graphics.cpp
  # 1 tier dep
  src/graphics/al_BufferObject.cpp
  src/graphics/al_FBO.cpp
  src/graphics/al_Lens.cpp
  src/graphics/al_Light.cpp
  src/graphics/al_Mesh.cpp
  src/graphics/al_Shader.cpp
  src/graphics/al_Texture.cpp
  # 2 tier dep
  src/graphics/al_MeshVBO.cpp
  src/graphics/al_Isosurface.cpp
  src/graphics/al_Shapes.cpp
  src/graphics/al_Stereographic.cpp

  # others
  src/io/al_Window.cpp
)

# TODO empty  allocore/graphics/al_Slab.hpp, remove?

list(APPEND ALLOCORE_HEADERS ${GL_HEADERS})

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  # ${GLEW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  # ${GLEW_LIBRARY}
  ${OPENGL_LIBRARY})

else()
    message("NOT Building OpenGL module. OpenGL and GLEW required.")

    foreach(header ${GL_HEADERS})
        list(APPEND GL_DUMMY_HEADER_INFO "${header}::::OpenGL and GLEW")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${GL_DUMMY_HEADER_INFO})

endif(GLEW_LIBRARY AND OPENGL_LIBRARY)
