#include "stdio.h"

#include "allo_types.h"

extern void maincpp(AlloLattice * lat);

typedef struct { double x; double y; } cell_d2;

int main (int argc, char * const argv[]) {
    
	void * data = malloc(sizeof(double) * 2 * 64);
	
	AlloLattice * lat = malloc(sizeof(AlloLattice));
	lat->data.ptr = data;
	lat->header.type = AlloFloat64Ty;
	lat->header.components = 2;
	lat->header.dimcount = 2;
	lat->header.dim[0] = 4;
	lat->header.dim[1] = 4;
	// stride is typically the dim size * the type size * the components
	lat->header.stride[0] = lat->header.components * allo_type_size(lat->header.type);
	lat->header.stride[1] = lat->header.dim[0] * lat->header.stride[0];
	
	printf("size %u\n", allo_type_size(lat->header.type));
	
	maincpp(lat);
	
	
	printf("lat %p, data %p, *lat %p\n", lat, data, *lat);
	printf("dimcount %i, components %i, dim %i %i sizeof %i\n", 
		lat->header.dimcount, lat->header.components, 
		lat->header.dim[0], lat->header.dim[1], allo_lattice_size(lat) + sizeof(AlloLattice));
		
	int rowstride = lat->header.stride[1];
	int width = lat->header.dim[0];
	int height = lat->header.dim[1];
	int components = lat->header.components;
	printf("---- %d\n", rowstride);
	for(int j=0; j < height; j++) {
		double *data = (double *)(lat->data.ptr + rowstride*j);
		for(int i=0; i < width; i++) {
			printf("\t[%3i %3i] = (%.5f, %.5f) ", i, j, data[0], data[1]);
			data += components;
		}
		printf("\n");
	}
	
	
	return 0;
}

