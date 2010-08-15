#include "protocol/opencl/al_OpenCL.hpp"
#include <stdio.h>

using namespace al;

const char *kernel_source = "\n" \
"__kernel void square(                                                  \n" \
"   __global float* input,                                              \n" \
"   __global float* output                                              \n" \
"   )																	\n" \
"{                                                                      \n" \
"   int i = get_global_id(0);                                           \n" \
"   int j = get_global_id(1);                                           \n" \
"   int w = get_global_size(0);                                         \n" \
"																		\n" \
"       output[i+j*w] = input[i+j*w] * input[i+j*w];                    \n" \
"}                                                                      \n" \
"\n";

int main (int argc, char * argv[]) {
	size_t sizes[] = {4, 4};
	size_t sizes1[] = {1, 1};
	float data1[16];
	float data2[16];
	for(int i=0; i < 16; i++) data1[i] = (float)i;

	cl::OpenCLEngine engine;
	engine.compile_source("square", kernel_source);

	cl::OpenCLKernel *kernel = engine.get_kernel("square");
	cl::OpenCLMemoryBuffer *mem1 = engine.create_memory_buffer(CL_MEM_READ_WRITE, sizeof(float)*16, data1);
	cl::OpenCLMemoryBuffer *mem2 = engine.create_memory_buffer(CL_MEM_READ_WRITE, sizeof(float)*16, NULL);
	
	kernel->set_argument(0, *mem1);
	kernel->set_argument(1, *mem2);
	
	
	
	printf("\nUSING GPU DEVICE:\n");
	cl::OpenCLDevice *dev = engine.gpu_device();
	printf("%s %s %s \tmax_compute_units: %d \tmax_work_group_size: %d\n", 
		dev->get_name().c_str(), 
		dev->get_vendor().c_str(), 
		dev->get_version().c_str(), 
		(int)dev->get_max_compute_units(),
		(int)dev->get_max_work_group_size()
	);
	engine.enqueue_kernel(*dev, kernel, 2, sizes, sizes);
	engine.enqueue_read(*dev, mem2, true, 0, sizeof(float)*16, data2);
	for(int i=0; i < 16; i++) printf("%d: %f\n", i, data2[i]);
	
	
	// reset input data to something different
	for(int i=0; i < 16; i++) data1[i] = ((float)i)*0.5;
	
	printf("\nUSING CPU DEVICE:\n");
	cl::OpenCLDevice *dev2 = engine.cpu_device();
	printf("%s %s %s \tmax_compute_units: %d \tmax_work_group_size: %d\n", 
		dev2->get_name().c_str(), 
		dev2->get_vendor().c_str(), 
		dev2->get_version().c_str(), 
		(int)dev2->get_max_compute_units(),
		(int)dev2->get_max_work_group_size()
	);
	engine.enqueue_kernel(*dev2, kernel, 2, sizes, sizes1);
	engine.enqueue_read(*dev2, mem2, true, 0, sizeof(float)*16, data2);
	for(int i=0; i < 16; i++) printf("%d: %f\n", i, data2[i]);
	
	return 0;
}
