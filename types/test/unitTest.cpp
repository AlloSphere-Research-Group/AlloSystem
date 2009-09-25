#include <assert.h>
#include "allo_types.h"
#include "allo_types_cpp.h"
#include "stdio.h"

typedef double data_t;

int main(){

	// test the basic C functionality
	{
		const int comps = 1;	//
		const int dims = 2;		// no. dimensions
		const int size1 = 2;	// no. elements in dimension 1
		const int size2 = 64;	// no. elements in dimension 2
		const int stride1 = 1;
		const int stride2 = size1;

		data_t data[comps*size1*size2];
	
		AlloLatticeHeader hdr;
		hdr.type = AlloFloat64Ty;
		hdr.components = comps;
		hdr.dimcount = dims;
		hdr.dim[0] = size1;
		hdr.dim[1] = size2;
		hdr.stride[0] = stride1;
		hdr.stride[1] = stride2;
		
		AlloLattice lat;
		lat.data.ptr = &data;
		
		allo_lattice_setheader(&lat, &hdr);

		assert(allo_lattice_elements(&lat) == size1*size2);
		assert(allo_lattice_size(&lat) == sizeof(data));
	}

	return 0;
}


