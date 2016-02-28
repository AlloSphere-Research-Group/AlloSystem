#ifndef CUDA_HELLO_CUH
#define CUDA_HELLO_CUH

#include <cuda_runtime_api.h>
#include <cuda.h>

__global__ 
void hello(char *a, int *b) ;

#endif
