
find_package(OpenCL QUIET)

set(OPENCL_HEADERS
    allocore/protocol/opencl/al_OpenCLCommandQueue.hpp
    allocore/protocol/opencl/al_OpenCLEvent.hpp
    allocore/protocol/opencl/al_OpenCLImage3D.hpp
    allocore/protocol/opencl/al_OpenCLMemoryBuffer.hpp
    allocore/protocol/opencl/al_OpenCLContext.hpp
    allocore/protocol/opencl/al_OpenCLExtensions.hpp
    allocore/protocol/opencl/al_OpenCLImageFormat.hpp
    allocore/protocol/opencl/al_OpenCLPlatform.hpp
    allocore/protocol/opencl/al_OpenCLDevice.hpp
    allocore/protocol/opencl/al_OpenCL.hpp
    allocore/protocol/opencl/al_OpenCLInternal.hpp
    allocore/protocol/opencl/al_OpenCLProgram.hpp
    allocore/protocol/opencl/al_OpenCLEngine.hpp
    allocore/protocol/opencl/al_OpenCLImage2D.hpp
    allocore/protocol/opencl/al_OpenCLKernel.hpp
)

if(OpenCL_LIBRARY AND OpenCL_INCLUDE_DIRS)
  message(STATUS "Building OpenCL module.")

  list(APPEND ALLOCORE_SRC
      src/protocol/opencl/al_OpenCLCommandQueue.cpp
      src/protocol/opencl/al_OpenCLEvent.cpp
      src/protocol/opencl/al_OpenCLInternal.cpp
      src/protocol/opencl/al_OpenCLProgram.cpp
      src/protocol/opencl/al_OpenCLContext.cpp
      src/protocol/opencl/al_OpenCLExtensions.cpp
      src/protocol/opencl/al_OpenCLKernel.cpp
      src/protocol/opencl/al_OpenCLDevice.cpp
      src/protocol/opencl/al_OpenCLImage2D.cpp
      src/protocol/opencl/al_OpenCLMemoryBuffer.cpp
      src/protocol/opencl/al_OpenCLEngine.cpp
      src/protocol/opencl/al_OpenCLImage3D.cpp
      src/protocol/opencl/al_OpenCLPlatform.cpp
  )

  list(APPEND ALLOCORE_HEADERS
    ${OPENCL_HEADERS}
  )

  list(APPEND ALLOCORE_DEP_INCLUDE_DIRS
    ${OpenCL_INCLUDE_DIRS}
  )

  list(APPEND ALLOCORE_LINK_LIBRARIES
    ${OpenCL_LIBRARY}
  )

else()
  message("NOT Building OpenCL module.")

  foreach(header ${OPENCL_HEADERS})
      list(APPEND OPENCL_DUMMY_HEADER_INFO "${header}::::OpenCL")
  endforeach(header ${OPENCL_HEADERS})

  list(APPEND ALLOCORE_DUMMY_HEADERS
    ${OPENCL_DUMMY_HEADER_INFO}
  )
endif(OpenCL_LIBRARY AND OpenCL_INCLUDE_DIRS)
