
find_package(Assimp QUIET)

if(ASSIMP_LIBRARY)
message("Building assimp module.")

list(APPEND ALLOCORE_SRC
  src/graphics/al_Asset.cpp)

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${ASSIMP_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${ASSIMP_LIBRARY})

else()
message("NOT Building assimp module.")
endif(ASSIMP_LIBRARY)
