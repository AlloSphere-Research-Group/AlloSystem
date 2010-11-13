#include "utAllocore.h"

typedef double data_t;

int utTypes(){

	{
		Buffer<int> a(0,2);
		assert(a.size() == 0);
		assert(a.capacity() == 2);
		assert(a.fill() == 0);
		
		a.append(1);
		assert(a[0] == 1);
		assert(a.atAbs(0) == 1);
		assert(a.size() == 1);
		
		a.append(2);
		a.append(3);
		assert(a.size() == 3);
		assert(a.capacity() == 4);
		
		a.clear();
		assert(a.size() == 0);
		assert(a.capacity() == 4);


		// Test ring buffering
		a.resize(4);
		assert(a.size() == 4);

		a.write(1);
		a.write(2);
		assert(a.fill() == 2);
		
		a.write(3);
		a.write(4);
		
		assert(a.pos() == 3);
		
		assert(a[0] == 1);
		assert(a[1] == 2);
		assert(a[2] == 3);
		assert(a[3] == 4);
		
		assert(a.atRel(0) == 4);
		assert(a.atRel(1) == 3);
		assert(a.atRel(2) == 2);
		assert(a.atRel(3) == 1);
		
		assert(a.fill() == 4);
		a.write(5);
		assert(a.fill() == 4);
		assert(a[0] == 5);
		assert(a.atRel(0) == 5);
		assert(a.atRel(1) == 4);
		assert(a.atRel(2) == 3);
		assert(a.atRel(3) == 2);
	}


	// test the basic C functionality
	{
		const int comps = 1;	//
		const int dims = 2;		// no. dimensions
		const int size0 = 2;	// no. elements in dimension 1
		const int size1 = 64;	// no. elements in dimension 2
		const int stride0 = comps * sizeof(data_t);	// byte offset at dim 0
		const int stride1 = stride0*size0;

		data_t data[comps*size0*size1];
	
		AlloArrayHeader hdr;
		hdr.type = AlloFloat64Ty;
		hdr.components = comps;
		hdr.dimcount = dims;
		hdr.dim[0] = size0;
		hdr.dim[1] = size1;
		hdr.stride[0] = stride0;
		hdr.stride[1] = stride1;
		
		AlloArray lat;
		lat.data.ptr = (char *)&data;
		
		allo_array_setheader(&lat, &hdr);

		assert(allo_array_elements(&lat) == size0*size1);
		assert(allo_array_size(&lat) == sizeof(data));
	}
	
	{
		AlloArrayHeader hdr;
		hdr.type = AlloSInt8Ty;
		hdr.components = 1;
		hdr.dim[0] = 7;
		hdr.dim[1] = 7;

		// 2D test
		hdr.dimcount = 2;
		
		allo_array_setstride(&hdr, 1);
		assert(hdr.stride[0] == 1);
		assert(hdr.stride[1] == 7);

		allo_array_setstride(&hdr, 4);
		assert(hdr.stride[1] == 8);
		
		allo_array_setstride(&hdr, 8);
		assert(hdr.stride[1] == 8);
		
		// 3D test
		hdr.dimcount = 3;
		allo_array_setstride(&hdr, 4);
		assert(hdr.stride[2] == 8*7);
	}
	
	// not implemented...
//	{
//		al::Array lat;
//		lat.setpacked3d<double>(2, 8, 8, 8);
//		lat.data.ptr = (char *)calloc(1, lat.size());
//
//		double vals[2];
//		lat.interp(vals, 1, 2, 3);
//		printf("value at 1, 2, 3: %f %f\n", vals[0], vals[1]);
//	}
	return 0;
}


