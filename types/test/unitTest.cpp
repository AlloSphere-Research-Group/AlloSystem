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
		const int size0 = 2;	// no. elements in dimension 1
		const int size1 = 64;	// no. elements in dimension 2
		const int stride0 = comps * sizeof(data_t);	// byte offset at dim 0
		const int stride1 = stride0*size0;

		data_t data[comps*size0*size1];
	
		AlloLatticeHeader hdr;
		hdr.type = AlloFloat64Ty;
		hdr.components = comps;
		hdr.dimcount = dims;
		hdr.dim[0] = size0;
		hdr.dim[1] = size1;
		hdr.stride[0] = stride0;
		hdr.stride[1] = stride1;
		
		AlloLattice lat;
		lat.data.ptr = (char *)&data;
		
		allo_lattice_setheader(&lat, &hdr);

		assert(allo_lattice_elements(&lat) == size0*size1);
		assert(allo_lattice_size(&lat) == sizeof(data));
	}

	return 0;
}


