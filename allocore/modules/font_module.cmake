find_package(Freetype QUIET)

if(FREETYPE_LIBRARY)
message("Building font module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Font.cpp)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${FREETYPE_INCLUDE_DIRS})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${FREETYPE_LIBRARY})

else()
message("NOT Building font module.")
endif(FREETYPE_LIBRARY)
