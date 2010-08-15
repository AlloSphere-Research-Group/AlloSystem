#ifndef AL_OPENCL_MEMORY_BUFFER_H
#define AL_OPENCL_MEMORY_BUFFER_H 1

#include "al_OpenCLInternal.hpp"
#include "al_OpenCLContext.hpp"
#include <vector>
#include <string>

using std::vector;
using std::string;

namespace al {
namespace cl {

class OpenCLMemoryBuffer : public OpenCLResource<OpenCLContext> {
public:
	OpenCLMemoryBuffer(cl_mem mem=0)
	:	mMem(mem)
	{}
	
	virtual ~OpenCLMemoryBuffer() {}
	
	cl_mem get_memory() const {return mMem;}
	void create(OpenCLContext &ctx, cl_mem_flags usage, size_t size, void *ptr);
	void destroy();

protected:
	cl_mem mMem;
};

}	// cl::
}	// al::


#endif // AL_OPENCL_MEMORY_BUFFER_H
