
#include "allocore/types/al_Array.hpp"

#include <stdio.h>
#include <complex>

static std::complex<double> foo;

void test (uint32_t * x) {}

extern "C" void maincpp(AlloArray * lat) {
    
	// coerce the C struct into a C++ class:
	al::Array * array = (al::Array *)lat;
	
	printf("lat is double? %i\n", array->checkType<double>());
	printf("lat is float? %i\n", array->checkType<float>());
	
	double vals[2];
	array->interp(vals, 1, 2, 3);
	printf("value at 1, 2, 3: %f %f\n", vals[0], vals[1]);
		
	// create a new header:
	uint32_t dims[] = {740, 480};
	test(dims);
}


