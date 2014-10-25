
find_package(FreeImage QUIET)

set(FREEIMAGE_HEADERS
    allocore/graphics/al_Image.hpp
)

if(FREEIMAGE_LIBRARY AND FREEIMAGE_INCLUDE_PATH)
# Depends on Glew and oepnGl, module must be included prior to this one
if(GLUT_LIBRARY AND OPENGL_LIBRARY)
message(STATUS "Building freeimage module.")

list(APPEND ALLOCORE_SRC
    src/graphics/al_Image.cpp)

list(APPEND ALLOCORE_HEADERS ${FREEIMAGE_HEADERS})

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${FREEIMAGE_INCLUDE_PATH})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${FREEIMAGE_LIBRARY})

else()
    message("NOT Building freeimage module. OpenGL, GLEW or GLUT not available.")
    foreach(header ${FREEIMAGE_HEADERS})
        list(APPEND FREEIMAGE_DUMMY_HEADER_INFO "${header}::::Freeimage")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${FREEIMAGE_DUMMY_HEADERS})

endif(GLUT_LIBRARY AND OPENGL_LIBRARY)

else()
    message("NOT Building freeimage module.")
    foreach(header ${FREEIMAGE_HEADERS})
        list(APPEND FREEIMAGE_DUMMY_HEADER_INFO "${header}::::Freeimage")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${FREEIMAGE_DUMMY_HEADERS})
endif(FREEIMAGE_LIBRARY AND FREEIMAGE_INCLUDE_PATH)

