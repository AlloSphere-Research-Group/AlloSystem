
#include "allocore/types/al_Lattice.hpp"

#include <stdio.h>
#include <complex>

static std::complex<double> foo;

void test (uint32_t * x) {}

extern "C" void maincpp(AlloLattice * lat) {
    
	// coerce the C struct into a C++ class:
	al::Lattice * lattice = (al::Lattice *)lat;
	
	printf("lat is double? %i\n", lattice->checkType<double>());
	printf("lat is float? %i\n", lattice->checkType<float>());
	
	double vals[2];
	lattice->interp(vals, 1, 2, 3);
	printf("value at 1, 2, 3: %f %f\n", vals[0], vals[1]);
		
	// create a new header:
	uint32_t dims[] = {740, 480};
	test(dims);
}


