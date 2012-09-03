#ifndef INC_AL_UTIL_FIELD3D_HPP
#define INC_AL_UTIL_FIELD3D_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, 
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright 
		notice, this list of conditions and the following disclaimer in the 
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its 
		contributors may be used to endorse or promote products derived from 
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	A collection of utilites for 3D fields (including Jos Stam's fluids)

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	
*/


#include "allocore/types/al_Array.hpp"
#include "allocore/math/al_Functions.hpp"
#include "allocore/math/al_Random.hpp"

namespace al {

/*!
	Field processing often requires double-buffering
*/

template<typename T=float>
class Field3D {
public:
	
	Field3D(int components=1, int dimx=32, int dimy=32, int dimz=32) 
	:	mDimX(ceilPow2(dimx)), 
		mDimY(ceilPow2(dimy)), 
		mDimZ(ceilPow2(dimz)),
		mDim3(mDimX*mDimY*mDimZ),
		mDimWrapX(mDimX-1),
		mDimWrapY(mDimY-1),
		mDimWrapZ(mDimZ-1),
		mFront(1),
		mArray0(components, Array::type<T>(), mDimX, mDimY, mDimZ),
		mArray1(components, Array::type<T>(), mDimX, mDimY, mDimZ)
	{}
	
	~Field3D() {}
	
	unsigned length() const { return components()*mDim3; }
	unsigned components() const { return mArray0.header.components; }
	unsigned dimx() const { return mDimX; }
	unsigned dimy() const { return mDimY; }
	unsigned dimz() const { return mDimZ; }
	size_t stride(int dim=0) const { return mArray0.header.stride[dim]; }
	
	// front is what is currently interacted with
	// back is used for intermediate processing
	Array& front() { return mFront ? mArray0 : mArray1; }
	Array& back() { return mFront ? mArray1 : mArray0; }
	const Array& front() const { return mFront ? mArray0 : mArray1; }
	const Array& back() const { return mFront ? mArray1 : mArray0; }
	
	// returns true if the vector v is outside of the field boundaries
	template <typename T1>
	inline bool oob(Vec<3,T1> v);
	
	/// read the intensity at a particular location:
	template<typename T1>
	void read(const Vec<3,T1> pos, T * elems) const;
	
	// raw access to internal pointer
	T * ptr() { return (T *)front().data.ptr; }
	// pointer index of a particular cell:
	size_t index(int x, int y, int z) const;
	// access a particular element:
	T& cell(int x, int y, int z, int k=0);
	
	// swap buffers:
	void swap() { mFront = !mFront; }

	/// multiply the front array:
	void scale(T v);
	/// src must have matching layout
	void scale(const Array& src);	
	/// src must have matching layout, except for components (which may be 1)
	void scale1(const Array& src);	
	/// multiply by 1./(src+1.), as a 'damping' factor:
	void damp(const Array& src);	// src must have matching layout
	/// add to front array. src must have matching layout
	void add(Array& src);
	/// add uniform vector to front array. vec must have as many components as the field
	void add(T * vec);
	/// add intensity at a particular location:
	template<typename T1>
	void add(const Vec<3,T1> pos, const T * elems);
	void add(int x, int y, int z, const T * elems);
	/// single component case:
	template<typename T1>
	void add(const Vec<3,T1> pos, T elem);
	// fill with noise:
	void adduniform(rnd::Random<>& rng, T scalar = T(1));
	void adduniformS(rnd::Random<>& rng, T scalar = T(1));
	// fill with sines:
	void setHarmonic(T px=T(1), T py=T(1), T pz=T(1));
	// 3-component fields only: scale velocities at boundaries
	void boundary();
	
	// diffusion
	void diffuse(T diffusion=T(0.01), unsigned passes=14);
	
	/// Diffusion with arbitrary kernel:
	/// the kernel layout:
	enum CellIndex {			
		C022, C122, C222,
		C012, C112, C212,
		C002, C102, C202,
		
		C021, C121, C221,
		C011, C111, C211,
		C001, C101, C201,
		
		C020, C120, C220,
		C010, C110, C210,
		C000, C100, C200,
		
		CINVALID
	};
	struct Kernel3 {
		T coeffs[27];
		
		Kernel3() { zero(); }
		
		void blur();
		void blur3();
		
		void zero() { memset(coeffs, 0, 27*sizeof(T)); }
		/// generates a value for coeffs[C111] that is the negative of
		/// the sum of all other coeffs (to create a balanced kernel)
		void calculateCenter();
	};
	void diffuse(const Kernel3& kernel, T diffusion=T(0.01), unsigned passes=14);
	
	// advect a field.
	// velocity field should have 3 components
	void advect(const Array& velocities, T rate = T(1.));
	static void advect(Array& dst, const Array& src, const Array& velocities, T rate = T(1.));
	
	/*
		Clever part of Jos Stam's work.
			A velocity field can become divergent (have regions that are purely emanating or aggregating)
				violating the definition of an incompressible fluid
			But, since a velocity field can be seen as an incompressible velocity field + a gradient field,
				we can subtract a gradient field from our bad velocity field to get an incompressible one
			To calculate this gradient field and then subtract it, we use this function:
	*/
	// grabs the previous instantaneous magnitude of velocity gradient
	void calculateGradientMagnitude(Array& gradient);
	void subtractGradientMagnitude(const Array& gradient);
	
	void relax(double a, int iterations);
	
protected:
	size_t mDimX, mDimY, mDimZ, mDim3, mDimWrapX, mDimWrapY, mDimWrapZ;
	volatile int mFront;	// which one is the front buffer?
	Array mArray0, mArray1; //mArrays[2];	// double-buffering
};

template<typename T=float>
class Fluid3D {
public:
	enum BoundaryMode {
		NONE = 0,
		CLAMP = 1,
		FIELD = 2
	};

	Fluid3D(int dimx=32, int dimy=32, int dimz=32) 
	:	velocities(3, dimx, dimy, dimz),
		gradient(1, dimx, dimy, dimz),
		boundaries(1, Array::type<T>(), dimx, dimy, dimz),
		passes(14),	
		viscocity(0.00001),
		selfadvection(0.9),
		selfdecay(0.99),
		selfbackgroundnoise(0.001),
		mBoundaryMode(CLAMP)
	{
		// set all values to T(1):
		T one = 1;
		boundaries.set3d(&one);
	}
	~Fluid3D() {}
	
	template<typename T1>
	void addVelocity(const Vec<3,T1> pos, const Vec<3,T> vel) {
		velocities.add(pos, vel.elems());
	}
	void addVelocity(int x, int y, int z, const Vec<3,T> vel) {
		velocities.add(x, y, z, vel.elems());
	}
	
	template<typename T1>
	void readVelocity(const Vec<3,T1> pos, Vec<3,T>& vel) const {
		velocities.read(pos, vel.elems());
	}
	
	template<typename T1>
	void addGradient(const Vec<3,T1> pos, float& g) {
		gradient.add(pos, &g);
	}
	
	template<typename T1>
	void readGradient(const Vec<3,T1> pos, float& g) const {
		gradient.read(pos, &g);
	}
	
	/// fluid simulation step
	// TODO: add dt param
	void update() {
		// VELOCITIES:
		// add a bit of random noise:
		velocities.adduniformS(rng, selfbackgroundnoise);
		// assume new data is in front();
		// smoothen the new data:
		velocities.diffuse(viscocity, passes);
		// zero velocities at boundaries:
		boundary();
		// (diffused data now in velocities.front())
		// stabilize: 
		project();
		// (projected data now in velocities.front())
		// advect velocities:
		velocities.advect(velocities.back(), selfadvection);
		// zero velocities at boundaries:
		boundary();
		// (advected data now in velocities.front())
		// stabilize again:
		project();
		// (projected data now in velocities.front())
		velocities.scale(selfdecay);
		// zero velocities at boundaries:
		boundary();
		
		gradient.front().zero();
		gradient.back().zero();
	}
	
	void boundary() {
		switch (mBoundaryMode) {
			case CLAMP:
				velocities.boundary();
				break;
			case FIELD:
				velocities.scale1(boundaries);
				break;
			default:
				break;
		}
	}
	
	void project() {
		gradient.back().zero();
		// prepare new gradient data:
		velocities.calculateGradientMagnitude(gradient.front());
		// diffuse it:
		gradient.diffuse(0.5, passes/2);
		// subtract from current velocities:
		velocities.subtractGradientMagnitude(gradient.front());
	}
	
	void boundary(BoundaryMode b) { mBoundaryMode = b; }
	
	Field3D<T> velocities, gradient;
	Array boundaries;
	unsigned passes;
	T viscocity, selfadvection, selfdecay, selfbackgroundnoise;
	rnd::Random<> rng;
	BoundaryMode mBoundaryMode;
};


// a combination of above classes for simplicity:
template<typename T=float>
class FluidField3D : public Fluid3D<T> {
public:
	typedef Fluid3D<T> Super;

	FluidField3D(int components, int dim) 
	:	Super(dim),
		densities(components, dim),
		diffusion(0.001),
		decay(0.98)
	{}
	~FluidField3D() {}
	
	template<typename T1>
	void addDensities(const Vec<3,T1> pos, const T * elems) {
		densities.front().write_interp(elems, pos);
	}
	
	template<typename T1>
	void readDensities(const Vec<3,T1> pos, T * elems) {
		densities.front().read_interp(elems, pos);
	}
	
	/// fluid simulation step
	// TODO: add dt param
	void update() {
		// VELOCITIES:
		Super::update();
		
		// DENSITIES:
		// assume new data is in front();
		// smoothen the new data:
		densities.diffuse(diffusion, Super::passes);
		//(diffused data now in densities.front())
		// and advect:
		densities.advect(Super::velocities.front());
		//(advected data now in densities.front())
		// fade, etc.
		densities.scale(decay);
	}
	
	Field3D<T> densities;
	T diffusion, decay;
};

// INLINE IMPLEMENTATION //

// returns true if the vector v is outside of the field boundaries
template <typename T>
template <typename T1>
inline bool Field3D<T>::oob(Vec<3,T1> v) {
	return (v[0] < 0 || v[0] >= dimx() 
		 || v[1] < 0 || v[1] >= dimy() 
		 || v[2] < 0 || v[2] >= dimz());
}

template<typename T>
inline void Field3D<T>::Kernel3::calculateCenter() {
	coeffs[C111] = -(
		coeffs[C022] + coeffs[C122] + coeffs[C222] + 
		coeffs[C012] + coeffs[C112] + coeffs[C212] + 
		coeffs[C002] + coeffs[C102] + coeffs[C202] + 
		
		coeffs[C021] + coeffs[C121] + coeffs[C221] + 
		coeffs[C011] +				  coeffs[C211] + 
		coeffs[C001] + coeffs[C101] + coeffs[C201] + 
		
		coeffs[C020] + coeffs[C120] + coeffs[C220] + 
		coeffs[C010] + coeffs[C110] + coeffs[C210] + 
		coeffs[C000] + coeffs[C100] + coeffs[C200]
	);
}

template<typename T>
inline void Field3D<T>::Kernel3::blur() {
	zero();
		
	// axes:
	coeffs[Field3D<float>::C010] = 
	coeffs[Field3D<float>::C012] = 
	coeffs[Field3D<float>::C212] = 
	coeffs[Field3D<float>::C210] = 
	coeffs[Field3D<float>::C121] = 
	coeffs[Field3D<float>::C101] = 1.;
		
	// seedskernel[Field3D::C111] = 0.;	// the center value
	calculateCenter();
}

template<typename T>
inline void Field3D<T>::Kernel3::blur3() {
	zero();
	
	// diagonals (12)
	coeffs[Field3D<float>::C122] = 
	coeffs[Field3D<float>::C120] = 
	coeffs[Field3D<float>::C021] = 
	coeffs[Field3D<float>::C221] = 	
	
	coeffs[Field3D<float>::C012] = 
	coeffs[Field3D<float>::C010] = 
	coeffs[Field3D<float>::C210] = 
	coeffs[Field3D<float>::C212] = 
	
	coeffs[Field3D<float>::C102] = 
	coeffs[Field3D<float>::C100] = 
	coeffs[Field3D<float>::C001] = 
	coeffs[Field3D<float>::C201] = 1.;
	
	// axes (6):
	coeffs[Field3D<float>::C110] = 
	coeffs[Field3D<float>::C112] = 
	coeffs[Field3D<float>::C011] = 
	coeffs[Field3D<float>::C211] = 
	coeffs[Field3D<float>::C121] = 
	coeffs[Field3D<float>::C101] = 2.;
		
	// seedskernel[Field3D::C111] = 0.;	// the center value
	calculateCenter();
}

template<typename T>
inline size_t Field3D<T>::index(int x, int y, int z) const {
	return	((x&mDimWrapX) * stride(0)) +
			((y&mDimWrapY) * stride(1)) +
			((z&mDimWrapZ) * stride(2));
}

template<typename T>
inline T& Field3D<T>::cell(int x, int y, int z, int k) {
	return ptr()[index(x, y, z) + k];
}

template<typename T>
template<typename T1>
inline void Field3D<T>::add(const Vec<3,T1> pos, const T * elems) {
	front().write_interp(elems, pos);
}

template<typename T>
inline void Field3D<T>::add(int x, int y, int z, const T * elems) {
	front().write(elems, x, y, z);
}
// single component case:
template<typename T>
template<typename T1>
inline void Field3D<T>::add(const Vec<3,T1> pos, T elem) {
	front().write_interp(&elem, pos);
}

template<typename T>
template<typename T1>
inline void Field3D<T>::read(const Vec<3,T1> pos, T * elems) const {
	if (mFront) {
		mArray0.read_interp(elems, pos);
	} else {
		mArray1.read_interp(elems, pos);
	}
}

template<typename T>
inline void Field3D<T>::setHarmonic(T px, T py, T pz) {
	T vals[3];
	for (size_t z=0;z<dimx();z++) {
		vals[2] = sin(pz * M_2PI * z/T(dimx()));
		for (size_t y=0;y<dimy();y++) {
			vals[1] = sin(py * M_2PI * y/T(dimy()));
			for (size_t x=0;x<dimz();x++) {
				vals[0] = sin(px * M_2PI * x/T(dimz()));
				T value = vals[0] * vals[1] * vals[2];
				front().write(&value, x, y, z);
			}
		}
	}
}

template<typename T>
inline void Field3D<T>::adduniformS(rnd::Random<>& rng, T scalar) {
	T * p = ptr();
	for (unsigned k=0;k<length();k++) p[k] += scalar * rng.uniformS();
}
template<typename T>
inline void Field3D<T>::adduniform(rnd::Random<>& rng, T scalar) {
	T * p = ptr();
	for (unsigned k=0;k<length();k++) p[k] += scalar * rng.uniform();
}

template<typename T>
inline void Field3D<T>::scale(T v) {
	T * p = ptr();
	for (unsigned k=0;k<length();k++) p[k] *= v;
}

template<typename T>
inline void Field3D<T>::scale(const Array& arr) {
	if (arr.isFormat(front().header)) {
		const size_t stride0 = stride(0);
		const size_t stride1 = stride(1);
		const size_t stride2 = stride(2);
		#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrapX)*stride0) +  (((y)&mDimWrapY)*stride1) + (((z)&mDimWrapZ)*stride2)))

		// zero the boundary fields
		char * optr = front().data.ptr;
		char * vptr = arr.data.ptr;
		for (size_t z=0;z<mDimZ;z++) {
			for (size_t y=0;y<mDimY;y++) {
				for (size_t x=0;x<mDimX;x++) {
					// cell to update:
					T * cell = INDEX(optr, x, y, z);
					T * src = INDEX(vptr, x, y, z);

					for (unsigned k=0; k<components(); k++) {
						cell[k] *= src[k];
					}
				}
			}
		}

		#undef INDEX
	} else {
		printf("Field3D::scale() Array format mismatch\n");
	}
}

template<typename T>
inline void Field3D<T>::scale1(const Array& arr) {

	AlloArrayHeader h1 = front().header;
	AlloArrayHeader h2 = arr.header;
	
	bool ok =	
		h1.type == h2.type &&
		h1.dimcount == h2.dimcount;
	for(int i=0; i < h1.dimcount; i++) {
		ok &= h1.dim[i] <= h2.dim[i];
	}
	
	if (ok) {
		const size_t stride0 = stride(0);
		const size_t stride1 = stride(1);
		const size_t stride2 = stride(2);
		
		const size_t astride0 = arr.stride(0);
		const size_t astride1 = arr.stride(1);
		const size_t astride2 = arr.stride(2);
		#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrapX)*stride0) +  (((y)&mDimWrapY)*stride1) +  (((z)&mDimWrapZ)*stride2)))
		
		#define AINDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrapX)*astride0) +  (((y)&mDimWrapY)*astride1) +  (((z)&mDimWrapZ)*astride2)))

		// scale the boundary fields
		char * optr = front().data.ptr;
		char * aptr = arr.data.ptr;
		for (size_t z=0;z<mDimZ;z++) {
			for (size_t y=0;y<mDimY;y++) {
				for (size_t x=0;x<mDimX;x++) {
					// cell to update:
					T * cell = INDEX(optr, x, y, z);
					T v = AINDEX(aptr, x, y, z)[0];
					for (unsigned k=0; k<components(); k++) {
						cell[k] *= v;
					}
				}
			}
		}

		#undef INDEX
		#undef AINDEX
	} else {
		printf("Field3D::scale1() Array format mismatch\n");
	}
}

template<typename T>
inline void Field3D<T>::damp(const Array& arr) {
	if (arr.isFormat(front().header)) {
		const size_t stride0 = stride(0);
		const size_t stride1 = stride(1);
		const size_t stride2 = stride(2);
		#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrapX)*stride0) +  (((y)&mDimWrapY)*stride1) +  (((z)&mDimWrapZ)*stride2)))

		// zero the boundary fields
		char * optr = front().data.ptr;
		char * vptr = arr.data.ptr;
		for (size_t z=0;z<mDimZ;z++) {
			for (size_t y=0;y<mDimY;y++) {
				for (size_t x=0;x<mDimX;x++) {
					// cell to update:
					T * cell = INDEX(optr, x, y, z);
					T v = fabs(INDEX(vptr, x, y, z)[0]);

					for (unsigned k=0; k<components(); k++) {
						cell[k] *= 1./(v+1.);
					}
				}
			}
		}

		#undef INDEX
	} else {
		printf("Array format mismatch\n");
	}
}
	
template<typename T>
inline void Field3D<T>::add(Array& src) {
	if (src.isFormat(front().header)) {
		int count = mDim3 * front().header.components;
		T * in  = (T *)src.data.ptr;
		T * out = (T *)front().data.ptr;
		for (int i=0; i<count; i++) {
			out[i] += in[i];
		}
	} else {
		printf("Array format mismatch\n");
	}
}

// Gauss-Seidel relaxation scheme:
template<typename T>
inline void Field3D<T> :: diffuse(T diffusion, unsigned passes) {
	swap(); 
	Array& out = front();
	const Array& in = back();
	const size_t stride0 = out.header.stride[0];
	const size_t stride1 = out.header.stride[1];
	const size_t stride2 = out.header.stride[2];
	const size_t components = out.header.components;
	const char * iptr = in.data.ptr;
	const char * optr = out.data.ptr;
	double div = 1.0/((1.+6.*diffusion));
	#define INDEX(p, x, y, z) ((T *)(p + ((((x)&mDimWrapX)*stride0) + (((y)&mDimWrapY)*stride1) + (((z)&mDimWrapZ)*stride2))))
	
	for (unsigned n=0 ; n<passes ; n++) {
		for (size_t z=0;z<mDimZ;z++) {
			for (size_t y=0;y<mDimY;y++) {
				for (size_t x=0;x<mDimX;x++) {
					const T * prev =	INDEX(iptr, x,	y,	z);
					T *		  next =	INDEX(optr, x,	y,	z);
					const T * va00 =	INDEX(optr, x-1,y,	z);
					const T * vb00 =	INDEX(optr, x+1,y,	z);
					const T * v0a0 =	INDEX(optr, x,	y-1,z);
					const T * v0b0 =	INDEX(optr, x,	y+1,z);
					const T * v00a =	INDEX(optr, x,	y,	z-1);
					const T * v00b =	INDEX(optr, x,	y,	z+1);
					for (size_t k=0;k<components;k++) {
						next[k] = div*(
							prev[k] +
							diffusion * (
								va00[k] + vb00[k] +
								v0a0[k] + v0b0[k] +
								v00a[k] + v00b[k]
							)
						);
					}
				}
			}
		}
	}
	#undef INDEX
}

// Gauss-Seidel relaxation scheme:
template<typename T>
inline void Field3D<T> :: diffuse(const Kernel3& kernel, T diffusion, unsigned passes) {
	swap(); 
	Array& out = front();
	const Array& in = back();
	const T a = -kernel.coeffs[C111]; // kernel center value
	const T afactor = 1./(1. + a*diffusion);	// balancing factor for relaxation scheme
	const size_t stride0 = out.header.stride[0];
	const size_t stride1 = out.header.stride[1];
	const size_t stride2 = out.header.stride[2];
	const size_t components = out.header.components;
	const char * iptr = in.data.ptr;
	const char * optr = out.data.ptr;
	#define INDEX(p, x, y, z) ((T *)(p + ((((x)&mDimWrapX)*stride0) + (((y)&mDimWrapY)*stride1) + (((z)&mDimWrapZ)*stride2))))
	
	for (unsigned n=0 ; n<passes ; n++) {
		for (size_t z=0;z<mDimZ;z++) {
			for (size_t y=0;y<mDimY;y++) {
				for (size_t x=0;x<mDimX;x++) {
					const T * prev =	INDEX(iptr, x,	y,	z);
					T *		  next =	INDEX(optr, x,	y,	z);
					// immediate neighbors
					const T * v011 = INDEX(optr, x-1, y, z);
					const T * v211 = INDEX(optr, x+1, y, z);
					const T * v101 = INDEX(optr, x, y-1, z);
					const T * v121 = INDEX(optr, x, y+1, z);
					const T * v110 = INDEX(optr, x, y, z-1);
					const T * v112 = INDEX(optr, x, y, z+1);
					// mid corner neighbors
					const T * v010 = INDEX(optr, x-1, y, z-1);
					const T * v100 = INDEX(optr, x, y-1, z-1);
					const T * v120 = INDEX(optr, x, y+1, z-1);
					const T * v210 = INDEX(optr, x+1, y, z-1);
					const T * v001 = INDEX(optr, x-1, y-1, z);
					const T * v201 = INDEX(optr, x+1, y-1, z);
					const T * v021 = INDEX(optr, x-1, y+1, z);
					const T * v221 = INDEX(optr, x+1, y+1, z);
					const T * v012 = INDEX(optr, x-1, y, z+1);
					const T * v102 = INDEX(optr, x, y-1, z+1);
					const T * v122 = INDEX(optr, x, y+1, z+1);
					const T * v212 = INDEX(optr, x+1, y, z+1);
					// far corner neighbors
					const T * v000 = INDEX(optr, x-1, y-1, z-1);
					const T * v200 = INDEX(optr, x+1, y-1, z-1);
					const T * v020 = INDEX(optr, x-1, y+1, z-1);
					const T * v220 = INDEX(optr, x+1, y+1, z-1);
					const T * v002 = INDEX(optr, x-1, y-1, z+1);
					const T * v202 = INDEX(optr, x+1, y-1, z+1);
					const T * v022 = INDEX(optr, x-1, y+1, z+1);
					const T * v222 = INDEX(optr, x+1, y+1, z+1);
					
					for (size_t k=0;k<components;k++) {
						next[k] = afactor * (
										prev[k] +
										diffusion * (
											kernel.coeffs[C022] * v022[k] +
											kernel.coeffs[C122] * v122[k] +
											kernel.coeffs[C222] * v222[k] +
											kernel.coeffs[C012] * v012[k] +
											kernel.coeffs[C112] * v112[k] +
											kernel.coeffs[C212] * v212[k] +
											kernel.coeffs[C002] * v002[k] +
											kernel.coeffs[C102] * v102[k] +
											kernel.coeffs[C202] * v202[k] +
											
											kernel.coeffs[C021] * v021[k] +
											kernel.coeffs[C121] * v121[k] +
											kernel.coeffs[C221] * v221[k] +
											kernel.coeffs[C011] * v011[k] +
											// no C111 term!
											kernel.coeffs[C211] * v211[k] +
											kernel.coeffs[C001] * v001[k] +
											kernel.coeffs[C101] * v101[k] +
											kernel.coeffs[C201] * v201[k] +
											
											kernel.coeffs[C020] * v020[k] +
											kernel.coeffs[C120] * v120[k] +
											kernel.coeffs[C220] * v220[k] +
											kernel.coeffs[C010] * v010[k] +
											kernel.coeffs[C110] * v110[k] +
											kernel.coeffs[C210] * v210[k] +
											kernel.coeffs[C000] * v000[k] +
											kernel.coeffs[C100] * v100[k] +
											kernel.coeffs[C200] * v200[k]
										)
									);
					}
					
				}
			}
		}
	}
	#undef INDEX
}

/*
	The solver uses a Merhstellen kernel:
	0, 1, 0		1, b, 1		0, 1, 0
	1, b, 1		b, -a, b	1, b, 1
	0, 1, 0		1, b, 1		0, 1, 0
	
	b = 4, a = 36
	b = 2, a = 24
*/
template<typename T>
inline void Field3D<T> :: relax( double diffusion, int iterations) {
	char * old = front().data.ptr;
	char * out = back().data.ptr;
	const uint32_t comps = components();
#ifdef NoMerhstellen
	const double c = 1./(1. + 6.*diffusion); 
#else
	const double c = 1./(1. + 24.*diffusion); 
#endif
	for (int iter=0; iter<iterations; iter++) {
		for (size_t z=0;z<mDimZ;z++) {
			for (size_t y=0;y<mDimY;y++) {
				for (size_t x=0;x<mDimX;x++) {
					const size_t idx = index(x, y, z);
#ifdef NoMerhstellen
					const size_t x0 = index(x-1, y, z);
					const size_t x1 = index(x+1, y, z);
					const size_t y0 = index(x, y-1, z);
					const size_t y1 = index(x, y+1, z);
					const size_t z0 = index(x, y, z-1);
					const size_t z1 = index(x, y, z+1);
					for (unsigned k=0; k<comps; k++) {
						out[idx+k] = c * (
										old[idx+k] +
										diffusion * (	
											out[x0+k] + out[x1+k] +
											out[y0+k] + out[y1+k] +
											out[z0+k] + out[z1+k]
										)
									);
					}
#else
					// immediate neighbors
					const size_t idx011 = index(x-1, y, z);
					const size_t idx211 = index(x+1, y, z);
					const size_t idx101 = index(x, y-1, z);
					const size_t idx121 = index(x, y+1, z);
					const size_t idx110 = index(x, y, z-1);
					const size_t idx112 = index(x, y, z+1);
					// corner neighbors
					const size_t idx010 = index(x-1, y, z-1);
					const size_t idx100 = index(x, y-1, z-1);
					const size_t idx120 = index(x, y+1, z-1);
					const size_t idx210 = index(x+1, y, z-1);
					const size_t idx001 = index(x-1, y-1, z);
					const size_t idx201 = index(x+1, y-1, z);
					const size_t idx021 = index(x-1, y+1, z);
					const size_t idx221 = index(x+1, y+1, z);
					const size_t idx012 = index(x-1, y, z+1);
					const size_t idx102 = index(x, y-1, z+1);
					const size_t idx122 = index(x, y+1, z+1);
					const size_t idx212 = index(x+1, y, z+1);
					for (unsigned k=0; k<comps; k++) {
						out[idx+k] = c * (
										old[idx+k] +
										diffusion * (2.*(	
											out[idx011+k] + out[idx211+k] +
											out[idx101+k] + out[idx121+k] +
											out[idx110+k] + out[idx112+k]
										) + out[idx010+k] + out[idx100+k] +
											out[idx120+k] + out[idx210+k] +
											out[idx001+k] + out[idx201+k] +
											out[idx021+k] + out[idx221+k] +
											out[idx012+k] + out[idx102+k] +
											out[idx122+k] + out[idx212+k]
										)
									);
					}
#endif
				}
			}
		}
		// todo: apply boundary here?
	}
	#undef CELL
}

template<typename T>
inline void Field3D<T> :: advect(Array& dst, const Array& src, const Array& velocities, T rate) {
	const size_t stride0 = src.stride(0);
	const size_t stride1 = src.stride(1);
	const size_t stride2 = src.stride(2);
	const size_t dim0 = src.dim(0);
	const size_t dim1 = src.dim(1);
	const size_t dim2 = src.dim(2);
	const size_t dimwrap0 = dim0-1;
	const size_t dimwrap1 = dim1-1;
	const size_t dimwrap2 = dim2-1;
	
	if (velocities.header.type != src.header.type ||
		velocities.header.components < 3 ||
		velocities.header.dim[0] != dim0 ||
		velocities.header.dim[1] != dim1 ||
		velocities.header.dim[2] != dim2) 
	{
		printf("Array format mismatch\n");
		return;
	}
	char * outptr = dst.data.ptr;
	char * velptr = velocities.data.ptr;
	
	const size_t vstride0 = velocities.stride(0);
	const size_t vstride1 = velocities.stride(1);
	const size_t vstride2 = velocities.stride(2);
	
	#define CELL(p, x, y, z, k) (((T *)((p) + (((x)&dimwrap0)*stride0) +  (((y)&dimwrap1)*stride1) +  (((z)&dimwrap2)*stride2)))[(k)])
	#define VCELL(p, x, y, z, k) (((T *)((p) + (((x)&dimwrap0)*vstride0) +  (((y)&dimwrap1)*vstride1) +  (((z)&dimwrap2)*vstride2)))[(k)]) 
	
	for (size_t z=0;z<dim2;z++) {
		for (size_t y=0;y<dim1;y++) {
			for (size_t x=0;x<dim0;x++) {
				// back trace: (current cell offset by vector at cell)
				T * bp  = &(CELL(outptr, x, y, z, 0));
				T * vp	= &(VCELL(velptr, x, y, z, 0));
				T vx = x - rate * vp[0];
				T vy = y - rate * vp[1];
				T vz = z - rate * vp[2];

				// read interpolated input field value into back-traced location:
				src.read_interp(bp, vx, vy, vz);
			}
		}
	}
	#undef CELL
	#undef VCELL
}

template<typename T>
inline void Field3D<T> :: advect(const Array& velocities, T rate) {
	swap();
	advect(front(), back(), velocities, rate);
}

template<typename T>
inline void Field3D<T> :: calculateGradientMagnitude(Array& gradient) {
	gradient.format(1, Array::type<T>(), mDimX, mDimY, mDimZ);
	
	const size_t stride0 = stride(0);
	const size_t stride1 = stride(1);
	const size_t stride2 = stride(2);
	const size_t gstride0 = gradient.stride(0);
	const size_t gstride1 = gradient.stride(1);
	const size_t gstride2 = gradient.stride(2);
	
	#define CELL(p, x, y, z, k) (((T *)((p) + (((x)&mDimWrapX)*stride0) +  (((y)&mDimWrapY)*stride1) +  (((z)&mDimWrapZ)*stride2)))[(k)])
	#define CELLG(p, x, y, z) (((T *)((p) + (((x)&mDimWrapX)*gstride0) +  (((y)&mDimWrapY)*gstride1) +  (((z)&mDimWrapZ)*gstride2)))[0])
	
	// calculate gradient.
	// previous instantaneous magnitude of velocity gradient
	//		= average of velocity gradients per axis:
	const double hx = -0.5/mDimX; //1./3.; //0.5/mDim;
	const double hy = -0.5/mDimY; //1./3.; //0.5/mDim;
	const double hz = -0.5/mDimZ; //1./3.; //0.5/mDim;
	char * iptr = front().data.ptr;
	char * gptr = gradient.data.ptr;
	
	for (size_t z=0;z<mDimZ;z++) {
		for (size_t y=0;y<mDimY;y++) {
			for (size_t x=0;x<mDimX;x++) {
				// gradients per axis:
				const T xgrad = CELL(iptr, x+1,y,	z,	0) - CELL(iptr, x-1,y,	z,	0);
				const T ygrad = CELL(iptr, x,	y+1,z,	1) - CELL(iptr, x,	y-1,z,	1);
				const T zgrad = CELL(iptr, x,	y,	z+1,2) - CELL(iptr, x,	y,	z-1,2);
				// gradient at current cell:
				const T grad = hx*xgrad + hy*ygrad + hz*zgrad;
				// add to 1-plane field
				CELLG(gptr, x, y, z) += grad;
			}
		}
	}
	#undef CELL
	#undef CELLG
}



template<typename T>
inline void Field3D<T> :: subtractGradientMagnitude(const Array& gradient) {
	if (gradient.header.type != Array::type<T>() ||
		gradient.header.dim[0] != mDimX ||
		gradient.header.dim[1] != mDimY ||
		gradient.header.dim[2] != mDimZ) 
	{
		printf("Array format mismatch\n");
		return;
	}
	const size_t stride0 = stride(0);
	const size_t stride1 = stride(1);
	const size_t stride2 = stride(2);
	const size_t gstride0 = gradient.stride(0);
	const size_t gstride1 = gradient.stride(1);
	const size_t gstride2 = gradient.stride(2);
	
	#define INDEX(p, x, y, z) ((T *)(p + ((((x)&mDimWrapX)*stride0) + (((y)&mDimWrapY)*stride1) + (((z)&mDimWrapZ)*stride2))))
	#define CELLG(p, x, y, z) (((T *)((p) + (((x)&mDimWrapX)*gstride0) +  (((y)&mDimWrapY)*gstride1) +  (((z)&mDimWrapZ)*gstride2)))[0])
	
	// now subtract gradient from current field:
	char * gptr = gradient.data.ptr;
	char * optr = front().data.ptr;
	//const double h = 1.; ///3.;
	const double hx = mDimX * 0.5;
	const double hy = mDimY * 0.5;
	const double hz = mDimZ * 0.5;
	for (size_t z=0;z<mDimZ;z++) {
		for (size_t y=0;y<mDimY;y++) {
			for (size_t x=0;x<mDimX;x++) {
				// cell to update:
				T * vel = INDEX(optr, x, y, z);
				// gradients per axis:
				vel[0] -= hx * ( CELLG(gptr, x+1,y,	z  ) - CELLG(gptr, x-1,y,	z  ) );
				vel[1] -= hy * ( CELLG(gptr, x,	y+1,z  ) - CELLG(gptr, x,	y-1,z  ) );
				vel[2] -= hz * ( CELLG(gptr, x,	y,	z+1) - CELLG(gptr, x,	y,	z-1) );
			}
		}
	}

	#undef INDEX
	#undef CELLG
}

template<typename T>
inline void Field3D<T> :: boundary() {
	if (front().header.components < 3) {
		printf("only valid for 3-component fields\n");
		return;
	}
	

	const size_t stride0 = stride(0);
	const size_t stride1 = stride(1);
	const size_t stride2 = stride(2);
	char * optr = front().data.ptr;

	#define CELL(p, x, y, z, k) (((T *)((p) + (((x)&mDimWrapX)*stride0) +  (((y)&mDimWrapY)*stride1) +  (((z)&mDimWrapZ)*stride2)))[(k)])
	
	// x planes
	for (size_t z=0;z<mDimZ;z++) {
		for (size_t y=0;y<mDimY;y++) {
			CELL(optr, 0, y, z, 0) = T(0);
			CELL(optr, mDimWrapX, y, z, 0) = T(0); 
		}
	}
	// y planes
	for (size_t z=0;z<mDimZ;z++) {
		for (size_t x=0;x<mDimX;x++) {
			CELL(optr, x, 0, z, 1) = T(0);
			CELL(optr, x, mDimWrapY, z, 1) = T(0); 
		}
	}
	// z planes
	for (size_t y=0;y<mDimY;y++) {
		for (size_t x=0;x<mDimX;x++) {
			CELL(optr, x, y, 0, 2) = T(0);
			CELL(optr, x, y, mDimWrapZ, 2) = T(0); 
		}
	}
	
	#undef CELL
	
}

template<typename T>
inline void Field3D<T> :: add(T * force) {
	const uint32_t stride0 = front().stride(0);
	const uint32_t stride1 = front().stride(1);
	const uint32_t stride2 = front().stride(2);
	const uint32_t dim0 = front().dim(0);
	const uint32_t dim1 = front().dim(1);
	const uint32_t dim2 = front().dim(2);
	const uint8_t components = front().components();
	#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrapX)*stride0) +  (((y)&mDimWrapY)*stride1) +  (((z)&mDimWrapZ)*stride2)))

	// zero the boundary fields
	char * optr = front().data.ptr;
	for (size_t z=0;z<dim2;z++) {
		for (size_t y=0;y<dim1;y++) {
			for (size_t x=0;x<dim0;x++) {
				// cell to update:
				T * vel = INDEX(optr, x, y, z);
				for (int i=0; i<components; i++ ){
					vel[i] += force[i];
				}
			}
		}
	}

	#undef INDEX
	#undef CELLG
}


}; // al
#endif
