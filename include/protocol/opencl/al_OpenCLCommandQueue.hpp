#ifndef AL_OPENCL_COMMAND_QUEUE_H
#define AL_OPENCL_COMMAND_QUEUE_H 1

#include "al_OpenCLInternal.hpp"
#include "al_OpenCLContext.hpp"
#include "al_OpenCLKernel.hpp"
#include "al_OpenCLMemoryBuffer.hpp"
#include "al_OpenCLEvent.hpp"
#include <vector>
#include <string>

using std::vector;
using std::string;

namespace al {
namespace cl {

class OpenCLCommandQueue : public OpenCLResource<OpenCLContext> {
public:	
	OpenCLCommandQueue(cl_command_queue command_queue=0)
	:	mCommandQueue(command_queue)
	{}


	virtual ~OpenCLCommandQueue() {}
	
	cl_command_queue get_command_queue() const {return mCommandQueue;}
	void create(OpenCLContext &ctx, const OpenCLDevice &dev, bool ordered=true, bool profiling=false);
	OpenCLEvent enqueueKernel(const OpenCLKernel &ker, cl_uint ndim, size_t *global, size_t *local);
	OpenCLEvent enqueueRead(const OpenCLMemoryBuffer &mem, bool block, size_t offset, size_t size, void *ptr);
	void destroy();

protected:
	cl_command_queue mCommandQueue;
};

}	// cl::
}	// al::


#endif // AL_OPENCL_COMMAND_QUEUE_H