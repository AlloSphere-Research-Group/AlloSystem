#ifndef INC_AL_UTIL_FIELD3D_HPP
#define INC_AL_UTIL_FIELD3D_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	A collection of utilites for 3D fields (including Jos Stam's fluids)

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/types/al_Array.hpp"
#include "allocore/math/al_Functions.hpp"

namespace al {

/*!
	Field processing often requires double-buffering
*/

template<typename T=double>
class Field3D {
public:
	
	Field3D(int components, int dim) 
	:	mDim(ceilPow2(dim)),
		mDim3(mDim*mDim*mDim),
		mDimWrap(mDim-1),
		mFront(1),
		mArray0(components, Array::type<T>(), mDim, mDim, mDim),
		mArray1(components, Array::type<T>(), mDim, mDim, mDim)
	{}
	
	~Field3D() {
		mArray0.dataFree();
		mArray1.dataFree();
	}
	
	unsigned length() const { return components()*mDim3; }
	unsigned components() const { return mArray0.header.components; }
	unsigned dim() const { return mDim; }
	size_t stride(int dim=0) const { return mArray0.header.stride[dim]; }
	
	// front is what is currently interacted with
	// back is used for intermediate processing
	Array& front() { return mFront ? mArray0 : mArray1; }
	Array& back() { return mFront ? mArray1 : mArray0; }
	
	// raw access to internal pointer
	T * ptr() { return (T *)front().data.ptr; }
	// pointer index of a particular cell:
	size_t index(int x, int y, int z) const;
	// access a particular element:
	T& cell(int x, int y, int z, int k=0);
	
	// swap buffers:
	void swap() { mFront = !mFront; }

	// multiply the front array:
	void scale(T v);
	// src must have matching layout
	void scale(const Array& src);	
	// src must have matching layout, except for components (which may be 1)
	void scale1(const Array& src);	
	// multiply by 1./(src+1.), as a 'damping' factor:
	void damp(const Array& src);	// src must have matching layout
	// add to front array. src must have matching layout
	void add(Array& src);
	// add uniform vector to front array. force must have as many components as the field
	void add(T * force);
	
	// fill with noise:
	void adduniform(rnd::Random<>& rng, T scalar = T(1));
	void adduniformS(rnd::Random<>& rng, T scalar = T(1));
	// fill with sines:
	void setHarmonic(T px=T(1), T py=T(1), T pz=T(1));
	// 3-component fields only: scale velocities at boundaries
	void boundary();
	
	// diffusion
	void diffuse(T diffusion=T(0.01), int passes=10);
	static void diffuse(Array& dst, const Array& src, T diffusion=T(0.01), int passes=10);
	
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
	size_t mDim, mDim3, mDimWrap;
	int mFront;			// which one is the front buffer?
	Array mArray0, mArray1; //mArrays[2];	// double-buffering
};

template<typename T=double>
class Fluid {
public:
	enum BoundaryMode {
		NONE = 0,
		CLAMP = 1,
		FIELD = 2
	};

	Fluid(int components, int dim) 
	:	densities(components, dim),
		velocities(3, dim),
		gradient(1, dim),
		boundaries(1, Array::type<T>(), dim, dim, dim),
		passes(10),
		viscocity(0.001),
		diffusion(0.01),
		selfadvection(0.1),
		selfdecay(0.99),
		selfbackgroundnoise(0.001),
		decay(0.98),
		mBoundaryMode(CLAMP)
	{
		// set all values to T(1):
		T one = 1;
		boundaries.set3d(&one);
	}
	~Fluid() {}
	
	template<typename T1>
	void addForce(const Vec<3,T1> pos, const Vec<3,T> vel) {
		velocities.front().write_interp(vel.elems(), pos);
	}
	
	template<typename T1>
	void addDensities(const Vec<3,T1> pos, const T * elems) {
		densities.front().write_interp(elems, pos);
	}
	
	template<typename T1>
	void readVelocity(const Vec<3,T1> pos, Vec<3,T>& vel) {
		velocities.front().read_interp(vel.elems(), pos);
	}
	
	template<typename T1>
	void readDensities(const Vec<3,T1> pos, T * elems) {
		velocities.front().read_interp(elems, pos);
	}
	
	template<typename T1>
	void readGradient(const Vec<3,T1> pos, float& vel) {
		gradient.front().read_interp(&vel, pos);
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

		// DENSITIES:
		// assume new data is in front();
		// smoothen the new data:
		densities.diffuse(diffusion, passes);
		//(diffused data now in densities.front())
		// and advect:
		densities.advect(velocities.front());
		//(advected data now in densities.front())
		// fade, etc.
		densities.scale(decay);
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
		// prepare new gradient data:
		gradient.back().zero();
		velocities.calculateGradientMagnitude(gradient.front());
		// diffuse it:
		gradient.diffuse(0.5, passes/2);
		// subtract from current velocities:
		velocities.subtractGradientMagnitude(gradient.front());
	}
	
	void boundary(BoundaryMode b) { mBoundaryMode = b; }
	
	Field3D<T> densities, velocities, gradient;
	Array boundaries;
	unsigned passes;
	T viscocity, diffusion, selfadvection, selfdecay, selfbackgroundnoise, decay;
	rnd::Random<> rng;
	BoundaryMode mBoundaryMode;
};



// INLINE IMPLEMENTATION //



template<typename T>
size_t Field3D<T>::index(int x, int y, int z) const {
	return	((x&mDimWrap) * stride(0)) +
			((y&mDimWrap) * stride(1)) +
			((z&mDimWrap) * stride(2));
}

template<typename T>
T& Field3D<T>::cell(int x, int y, int z, int k) {
	return ptr()[index(x, y, z) + k];
}

template<typename T>
void Field3D<T>::setHarmonic(T px, T py, T pz) {
	T vals[3];
	for (int z=0;z<mDim;z++) {
		vals[2] = sin(pz * M_2PI * z/T(dim()));
		for (int y=0;y<mDim;y++) {
			vals[1] = sin(py * M_2PI * y/T(dim()));
			for (int x=0;x<mDim;x++) {
				vals[0] = sin(px * M_2PI * x/T(dim()));
				T value = vals[0] * vals[1] * vals[2];
				front().write(&value, x, y, z);
			}
		}
	}
}

template<typename T>
void Field3D<T>::adduniformS(rnd::Random<>& rng, T scalar) {
	T * p = ptr();
	for (int k=0;k<length();k++) p[k] += scalar * rng.uniformS();
}
template<typename T>
void Field3D<T>::adduniform(rnd::Random<>& rng, T scalar) {
	T * p = ptr();
	for (int k=0;k<length();k++) p[k] += scalar * rng.uniform();
}

template<typename T>
inline void Field3D<T>::scale(T v) {
	T * p = ptr();
	for (int k=0;k<length();k++) p[k] *= v;
}

template<typename T>
inline void Field3D<T>::scale(const Array& arr) {
	if (arr.isFormat(front().header)) {
		const size_t stride0 = stride(0);
		const size_t stride1 = stride(1);
		const size_t stride2 = stride(2);
		#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrap)*stride0) +  (((y)&mDimWrap)*stride1) +  (((z)&mDimWrap)*stride2)))

		// zero the boundary fields
		char * optr = front().data.ptr;
		char * vptr = arr.data.ptr;
		for (int z=0;z<mDim;z++) {
			for (int y=0;y<mDim;y++) {
				for (int x=0;x<mDim;x++) {
					// cell to update:
					T * cell = INDEX(optr, x, y, z);
					T * src = INDEX(vptr, x, y, z);

					for (int k=0; k<components(); k++) {
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
		#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrap)*stride0) +  (((y)&mDimWrap)*stride1) +  (((z)&mDimWrap)*stride2)))
		
		#define AINDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrap)*astride0) +  (((y)&mDimWrap)*astride1) +  (((z)&mDimWrap)*astride2)))

		// scale the boundary fields
		char * optr = front().data.ptr;
		char * aptr = arr.data.ptr;
		for (int z=0;z<mDim;z++) {
			for (int y=0;y<mDim;y++) {
				for (int x=0;x<mDim;x++) {
					// cell to update:
					T * cell = INDEX(optr, x, y, z);
					T v = AINDEX(aptr, x, y, z)[0];
					for (int k=0; k<components(); k++) {
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
		#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrap)*stride0) +  (((y)&mDimWrap)*stride1) +  (((z)&mDimWrap)*stride2)))

		// zero the boundary fields
		char * optr = front().data.ptr;
		char * vptr = arr.data.ptr;
		for (int z=0;z<mDim;z++) {
			for (int y=0;y<mDim;y++) {
				for (int x=0;x<mDim;x++) {
					// cell to update:
					T * cell = INDEX(optr, x, y, z);
					T v = fabs(INDEX(vptr, x, y, z)[0]);

					for (int k=0; k<components(); k++) {
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
			in[i] = T(0);
		}
	} else {
		printf("Array format mismatch\n");
	}
}

template<typename T>
inline void Field3D<T> :: diffuse(T diffusion, int passes) {
	swap(); 
	diffuse(front(), back(), diffusion, passes);
}

// Gauss-Seidel relaxation scheme:
template<typename T>
inline void Field3D<T> :: diffuse(Array& out, const Array& in, T diffusion, int passes) {
	const size_t stride0 = out.header.stride[0];
	const size_t stride1 = out.header.stride[1];
	const size_t stride2 = out.header.stride[2];
	const size_t components = out.header.components;
	const size_t dim = out.header.dim[0];
	const size_t dimwrap = dim-1;
	const char * iptr = in.data.ptr;
	const char * optr = out.data.ptr;
	double div = 1.0/((1.+6.*diffusion));
	#define INDEX(p, x, y, z) ((T *)(p + ((((x)&dimwrap)*stride0) + (((y)&dimwrap)*stride1) + (((z)&dimwrap)*stride2))))
	
	for (unsigned n=0 ; n<passes ; n++) {
		for (unsigned z=0;z<dim;z++) {
			for (unsigned y=0;y<dim;y++) {
				for (unsigned x=0;x<dim;x++) {
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

template<typename T>
inline void Field3D<T> :: relax( double a, int iterations) {
	char * old = front().data.ptr;
	char * out = back().data.ptr;
	const uint32_t comps = components();
	const double c = 1./(1. + 6.*a); 
	for (int iter=0; iter<iterations; iter++) {
		for (int z=0;z<mDim;z++) {
			for (int y=0;y<mDim;y++) {
				for (int x=0;x<mDim;x++) {
					const size_t idx = index(x, y, z);
					const size_t x0 = index(x-1, y, z);
					const size_t x1 = index(x+1, y, z);
					const size_t y0 = index(x, y-1, z);
					const size_t y1 = index(x, y+1, z);
					const size_t z0 = index(x, y, z-1);
					const size_t z1 = index(x, y, z+1);
					for (unsigned k=0; k<comps; k++) {
						out[idx+k] = c * (
										old[idx+k] +
										a * (	
											out[x0+k] + out[x1+k] +
											out[y0+k] + out[y1+k] +
											out[z0+k] + out[z1+k]
										)
									);
					}
				}
			}
		}
		// todo: apply boundary here
	}
	#undef CELL
}

template<typename T>
inline void Field3D<T> :: advect(Array& dst, const Array& src, const Array& velocities, T rate) {
	const size_t stride0 = src.stride(0);
	const size_t stride1 = src.stride(1);
	const size_t stride2 = src.stride(2);
	const size_t dim = src.dim(0);
	const size_t dimwrap = dim-1;
	
	if (velocities.header.type != src.header.type ||
		velocities.header.components < 3 ||
		velocities.header.dim[0] != dim ||
		velocities.header.dim[1] != dim ||
		velocities.header.dim[2] != dim) 
	{
		printf("Array format mismatch\n");
		return;
	}
	char * outptr = dst.data.ptr;
	char * velptr = velocities.data.ptr;

	#define CELL(p, x, y, z, k) (((T *)((p) + (((x)&dimwrap)*stride0) +  (((y)&dimwrap)*stride1) +  (((z)&dimwrap)*stride2)))[(k)])

	for (int z=0;z<dim;z++) {
		for (int y=0;y<dim;y++) {
			for (int x=0;x<dim;x++) {
				// back trace: (current cell offset by vector at cell)
				T * bp  = &(CELL(outptr, x, y, z, 0));
				T * vp	= &(CELL(velptr, x, y, z, 0));
				T vx = x - rate * vp[0];
				T vy = y - rate * vp[1];
				T vz = z - rate * vp[2];

				// read interpolated input field value into back-traced location:
				src.read_interp(bp, vx, vy, vz);
			}
		}
	}
	#undef CELL
}

template<typename T>
inline void Field3D<T> :: advect(const Array& velocities, T rate) {
	swap();
	advect(front(), back(), velocities, rate);
}

template<typename T>
inline void Field3D<T> :: calculateGradientMagnitude(Array& gradient) {
	gradient.format(1, Array::type<T>(), mDim, mDim, mDim);
	const size_t stride0 = stride(0);
	const size_t stride1 = stride(1);
	const size_t stride2 = stride(2);
	const size_t gstride0 = gradient.stride(0);
	const size_t gstride1 = gradient.stride(1);
	const size_t gstride2 = gradient.stride(2);
	
	#define CELL(p, x, y, z, k) (((T *)((p) + (((x)&mDimWrap)*stride0) +  (((y)&mDimWrap)*stride1) +  (((z)&mDimWrap)*stride2)))[(k)])
	#define CELLG(p, x, y, z) (((T *)((p) + (((x)&mDimWrap)*gstride0) +  (((y)&mDimWrap)*gstride1) +  (((z)&mDimWrap)*gstride2)))[0])
	
	// calculate gradient.
	// previous instantaneous magnitude of velocity gradient
	//		= average of velocity gradients per axis:
	const double h = -0.5/mDim; //1./3.; //0.5/mDim;
	char * iptr = front().data.ptr;
	char * gptr = gradient.data.ptr;
	
	for (int z=0;z<mDim;z++) {
		for (int y=0;y<mDim;y++) {
			for (int x=0;x<mDim;x++) {
				// gradients per axis:
				const T xgrad = CELL(iptr, x+1,y,	z,	0) - CELL(iptr, x-1,y,	z,	0);
				const T ygrad = CELL(iptr, x,	y+1,z,	1) - CELL(iptr, x,	y-1,z,	1);
				const T zgrad = CELL(iptr, x,	y,	z+1,2) - CELL(iptr, x,	y,	z-1,2);
				// gradient at current cell:
				const T grad = h * (xgrad+ygrad+zgrad);
				// store in a 1-plane field
				CELLG(gptr, x, y, z) = grad;
			}
		}
	}
	#undef CELL
	#undef CELLG
}



template<typename T>
inline void Field3D<T> :: subtractGradientMagnitude(const Array& gradient) {
	if (gradient.header.type != Array::type<T>() ||
		gradient.header.dim[0] != mDim ||
		gradient.header.dim[1] != mDim ||
		gradient.header.dim[2] != mDim) 
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
	
	#define INDEX(p, x, y, z) ((T *)(p + ((((x)&mDimWrap)*stride0) + (((y)&mDimWrap)*stride1) + (((z)&mDimWrap)*stride2))))
	#define CELLG(p, x, y, z) (((T *)((p) + (((x)&mDimWrap)*gstride0) +  (((y)&mDimWrap)*gstride1) +  (((z)&mDimWrap)*gstride2)))[0])
	
	// now subtract gradient from current field:
	char * gptr = gradient.data.ptr;
	char * optr = front().data.ptr;
	//const double h = 1.; ///3.;
	const double h = mDim * 0.5;
	for (int z=0;z<mDim;z++) {
		for (int y=0;y<mDim;y++) {
			for (int x=0;x<mDim;x++) {
				// cell to update:
				T * vel = INDEX(optr, x, y, z);
				// gradients per axis:
				vel[0] -= h * ( CELLG(gptr, x+1,y,	z  ) - CELLG(gptr, x-1,y,	z  ) );
				vel[1] -= h * ( CELLG(gptr, x,	y+1,z  ) - CELLG(gptr, x,	y-1,z  ) );
				vel[2] -= h * ( CELLG(gptr, x,	y,	z+1) - CELLG(gptr, x,	y,	z-1) );
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

	#define CELL(p, x, y, z, k) (((T *)((p) + (((x)&mDimWrap)*stride0) +  (((y)&mDimWrap)*stride1) +  (((z)&mDimWrap)*stride2)))[(k)])
	
	// x planes
	for (int z=0;z<mDim;z++) {
		for (int y=0;y<mDim;y++) {
			CELL(optr, 0, y, z, 0) = T(0);
			CELL(optr, mDimWrap, y, z, 0) = T(0); 
		}
	}
	// y planes
	for (int z=0;z<mDim;z++) {
		for (int x=0;x<mDim;x++) {
			CELL(optr, x, 0, z, 1) = T(0);
			CELL(optr, x, mDimWrap, z, 1) = T(0); 
		}
	}
	// z planes
	for (int y=0;y<mDim;y++) {
		for (int x=0;x<mDim;x++) {
			CELL(optr, x, y, 0, 2) = T(0);
			CELL(optr, x, y, mDimWrap, 2) = T(0); 
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
	#define INDEX(p, x, y, z) ((T *)((p) + (((x)&mDimWrap)*stride0) +  (((y)&mDimWrap)*stride1) +  (((z)&mDimWrap)*stride2)))

	// zero the boundary fields
	char * optr = front().data.ptr;
	for (int z=0;z<dim2;z++) {
		for (int y=0;y<dim1;y++) {
			for (int x=0;x<dim0;x++) {
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