set(BUILD_ZEROCONF 1)

set(ZEROCONF_HEADERS
        allocore/protocol/al_Zeroconf.hpp)

if(BUILD_ZEROCONF)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND ALLOCORE_SRC
    src/protocol/al_Zeroconf_OSX.mm)
    message(STATUS "Building OS X Zeroconf module.")

    list(APPEND ALLOCORE_HEADERS
        allocore/protocol/al_Zeroconf.hpp)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package(Threads QUIET)
  find_library(AVAHI_COMMON_LIBRARY NAMES avahi-common)
  find_library(AVAHI_CLIENT_LIBRARY NAMES avahi-client)
  if(CMAKE_THREAD_LIBS_INIT AND AVAHI_COMMON_LIBRARY AND AVAHI_CLIENT_LIBRARY)
  list(APPEND ALLOCORE_LINK_LIBRARIES ${AVAHI_COMMON_LIBRARY} ${AVAHI_CLIENT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
  list(APPEND ALLOCORE_SRC
    src/protocol/al_Zeroconf.cpp)
  message(STATUS "Building Linux Zeroconf module.")
    list(APPEND ALLOCORE_HEADERS ${ZEROCONF_HEADERS})
  else()
    message("NOT Building Zeroconf module. Pthreads, avahi-common and avahi-client required.")

    foreach(header ${ZEROCONF_HEADERS})
        list(APPEND ZEROCONF_DUMMY_HEADER_INFO "${header}::::ZeroConf")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${ZEROCONF_DUMMY_HEADER_INFO})
  endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message("NOT Building Zeroconf module. (Not supported on Windows)")

    foreach(header ${ZEROCONF_HEADERS})
        list(APPEND ZEROCONF_DUMMY_HEADER_INFO "${header}::::ZeroConf")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${ZEROCONF_DUMMY_HEADER_INFO})
endif()

#list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
#  ${GLUT_INCLUDE_DIR})

#list(APPEND ALLOCORE_LINK_LIBRARIES
#  ${GLUT_LIBRARY})

else()
    message("NOT Building Zeroconf module.")

    foreach(header ${ZEROCONF_HEADERS})
        list(APPEND ZEROCONF_DUMMY_HEADER_INFO "${header}::::ZeroConf")
    endforeach()
    list(APPEND ALLOCORE_DUMMY_HEADERS ${ZEROCONF_DUMMY_HEADER_INFO})
endif(BUILD_ZEROCONF)
