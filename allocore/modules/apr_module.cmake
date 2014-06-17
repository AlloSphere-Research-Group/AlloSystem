
find_package(APR QUIET)

if(APR_LIBRARY)
message("Building APR module.")

list(APPEND ALLOCORE_SRC
  src/io/al_FileAPR.cpp
  src/io/al_SocketAPR.cpp
  src/protocol/al_XML.cpp
  src/system/al_Memory.cpp
  src/system/al_Time.cpp)

# al_Memory.cpp was not actually being built with cmake, is it even used, worth keeping?

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${APR_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${APR_LIBRARY})

else()
message("NOT Building APR module.")
endif(APR_LIBRARY)
