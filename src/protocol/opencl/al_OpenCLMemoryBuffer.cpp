#include "al_OpenCLMemoryBuffer.hpp"


namespace al {
namespace cl {

// CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR initializes gost memory
// CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR are mutually exclusive
// CL_MEM_ALLOC_HOST_PTR | CL_MEM_USE_HOST_PTR are mutually exclusive
void OpenCLMemoryBuffer :: create(OpenCLContext &ctx, cl_mem_flags usage, size_t size, void *ptr) {
	destroy();
	detach();
	
	if(GET_FLAG(CL_MEM_COPY_HOST_PTR, usage) && GET_FLAG(CL_MEM_USE_HOST_PTR, usage)) {
		opencl_error(USER_OPENCL_ERROR, "OpenCLMemoryBuffer::create CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR are mutually exclusive");
	}
	if(GET_FLAG(CL_MEM_ALLOC_HOST_PTR, usage) && GET_FLAG(CL_MEM_USE_HOST_PTR, usage)) {
		opencl_error(USER_OPENCL_ERROR, "OpenCLMemoryBuffer::create CL_MEM_ALLOC_HOST_PTR | CL_MEM_USE_HOST_PTR are mutually exclusive");
	}
	
	usage |= (ptr ? CL_MEM_USE_HOST_PTR : 0);
	
	cl_int res = CL_SUCCESS;
	cl_mem mem = clCreateBuffer(
		ctx.get_context(),
		usage,
		size,
		ptr,
		&res
	);
	
	if(opencl_error(res, "clCreateBuffer error creating buffer")) {
		return;
	}
	
	mMem = mem;
	ctx.attach_resource(this);
}

void OpenCLMemoryBuffer :: destroy() {
	if(mMem) {
		cl_int res = clReleaseMemObject(mMem);
		mMem = 0;
		
		opencl_error(res, "clReleaseMemObject error releasing memory");
	}
}

}	// cl::
}	// al::
