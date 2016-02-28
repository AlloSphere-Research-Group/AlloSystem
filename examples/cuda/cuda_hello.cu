
#include "cuda_hello.cuh"

__global__
void hello(char *a, int *b)
{
	a[threadIdx.x] += b[threadIdx.x];
}
