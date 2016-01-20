
# glew only for linux and windows?
# currently glfw branch only on osx
# find_package(GLEW QUIET)

find_package(OpenGL QUIET)

set(GL_HEADERS
    allocore/graphics/al_GPUObject.hpp
    allocore/graphics/al_OpenGL.hpp
    allocore/graphics/al_OpenGLGLFW.hpp
    # allocore/graphics/al_OpenGLGLUT.hpp

    allocore/graphics/al_Graphics.hpp

    allocore/io/al_Window.hpp
)

# if(GLEW_LIBRARY AND OPENGL_LIBRARY)
if(OPENGL_LIBRARY)
# message(STATUS "Building OpenGL module (OpenGL + GLEW).")
message(STATUS "Building OpenGL module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_GPUObject.cpp

  src/graphics/al_Graphics.cpp

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
