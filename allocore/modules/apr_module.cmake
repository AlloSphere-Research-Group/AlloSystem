
find_package(APR QUIET)

set(APR_HEADERS
    allocore/io/al_Socket.hpp
    allocore/system/al_Memory.hpp
)

if(APR_LIBRARY AND APR_INCLUDE_DIR)
message(STATUS "Building APR module.")

list(APPEND ALLOCORE_SRC
    src/io/al_SocketAPR.cpp
    src/system/al_Memory.cpp
)

list(APPEND ALLOCORE_HEADERS ${APR_HEADERS})

# al_Memory.cpp was not actually being built with cmake, is it even used, worth keeping?

list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
  ${APR_INCLUDE_DIR})

list(APPEND ALLOCORE_LINK_LIBRARIES
  ${APR_LIBRARY})

else()
    message("NOT Building APR module.")
    foreach(header ${APR_HEADERS})
        list(APPEND APR_DUMMY_HEADER_INFO "${header}::::APR")
    endforeach()

    list(APPEND ALLOCORE_DUMMY_HEADERS ${APR_DUMMY_HEADER_INFO})
endif(APR_LIBRARY AND APR_INCLUDE_DIR)
