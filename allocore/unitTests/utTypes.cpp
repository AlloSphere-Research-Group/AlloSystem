#include "utAllocore.h"

typedef double data_t;

int utTypes(){

	// Conversion
	{ auto s = toString(0.3f); assert("0.3"==s);}
	{ auto s = toString(0.3 ); assert("0.3"==s);}
	{ auto s = toString(333 ); assert("333"==s);}

	{ auto v = fromString<float>("0.3"); assert(float(0.3)==v); }
	{ auto v = fromString<double>("0.3"); assert(double(0.3)==v); }
	{ auto v = fromString<int>("333"); assert(int(333)==v); }

	// Array
	{	// test the basic C functionality
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

	{	// Basic Array usage

		// TODO: move this to an example
//		// a 1D array of float-pairs (e.g. interleaved audio buffer)
//		Array buf(2, AlloFloat32Ty, 64);
//
//		// a 2D array of char[4] data (e.g. ARGB image matrix with color values as 0-255)
//		Array img(4, AlloUInt8Ty, 720, 480);
//
//		// a 3D array of float triplets (e.g. vector field)
//		Array field(3, Array::type<float>(), 16, 16, 16);

		{	Array a;
			assert(a.size() == 0);
			assert(!a.hasData());
			assert(a.type() == AlloVoidTy);
			assert(a.isType(AlloVoidTy));
			assert(a.isFormat(a));
		}

		// Run tests on Arrays with various sizes and dimensions
		const int Nc= 3;
		const int Ns[] = {1, 4, 5, 6, 7, 64, 128, 129};

		for(unsigned iN=0; iN<sizeof(Ns)/sizeof(*Ns); ++iN){

			int N = Ns[iN]; // number of cells along each dimension

			{	// Constructors
				{	Array a(Nc, AlloFloat32Ty, N);
					assert(a.hasData());
					assert(a.type() == AlloFloat32Ty);
					assert(a.components() == Nc);
					assert(a.dimcount() == 1);
				}
				{	Array a(Nc, AlloFloat32Ty, N,N);
					assert(a.hasData());
					assert(a.type() == AlloFloat32Ty);
					assert(a.components() == Nc);
					assert(a.dimcount() == 2);
				}
				{	Array a(Nc, AlloFloat32Ty, N,N,N);
					assert(a.hasData());
					assert(a.type() == AlloFloat32Ty);
					assert(a.components() == Nc);
					assert(a.dimcount() == 3);
				}
			}

			{	// Memory allocation
				Array a;

				a.formatAligned(Nc, AlloFloat32Ty, N, 1);
				assert((int)a.size() == Nc*N*4);
				assert(a.hasData());
				assert(a.isType(AlloFloat32Ty));
				assert(a.isType<float>());

				a.formatAligned(Nc, AlloSInt8Ty, N, N, 1);
				assert((int)a.size() == Nc*N*N*1);
				assert(a.hasData());
				assert(a.isType(AlloSInt8Ty));
				assert(a.isType<int8_t>());

				a.formatAligned(Nc, AlloSInt16Ty, N, N, N, 1);
				assert((int)a.size() == Nc*N*N*N*2);
				assert(a.hasData());
				assert(a.isType(AlloSInt16Ty));
				assert(a.isType<int16_t>());
			}

			{	// 1-D element access
				Array a(Nc, AlloSInt8Ty, N);
				for(int i=0,t=0; i<N; ++i){

					int8_t x[Nc] = {int8_t(i), int8_t(i+1), int8_t(i+2)};
					int8_t y[Nc] = {-1,-1,-1};
					a.write(x, i);
					a.read(y, i);
					for(int c=0; c<Nc; ++c) assert(y[c] == x[c]);

					for(int c=0; c<Nc; ++c){
						a.elem<int8_t>(c,i) = t+1;
						assert(a.elem<int8_t>(c,i) == int8_t(t+1));
						++t;
					}
				}
			}

			{	// 2-D element access
				Array a(Nc, AlloSInt8Ty, N,N);
				for(int j=0,t=0; j<N; ++j){
				for(int i=0; i<N; ++i){

					int8_t x[Nc] = {int8_t(j), int8_t(j+1), int8_t(j+2)};
					int8_t y[Nc] = {-1,-1,-1};
					a.write(x, i,j);
					a.read(y, i,j);
					for(int c=0; c<Nc; ++c) assert(y[c] == x[c]);

					for(int c=0; c<Nc; ++c){
						a.elem<int8_t>(c,i,j) = t+1;
						assert(a.elem<int8_t>(c,i,j) == int8_t(t+1));
						++t;
					}
				}}
			}

			{	// 3-D element access
				Array a(Nc, AlloSInt8Ty, N,N,N);
				for(int k=0,t=0; k<N; ++k){
				for(int j=0; j<N; ++j){
				for(int i=0; i<N; ++i){

					int8_t x[Nc] = {int8_t(k), int8_t(k+1), int8_t(k+2)};
					int8_t y[Nc] = {-1,-1,-1};
					a.write(x, i,j,k);
					a.read(y, i,j,k);
					for(int c=0; c<Nc; ++c) assert(y[c] == x[c]);

					for(int c=0; c<Nc; ++c){
						a.elem<int8_t>(c,i,j,k) = t+1;
						assert(a.elem<int8_t>(c,i,j,k) == int8_t(t+1));
						++t;
					}
				}}}
			}

		}	// end size loop
	}


	{
		Buffer<int> a(0,2);
		assert(a.size() == 0);
		assert(a.capacity() == 2);
		//assert(a.fill() == 0);

		a.append(1);
		assert(a[0] == 1);
		assert(a.size() == 1);
		assert(a.last() == 1);

		a.append(2);
		a.append(3);
		assert(a.size() == 3);
		assert(a.capacity() == 4);
		assert(a.last() == 3);

		a.reset();
		assert(a.size() == 0);
		assert(a.capacity() == 4);

		a.append(7);
		a.repeatLast();
		assert(a[0] == 7);
		assert(a[1] == 7);

		// Appending another Buffer
		{
			Buffer<int> b(4);
			for(int i=0; i<b.size(); ++i) b[i] = i+4;

			a.size(4);
			for(int i=0; i<a.size(); ++i) a[i] = i;

			// Append non-zero sized to non-zero sized
			{
			int N = a.size() + b.size();
			a.append(b);
				assert(a.size() == N);
				for(int i=0; i<N; ++i) assert(a[i] == i);
			}

			// Append non-zero sized to zero sized
			a.size(0);
			a.append(b);
				assert(a.size() == b.size());
				for(int i=0; i<a.size(); ++i) assert(a[i] == b[i]);

			// Append zero sized to non-zero sized
			{
			int N = a.size();
			b.size(0);
			a.append(b);
				assert(a.size() == N);
			}
		}
	}

	{
		RingBuffer<int> a;

		// Test ring buffering
		a.resize(4);
		assert(a.size() == 4);
		assert(a.fill() == 0);

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
		assert(a.read(0) == 4);
		assert(a.read(1) == 3);
		assert(a.read(2) == 2);
		assert(a.read(3) == 1);
		assert(a.newest() == 4);
		assert(a.oldest() == 1);

		a.write(5);

		assert(a.fill() == 4);
		assert(a[0] == 5);
		assert(a.read(0) == 5);
		assert(a.read(1) == 4);
		assert(a.read(2) == 3);
		assert(a.read(3) == 2);
		assert(a.newest() == 5);
		assert(a.oldest() == 2);

		a.next() = 6;
		assert(a.newest() == 6);

		a.reset();
		assert(a.fill() == 0);
	}

	return 0;
}


