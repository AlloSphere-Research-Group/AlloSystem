
#include "al_types.hpp"

#include <stdio.h>
#include <complex>

static std::complex<double> foo;

void test (uint32_t * x) {}

extern "C" void maincpp(AlloLattice * lat) {
    
	// coerce the C struct into a C++ class:
	allo::Lattice * lattice = (allo::Lattice *)lat;
	
	printf("lat is double? %i\n", lattice->checkType<double>());
	printf("lat is float? %i\n", lattice->checkType<float>());
		
	// create a new header:
	uint32_t dims[] = {740, 480};
	test(dims);
}


