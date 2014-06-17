

find_package(FreeImage QUIET)

if(FREEIMAGE_LIBRARY)
message("Building freeimage module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Image.cpp)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${FREEIMAGE_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${FREEIMAGE_LIBRARY})

else()
message("NOT Building freeimage module.")
endif(FREEIMAGE_LIBRARY)
