# OSX
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# LINUX
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	# ???
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# WINDOWS
# Hardcode path to library
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(GLFW_LIBRARIES /mingw64/bin/glfw3.dll)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(GLFW_LIBRARIES)
	set(GLFW_FOUND 1)
	list(APPEND ALLOCORE_DEP_INCLUDE_DIRS ${GLFW_INCLUDE_DIRS})
	list(APPEND ALLOCORE_LINK_LIBRARIES ${GLFW_LIBRARIES})
	
	list(APPEND ALLOCORE_HEADERS ${GLFW_HEADER})

	list(APPEND ALLOCORE_HEADERS allocore/system/al_MainLoop.hpp)
	list(APPEND ALLOCORE_SRC
		src/system/al_MainLoop.cpp
		src/io/al_WindowGLFW.cpp)

	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND ALLOCORE_SRC src/system/al_MainLoopOSX.mm)
  endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

else()
	# GLFW not found, write dummy headers
	message("NOT Building GLFW module.")
  foreach(header ${GLFW_HEADER})
      list(APPEND GLFW_DUMMY_HEADER_INFO "${header}::::GLFW")
  endforeach(header ${GLFW_HEADER})
  list(APPEND ALLOCORE_DUMMY_HEADERS ${GLFW_DUMMY_HEADER_INFO})

endif(GLFW_LIBRARIES)