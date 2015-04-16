
find_package(Assimp QUIET)

set(ASSIMP_HEADERS
    allocore/graphics/al_Asset.hpp
    )

if(ASSIMP_LIBRARY)

    # Depends on Glew and OpenGl, OpenGL module must be included prior to this one
    if(GLUT_LIBRARY AND OPENGL_LIBRARY)

        message(STATUS "Building assimp module.")

        list(APPEND ALLOCORE_SRC
          src/graphics/al_Asset.cpp)

        list(APPEND ALLOCORE_HEADERS ${ASSIMP_HEADERS})

        list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
          ${ASSIMP_INCLUDE_DIR})

        list(APPEND ALLOCORE_LINK_LIBRARIES
          ${ASSIMP_LIBRARY})

    else()
        message("NOT Building assimp module. OpenGL, GLEW or GLUT not available.")
        foreach(header ${ASSIMP_HEADERS})
            list(APPEND ASSIMP_DUMMY_HEADER_INFO "${header}::::Assimp")
        endforeach()
        list(APPEND ALLOCORE_DUMMY_HEADERS ${ASSIMP_DUMMY_HEADER_INFO})
    endif(GLUT_LIBRARY AND OPENGL_LIBRARY)
else()
    message("NOT Building assimp module. Assimp not found.")
    foreach(header ${ASSIMP_HEADERS})
        list(APPEND ASSIMP_DUMMY_HEADER_INFO "${header}::::Assimp")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${ASSIMP_DUMMY_HEADER_INFO})
endif(ASSIMP_LIBRARY)
