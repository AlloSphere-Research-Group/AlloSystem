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
	C++ wrapper and utilities for AlloArray

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#ifndef INCLUDE_ALLO_ARRAY_HPP
#define INCLUDE_ALLO_ARRAY_HPP 1

#include <cstdio> // FILE
#include "allocore/types/al_Array.h"
#include "allocore/math/al_Vec.hpp"

#ifndef AL_ARRAY_DEFAULT_ALIGNMENT
#define AL_ARRAY_DEFAULT_ALIGNMENT (4)
#endif

namespace al {


/// Object-oriented interface to AlloArray
///
/// @ingroup allocore
class Array : public AlloArray {
public:

	/// Empty constructor defines a 0-dimensional, 1-component array of void type; unallocated data
	Array();

	/// Construct 1-dimensional array
	Array(int components, AlloTy ty, uint32_t dimx);

	/// Construct 2-dimensional array
	Array(int components, AlloTy ty, uint32_t dimx, uint32_t dimy);

	/// Construct 3-dimensional array
	Array(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz);

	///	Copy constructor; copies both the layout and data from cpy
	explicit Array(const AlloArray& cpy);
	explicit Array(const AlloArrayHeader& h2);

	~Array();


	/// Assignment operator copies format and data (allocates memory if necessary)
	Array& operator= (const AlloArray& cpy);


	/// Get type of elements
	AlloTy type() const { return header.type; }

	/// Verify elements are a particular type
	bool isType(AlloTy ty) const { return header.type == ty; }
	template<typename T> bool isType() const { return isType(type<T>()); }

	uint8_t components() const { return header.components; }	///< Get number of components
	uint8_t dimcount() const { return header.dimcount; }		///< Get number of dimensions
	uint32_t dim(int i=0) const { return header.dim[i]; }		///< Get size of dimension
	unsigned width() const { return dim(0); }					///< Get size of first dimension
	unsigned height() const { return dim(1); }					///< Get size of second dimension
	unsigned depth() const { return dim(2); }					///< Get size of third dimension
	uint32_t stride(int i=0) const { return header.stride[i]; }	///< Get stride of dimension, in bytes

	/// Returns the maximum possible byte alignment of the rows (1, 2, 4 or 8 byte)
	uint32_t alignment() const;

	/// Returns the total memory footprint, in bytes
	size_t size() const { return allo_array_size(this); }

	/// Returns number of cells in the Array:
	unsigned cells() const { return allo_array_elements(this); }

	/// Change the format without de/re/allocating:
	void configure(const AlloArrayHeader& h2);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(const AlloArrayHeader& h2);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(const AlloArray& array) { format(array.header); }

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(int components, AlloTy ty, uint32_t dimx);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(int components, AlloTy ty, uint32_t dimx, uint32_t dimy);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void formatAligned(int components, AlloTy ty, uint32_t dimx, size_t align);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void formatAligned(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, size_t align);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void formatAligned(int components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, size_t align);

	/// Check if this Array conforms to an ArrayHeader format
	bool isFormat(const AlloArrayHeader& h2) const;
	bool isFormat(const Array& src) const { return isFormat(src.header); }

	/// Returns true if Array contains data, false otherwise
	bool hasData() const { return NULL != data.ptr; }

	/// Allocate memory for the given header.

	/// Warning: does not check if memory was already allocated;
	/// Call dataFree() first if you know it will be safe to do so.
	void dataCalloc();

	/// Free memory and set data.ptr to NULL
	void dataFree();

	/// Set all data to zero
	void zero();


	/// Get mutable component using 1-D index
	template <class T> T& elem(size_t ic, size_t ix){
		return cell<T>(ix)[ic]; }

	/// Get mutable component using 2-D index
	template <class T> T& elem(size_t ic, size_t ix, size_t iy){
		return cell<T>(ix,iy)[ic]; }

	/// Get mutable component using 3-D index
	template <class T> T& elem(size_t ic, size_t ix, size_t iy, size_t iz){
		return cell<T>(ix,iy,iz)[ic]; }

	/// Get const component using 1-D index
	template <class T> const T& elem(size_t ic, size_t ix) const{
		return cell<T>(ix)[ic]; }

	/// Get const component using 2-D index
	template <class T> const T& elem(size_t ic, size_t ix, size_t iy) const{
		return cell<T>(ix,iy)[ic]; }

	/// Get const component using 3-D index
	template <class T> const T& elem(size_t ic, size_t ix, size_t iy, size_t iz) const{
		return cell<T>(ix,iy,iz)[ic]; }


	/// Fill with the same cell value throughout
	template<class T> void set1d(T * cell);
	template<class T> void set2d(T * cell);
	template<class T> void set3d(T * cell);

	template<class T> void setall(T value);

	/// Use a pure C function to fill an array with data
	template<class T> void fill(void (*func)(T * values, double normx));
	template<class T> void fill(void (*func)(T * values, double normx, double normy));
	template<class T> void fill(void (*func)(T * values, double normx, double normy, double normz));

	/// Get the components at a given index in the array (no bounds checking)
	template<class T> T * cell(size_t x) const;
	template<class T> T * cell(size_t x, size_t y) const;
	template<class T> T * cell(size_t x, size_t y, size_t z) const;

	template<class T, class U> T * cell(T* val, Vec<2,U> p) const { return cell(val, p[0], p[1]); }
	template<class T, class U> T * cell(T* val, Vec<3,U> p) const { return cell(val, p[0], p[1], p[2]); }

	/// Return a particular cell casted to a typed reference (no bounds checking)
	template<class T> T& as(int x) { return *cell<T>(x); }
	template<class T> const T& as(int x) const { return *cell<T>(x); }
	template<class T> T& as(int x, int y) { return *cell<T>(x,y); }
	template<class T> const T& as(int x, int y) const { return *cell<T>(x,y); }
	template<class T> T& as(int x, int y, int z) { return *cell<T>(x,y,z); }
	template<class T> const T& as(int x, int y, int z) const { return *cell<T>(x,y,z); }
	/// Return casted cell indexed by a vector (no bounds checking)
	template<class T, class U> T& as(Vec<2,U> p) { return *cell<T>(p.x,p.y); }
	template<class T, class U> const T& as(Vec<2,U> p) const { return *cell<T>(p.x,p.y); }
	template<class T, class U> T& as(Vec<3,U> p) { return *cell<T>(p.x,p.y,p.z); }
	template<class T, class U> const T& as(Vec<3,U> p) const { return *cell<T>(p.x,p.y,p.z); }

	/// Read the component values from array into val array (no bounds checking)
	template<class T> void read(T* val, int x) const;
	template<class T> void read(T* val, int x, int y) const;
	template<class T> void read(T* val, int x, int y, int z) const;

	template<class T, class U> void read(T* val, Vec<2,U> p) const { read(val, p[0], p[1]); }
	template<class T, class U> void read(T* val, Vec<3,U> p) const { read(val, p[0], p[1], p[2]); }

	/// Read the component values from array into val array (wraps periodically at bounds)
	template<class T> void read_wrap(T* val, int x) const;
	template<class T> void read_wrap(T* val, int x, int y) const;
	template<class T> void read_wrap(T* val, int x, int y, int z) const;

	template<class T, class U> void read_wrap(T* val, Vec<2,U> p) const { read_wrap(val, p[0], p[1]); }
	template<class T, class U> void read_wrap(T* val, Vec<3,U> p) const { read_wrap(val, p[0], p[1], p[2]); }

	/// Linear interpolated lookup (virtual array index)

	/// Reads the linearly interpolated component values into val array
	///
	template<class T> void read_interp(T * val, double x) const;
	template<class T> void read_interp(T * val, double x, double y) const;
	template<class T> void read_interp(T * val, double x, double y, double z) const;

	template<class T, class U> void read_interp(T* val, Vec<2,U> p) const { read_interp(val, p[0], p[1]); }
	template<class T, class U> void read_interp(T* val, Vec<3,U> p) const { read_interp(val, p[0], p[1], p[2]); }

	/// Write component values from val array into array (no bounds checking)
	template<class T> void write(const T* val, int x);
	template<class T> void write(const T* val, int x, int y);
	template<class T> void write(const T* val, int x, int y, int z);

	template<class T, class U> void write(const T* val, Vec<2,U> p) { write(val, p[0], p[1]); }
	template<class T, class U> void write(const T* val, Vec<3,U> p) { write(val, p[0], p[1], p[2]); }

	/// Write plane values from val array into array (wraps periodically at bounds)
	template<class T> void write_wrap(const T* val, int x);
	template<class T> void write_wrap(const T* val, int x, int y);
	template<class T> void write_wrap(const T* val, int x, int y, int z);

	template<class T, class U> void write_wrap(const T* val, Vec<2,U> p) { write_wrap(val, p[0], p[1]); }
	template<class T, class U> void write_wrap(const T* val, Vec<3,U> p) { write_wrap(val, p[0], p[1], p[2]); }

	/// Linear interpolated write (virtual array index)

	/// AKA trilinear splat
	/// writes the linearly interpolated plane values from val array into array
	template<class T> void write_interp(const T* val, double x);
	template<class T> void write_interp(const T* val, double x, double y);
	template<class T> void write_interp(const T* val, double x, double y, double z);

	template<class T, class U> void write_interp(const T* val, Vec<2,U> p) { write_interp(val, p[0], p[1]); }
	template<class T, class U> void write_interp(const T* val, Vec<3,U> p) { write_interp(val, p[0], p[1], p[2]); }

	/// Print array information
	void print(FILE * fp = stdout) const;


	///	Returns the type enumeration ID (AlloTy) for a given type (given as template argument).

	/// E.g., assert(Array::type<float>() == AlloFloat32Ty);
	///
	template<class T> static AlloTy type();

	///	Derive the appropriate stride values for a given row alignment
	static void deriveStride(AlloArrayHeader& h, size_t rowAlignSize);

protected:
	void formatAlignedGeneral(int comps, AlloTy ty, uint32_t * dims, int numDims, size_t align);
public:	// temporarily made public, because protected broke some other project code -gw
	Array(const Array&);
	Array& operator= (const Array&);
protected:

	// temporary hack because the one in al_Function gave a bad result
	// for e.g. wrap<double>(-64.0, -32.0);
	template<class T>
	static T wrap(T v, const T hi=T(1.), const T lo=T(0.)){
		if(lo == hi) return lo;
		//if(v >= hi){
		if(!(v < hi)){
			T diff = hi - lo;
			v -= diff;
			if(!(v < hi)) v -= diff * (T)(uint32_t)((v - lo)/diff);
		}
		else if(v < lo){
			T diff = hi - lo;
			v += diff;
			if(v < lo) v += diff * (T)(uint32_t)(((lo - v)/diff) + 1);
			if(v==diff) return lo;
		}
		return v;
	}
};


// ********* INLINE IMPLEMENTATION BELOW ***********

// Type traits by partial specialization:
template<> constexpr AlloTy Array::type<uint8_t  >(){ return AlloUInt8Ty; }
template<> constexpr AlloTy Array::type<uint16_t >(){ return AlloUInt16Ty; }
template<> constexpr AlloTy Array::type<uint32_t >(){ return AlloUInt32Ty; }
template<> constexpr AlloTy Array::type<uint64_t >(){ return AlloUInt64Ty; }
template<> constexpr AlloTy Array::type<int8_t   >(){ return AlloSInt8Ty; }
template<> constexpr AlloTy Array::type<int16_t  >(){ return AlloSInt16Ty; }
template<> constexpr AlloTy Array::type<int32_t  >(){ return AlloSInt32Ty; }
template<> constexpr AlloTy Array::type<int64_t  >(){ return AlloSInt64Ty; }
template<> constexpr AlloTy Array::type<float    >(){ return AlloFloat32Ty; }
template<> constexpr AlloTy Array::type<double   >(){ return AlloFloat64Ty; }
template<> constexpr AlloTy Array::type<AlloArray>(){ return AlloArrayTy; }
template<> constexpr AlloTy Array::type<void *>(){
	switch(sizeof(void *)) {
		case 4: return AlloPointer32Ty;
		case 8: return AlloPointer64Ty;
	}
	return 0;
}

template<class T> inline T * Array::cell(size_t x) const {
	size_t fieldstride_x = header.stride[0];
	return (T *)(data.ptr + x*fieldstride_x);
}
template<class T> inline T * Array::cell(size_t x, size_t y) const {
	size_t fieldstride_x = header.stride[0];
	size_t fieldstride_y = header.stride[1];
	return (T *)(data.ptr + x*fieldstride_x + y*fieldstride_y);
}
template<class T> inline T * Array::cell(size_t x, size_t y, size_t z) const {
	size_t fieldstride_x = header.stride[0];
	size_t fieldstride_y = header.stride[1];
	size_t fieldstride_z = header.stride[2];
	return (T *)(data.ptr + x*fieldstride_x + y*fieldstride_y + z*fieldstride_z);
}


// read the plane values from array into val array (no bounds checking)
template<class T> inline void Array::read(T* val, int x) const {
	T * paaa = cell<T>(x);
	for (uint8_t p=0; p<header.components; p++) {
		val[p] = paaa[p];
	}
}
template<class T> inline void Array::read(T* val, int x, int y) const {
	T * paaa = cell<T>(x, y);
	for (uint8_t p=0; p<header.components; p++) {
		val[p] = paaa[p];
	}
}
template<class T> inline void Array::read(T* val, int x, int y, int z) const {
	T * paaa = cell<T>(x, y, z);
	for (uint8_t p=0; p<header.components; p++) {
		val[p] = paaa[p];
	}
}

// read the plane values from array into val array (wraps periodically at bounds)
template<class T> inline void Array::read_wrap(T* val, int x) const {
	read(val, wrap<int>(x, header.dim[0], 0));
}
template<class T> inline void Array::read_wrap(T* val, int x, int y) const {
	read(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0));
}
template<class T> inline void Array::read_wrap(T* val, int x, int y, int z) const {
	read(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0), wrap<int>(z, header.dim[2], 0));
}

#define AL_ARRAY_FLOOR(v) ( (long)(v) - ((v)<0. && (v)!=(long)(v)) )
#define AL_ARRAY_FRAC(v) ( ((v)>=0.) ? (v)-(long)(v) : (-v)-(long)(v) )

// linear interpolated lookup (virtual array index)
// reads the linearly interpolated plane values into val array
template<class T> inline void Array::read_interp(T * val, double x) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	const unsigned xa = (const unsigned)AL_ARRAY_FLOOR(x);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double faaa = AL_ARRAY_FRAC(x);
	double fbaa = 1.f - faaa;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa);
	T * pbaa = cell<T>(xb);
	// for each plane of the field, do the interp:
	for (uint8_t p=0; p<header.components; p++) {
		val[p] =	(paaa[p] * faaa) + (pbaa[p] * fbaa);
	}
}

template<class T> inline void Array::read_interp(T * val, double x, double y) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	const unsigned xa = (const unsigned)AL_ARRAY_FLOOR(x);
	const unsigned ya = (const unsigned)AL_ARRAY_FLOOR(y);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1.f - ybf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf;
	double faba = xaf * ybf;
	double fbaa = xbf * yaf;
	double fbba = xbf * ybf;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa, ya);
	T * paba = cell<T>(xa, yb);
	T * pbaa = cell<T>(xb, ya);
	T * pbba = cell<T>(xb, yb);
	// for each plane of the field, do the interp:
	for (uint8_t p=0; p<header.components; p++) {
		val[p] =	(paaa[p] * faaa) +
		(pbaa[p] * fbaa) +
		(paba[p] * faba) +
		(pbba[p] * fbba);
	}
}

template<class T> inline void Array::read_interp(T * val, double x, double y, double z) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	z = wrap<double>(z, (double)header.dim[2], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	const unsigned xa = (const unsigned)AL_ARRAY_FLOOR(x);
	const unsigned ya = (const unsigned)AL_ARRAY_FLOOR(y);
	const unsigned za = (const unsigned)AL_ARRAY_FLOOR(z);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	unsigned zb = za+1;	if (zb == header.dim[2]) zb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1.f - ybf;
	double zbf = AL_ARRAY_FRAC(z);
	double zaf = 1.f - zbf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf * zaf;
	double faab = xaf * yaf * zbf;
	double faba = xaf * ybf * zaf;
	double fabb = xaf * ybf * zbf;
	double fbaa = xbf * yaf * zaf;
	double fbab = xbf * yaf * zbf;
	double fbba = xbf * ybf * zaf;
	double fbbb = xbf * ybf * zbf;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa, ya, za);
	T * paab = cell<T>(xa, ya, zb);
	T * paba = cell<T>(xa, yb, za);
	T * pabb = cell<T>(xa, yb, zb);
	T * pbaa = cell<T>(xb, ya, za);
	T * pbab = cell<T>(xb, ya, zb);
	T * pbba = cell<T>(xb, yb, za);
	T * pbbb = cell<T>(xb, yb, zb);
	// for each plane of the field, do the 3D interp:
	for (size_t p=0; p<header.components; p++) {
		val[p] =	(paaa[p] * faaa) +
					(pbaa[p] * fbaa) +
					(paba[p] * faba) +
					(paab[p] * faab) +
					(pbab[p] * fbab) +
					(pabb[p] * fabb) +
					(pbba[p] * fbba) +
					(pbbb[p] * fbbb);
	}
}

// write plane values from val array into array (no bounds checking)
template<class T> inline void Array::write(const T* val, int x) {
	T * paaa = cell<T>(x);
	for (uint8_t p=0; p<header.components; ++p) {
		paaa[p] = val[p];
	}
}
template<class T> inline void Array::write(const T* val, int x, int y) {
	T * paaa = cell<T>(x, y);
	for (uint8_t p=0; p<header.components; ++p) {
		paaa[p] = val[p];
	}
}
template<class T> inline void Array::write(const T* val, int x, int y, int z) {
	T * paaa = cell<T>(x, y, z);
	for (uint8_t p=0; p<header.components; ++p) {
		paaa[p] = val[p];
	}
}

// write plane values from val array into array (wraps periodically at bounds)
template<class T> inline void Array::write_wrap(const T* val, int x) {
	write(val, wrap<int>(x, header.dim[0], 0));
}
template<class T> inline void Array::write_wrap(const T* val, int x, int y) {
	write(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0));
}
template<class T> inline void Array::write_wrap(const T* val, int x, int y, int z) {
	write(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0), wrap<int>(z, header.dim[2], 0));
}

// linear interpolated write (virtual array index)
// writes the linearly interpolated plane values from val array into array
template<class T> inline void Array::write_interp(const T* val, double x) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	const unsigned xa = (const unsigned)AL_ARRAY_FLOOR(x);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1.f - xbf;
	// get the interpolation corner weights:
	double faaa = xaf;
	double fbaa = xbf;
	T * paaa = cell<T>(xa);
	T * pbaa = cell<T>(xb);
	// for each plane of the field, do the 3D interp:
	for (uint8_t p=0; p<header.components; p++) {
		T tmp = val[p];
		paaa[p] += tmp * faaa;
		pbaa[p] += tmp * fbaa;
	}
}
template<class T> inline void Array::write_interp(const T* val, double x, double y) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	const unsigned xa = (const unsigned)AL_ARRAY_FLOOR(x);
	const unsigned ya = (const unsigned)AL_ARRAY_FLOOR(y);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1.f - ybf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf;
	double faba = xaf * ybf;
	double fbaa = xbf * yaf;
	double fbba = xbf * ybf;
	T * paaa = cell<T>(xa, ya);
	T * paba = cell<T>(xa, yb);
	T * pbaa = cell<T>(xb, ya);
	T * pbba = cell<T>(xb, yb);
	// for each plane of the field, do the 3D interp:
	for (uint8_t p=0; p<header.components; p++) {
		T tmp = val[p];
		paaa[p] += tmp * faaa;
		paba[p] += tmp * faba;
		pbaa[p] += tmp * fbaa;
		pbba[p] += tmp * fbba;
	}
}

template<class T> inline void Array::write_interp(const T* val, double x0, double y0, double z0) {
	double x = wrap<double>(x0, (double)header.dim[0], 0.);
	double y = wrap<double>(y0, (double)header.dim[1], 0.);
	double z = wrap<double>(z0, (double)header.dim[2], 0.);
	const unsigned xa = (const unsigned)AL_ARRAY_FLOOR(x);
	const unsigned ya = (const unsigned)AL_ARRAY_FLOOR(y);
	const unsigned za = (const unsigned)AL_ARRAY_FLOOR(z);
	unsigned xb = xa+1;	if (xb == header.dim[0]) xb = 0;
	unsigned yb = ya+1;	if (yb == header.dim[1]) yb = 0;
	unsigned zb = za+1;	if (zb == header.dim[2]) zb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1.f - ybf;
	double zbf = AL_ARRAY_FRAC(z);
	double zaf = 1.f - zbf;
	// get the interpolation corner weights:
	double faaa = xaf * yaf * zaf;
	double faab = xaf * yaf * zbf;
	double faba = xaf * ybf * zaf;
	double fabb = xaf * ybf * zbf;
	double fbaa = xbf * yaf * zaf;
	double fbab = xbf * yaf * zbf;
	double fbba = xbf * ybf * zaf;
	double fbbb = xbf * ybf * zbf;
	T * paaa = cell<T>(xa, ya, za);
	T * paab = cell<T>(xa, ya, zb);
	T * paba = cell<T>(xa, yb, za);
	T * pabb = cell<T>(xa, yb, zb);
	T * pbaa = cell<T>(xb, ya, za);
	T * pbab = cell<T>(xb, ya, zb);
	T * pbba = cell<T>(xb, yb, za);
	T * pbbb = cell<T>(xb, yb, zb);
	// for each plane of the field, do the 3D interp:
	for (uint8_t p=0; p<header.components; p++) {
		T tmp = val[p];
		paaa[p] += tmp * faaa;
		paab[p] += tmp * faab;
		paba[p] += tmp * faba;
		pabb[p] += tmp * fabb;
		pbaa[p] += tmp * fbaa;
		pbab[p] += tmp * fbab;
		pbba[p] += tmp * fbba;
		pbbb[p] += tmp * fbbb;
	}
}

#undef AL_ARRAY_FLOOR
#undef AL_ARRAY_FRAC

template<class T> void Array::fill(void (*func)(T * values, double normx)) {
	int d0 = header.dim[0];
	double inv_d0 = 1.0/(double)d0;
	int components = header.components;

	T *vals = (T *)(data.ptr);
	for(int x=0; x < d0; x++) {
		func(vals, inv_d0 * x);
		vals += components;
	}
}

template<class T> void Array::fill(void (*func)(T * values, double normx, double normy)) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int s1 = header.stride[1];
	double inv_d0 = 1.0/(double)d0;
	double inv_d1 = 1.0/(double)d1;
	int components = header.components;

	for(int y=0; y < d1; y++) {
		T *vals = (T *)(data.ptr + s1*y);
		for(int x=0; x < d0; x++) {
			func(vals, inv_d0 * x, inv_d1 * y);
			vals += components;
		}
	}
}

template<class T> void Array::fill(void (*func)(T * values, double normx, double normy, double normz)) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int d2 = header.dim[2];
	int s1 = header.stride[1];
	int s2 = header.stride[2];
	double inv_d0 = 1.0/(double)d0;
	double inv_d1 = 1.0/(double)d1;
	double inv_d2 = 1.0/(double)d2;
	int components = header.components;

	for(int z=0; z < d1; z++) {
		for(int y=0; y < d1; y++) {
			T *vals = (T *)(data.ptr + s1*y + s2*z);
			for(int x=0; x < d0; x++) {
				func(vals, inv_d0 * x, inv_d1 * y, inv_d2 * z);
				vals += components;
			}
		}
	}
}

template<class T> void Array::setall(T value) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	//int d2 = header.dim[2];
	int s0 = header.stride[0];
	int s1 = header.stride[1];
	int s2 = header.stride[2];
	int components = header.components;
	T * vals;
	switch (header.dimcount) {
		case 3:
			for(int z=0; z < d1; z++) {
				for(int y=0; y < d1; y++) {
					vals = (T *)(data.ptr + s1*y + s2*z);
					for(int x=0; x < d0; x++) {
						for (int i=0; i<components; i++) {
							vals[i] = value;
						}
					}
				}
			}
			break;
		case 2:
			for(int y=0; y < d1; y++) {
				for(int x=0; x < d0; x++) {
					vals = (T *)(data.ptr + s0*x + s1*y);
					for (int i=0; i<components; i++) {
						vals[i] = value;
					}
				}
			}
			break;
		case 1:
			vals = (T *)(data.ptr);
			for(int x=0; x < d0; x++) {
				for (int i=0; i<components; i++) {
					vals[i] = value;
				}
			}
			break;
		default:
			break;
	}

}

template<class T> void Array::set1d(T * cell) {
	int d0 = header.dim[0];
	int s0 = header.stride[0];
	int components = header.components;

	for(int x=0; x < d0; x++) {
		T *vals = (T *)(data.ptr + s0*x);
		for (int i=0; i<components; i++) {
			vals[i] = cell[i];
		}
	}
}

template<class T> void Array::set2d(T * cell) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int s0 = header.stride[0];
	int s1 = header.stride[1];
	int components = header.components;

	for(int y=0; y < d1; y++) {
		for(int x=0; x < d0; x++) {
			T *vals = (T *)(data.ptr + s0*x + s1*y);
			for (int i=0; i<components; i++) {
				vals[i] = cell[i];
			}
		}
	}
}

template<class T> void Array::set3d(T * cell) {
	int d0 = header.dim[0];
	int d1 = header.dim[1];
	int d2 = header.dim[2];
	int s0 = header.stride[0];
	int s1 = header.stride[1];
	int s2 = header.stride[2];
	int components = header.components;

	for(int z=0; z < d2; z++) {
		for(int y=0; y < d1; y++) {
			for(int x=0; x < d0; x++) {
				T *vals = (T *)(data.ptr + s0*x + s1*y + s2*z);
				for (int i=0; i<components; i++) {
					vals[i] = cell[i];
				}
			}
		}
	}
}

} // al::

#endif // include guard
