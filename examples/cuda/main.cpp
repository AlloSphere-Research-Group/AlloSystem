#include <stdio.h>

#include "cuda_hello.cuh"


int main(int argc, char **argv)
{
	char out[16];
	process(out);

	printf("%s\n", out);
	return 0;
}
