/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */
 
/*
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
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#ifndef INCLUDE_ALLO_TYPES_CPP_H
#define INCLUDE_ALLO_TYPES_CPP_H 1

#include "allocore/types/al_Array.h"
#include "allocore/math/al_Functions.hpp"	
#include <stdlib.h>

#define AL_ARRAY_DEFAULT_ALIGNMENT (4)

#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )

namespace al {
	
	
/*
	A friendlier interface to AlloArray
		N.B. Derived type of methods only... no instance member data or vtable!
*/
class Array : public AlloArray {
public:
	
	/*!
	 Returns the type enumeration ID (AlloTy) for a given type (given as template argument)
	 e.g. assert(Array::type<float>() == AlloFloat32Ty);
	 */
	template<typename T> inline static AlloTy type();
	
	/*!
		Define an AlloArrayHeader layout:
	 */
	static void define(AlloArrayHeader& h, int components, AlloTy ty, int dimx);
	static void define(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy);
	static void define(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy, int dimz);
	static void defineAligned(AlloArrayHeader& h, int components, AlloTy ty, int dimx, size_t align);
	static void defineAligned(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy, size_t align);
	static void defineAligned(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy, int dimz, size_t align);
	static void deriveStride(AlloArrayHeader& h, size_t alignSize);
	
	/*!
		Empty constructor defines a 0-D, 1-plane array of void type; unallocated data
	 */
	Array() : AlloArray() {
		data.ptr = 0;
		header.type= 0;
		header.components = 1;
		header.dimcount = 0;
	}
	
	/*! 
		Copy constructor
	*/
	Array(const Array& cpy) {
		adapt(cpy);
		if (cpy.hasData()) {
			memcpy(data.ptr, cpy.data.ptr, size());
		}
	}
	
	/*
		Standard constructors; will allocate memory accordingly
	 */
	Array(int components, AlloTy ty, int dimx) {
		create(components, ty, dimx);
	}
	
	Array(int components, AlloTy ty, int dimx, int dimy) {
		create(components, ty, dimx, dimy);
	}
	
	Array(int components, AlloTy ty, int dimx, int dimy, int dimz) {
		create(components, ty, dimx, dimy, dimz);
	}
	
	///! verify a type:
	inline bool isType(AlloTy ty) const { return header.type == ty; }
	template<typename T> inline bool isType() { return isType(type<T>()); }
	
	///! verify that Array contains data
	bool hasData() const { return data.ptr != NULL; }

	///! Return the memory footprint of a array
	size_t size() const { return allo_array_size(this); }
	
	///! Allocate memory for the given header 
	/// (warning, does not check if data.ptr was not NULL!)
	void dataCalloc() {
		data.ptr = (char *)calloc(1, size());
	}
	
	void dataFree() {
		if(hasData()) free(data.ptr);
	}
	
	///! Check if this Array conforms to an ArrayHeader format
	bool equal(const AlloArrayHeader &h2) const {
		bool equiv =	header.components == h2.components && 
						header.type == h2.type && 
						header.dimcount == h2.dimcount;
						
		for(int i=0; i < header.dimcount; i++) {
			equiv &= header.dim[i] == h2.dim[i];
			equiv &= header.stride[i] == h2.stride[i];
		}

		return equiv;
	}
	
	void create(int components, AlloTy type, int w) {
		define(header, components, type, w);
		dataCalloc();
	}
	
	void create(int components, AlloTy type, int w, int h) {
		define(header, components, type, w, h);
		dataCalloc();
	}
	
	void create(int components, AlloTy type, int w, int h, int d) {
		define(header, components, type, w, h, d);
		dataCalloc();
	}
	
	void create(AlloArrayHeader &h) {
		define(h);
		dataCalloc();
	}

	void adapt1d(int components, AlloTy type, int w, size_t align = 4) {
		AlloArrayHeader h;
		h.type = type;
		h.components = components;
		h.dimcount = 1;
		h.dim[0] = w;
		allo_array_setstride(&h, align);
		adapt(h);
	}
	
	void adapt2d(int components, AlloTy type, int w, int h, size_t align = 4) {
		AlloArrayHeader hh;
		hh.type = type;
		hh.components = components;
		hh.dimcount = 2;
		hh.dim[0] = w;
		hh.dim[1] = h;
		allo_array_setstride(&hh, align);
		adapt(hh);
	}
	
	void adapt3d(int components, AlloTy type, int w, int h, int d, size_t align = 4) {
		AlloArrayHeader hh;
		hh.type = type;
		hh.components = components;
		hh.dimcount = 3;
		hh.dim[0] = w;
		hh.dim[1] = h;
		hh.dim[2] = d;
		allo_array_setstride(&hh, align);
		adapt(hh);
	}
	
	void adapt(const AlloArrayHeader &h) {
		if(! equal(h)) {
			dataFree();
			define(h);
			dataCalloc();
		}
	}
	
	void adapt(const AlloArray& array) {
		if(! equal(array.header)) {
			dataFree();
			define(array.header);
			dataCalloc();
		}
	}
	
	void define(const AlloArrayHeader &h2) {
		header.components = h2.components;
		header.type = h2.type;
		header.dimcount = h2.dimcount;
		for(int i=0; i < header.dimcount; i++) {
			header.dim[i] = h2.dim[i];
			header.stride[i] = h2.stride[i];
		}
	}
	
	/*
		Use a function to fill a array with data:
	*/
	template<typename T> void fill1d(void (*func)(T * values, double normx)) {
		int d0 = header.dim[0];
		double inv_d0 = 1.0/(double)d0;
		int components = header.components;
			
		T *vals = (T *)(data.ptr);
		for(int x=0; x < d0; x++) {
			func(vals, inv_d0 * x);
			vals += components;
		}
	}
	
	template<typename T> void fill2d(void (*func)(T * values, double normx, double normy)) {
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
	
	template<typename T> void fill3d(void (*func)(T * values, double normx, double normy, double normz)) {
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
	
	// get the components at a given index in the array (no bounds checking)
	template<typename T> T * cell(int x) const;
	template<typename T> T * cell(int x, int y) const;
	template<typename T> T * cell(int x, int y, int z) const;
	
	// read the plane values from array into val array (no bounds checking)
	template<typename T> void read(T* val, int x) const;
	template<typename T> void read(T* val, int x, int y) const;
	template<typename T> void read(T* val, int x, int y, int z) const;
	
	// read the plane values from array into val array (wraps periodically at bounds)
	template<typename T> void read_wrap(T* val, int x) const;
	template<typename T> void read_wrap(T* val, int x, int y) const;
	template<typename T> void read_wrap(T* val, int x, int y, int z) const;
	
	// linear interpolated lookup (virtual array index)
	// reads the linearly interpolated plane values into val array
	template<typename T> void read_interp(T * val, double x) const;
	template<typename T> void read_interp(T * val, double x, double y) const;
	template<typename T> void read_interp(T * val, double x, double y, double z) const;
	
	// write plane values from val array into array (no bounds checking)
	template<typename T> void write(T* val, double x);
	template<typename T> void write(T* val, double x, double y);
	template<typename T> void write(T* val, double x, double y, double z);
	
	// write plane values from val array into array (wraps periodically at bounds)
	template<typename T> void write_wrap(T* val, int x);
	template<typename T> void write_wrap(T* val, int x, int y);
	template<typename T> void write_wrap(T* val, int x, int y, int z);
	
	// linear interpolated write (virtual array index)
	// writes the linearly interpolated plane values from val array into array
	template<typename T> void write_interp(T* val, double x);
	template<typename T> void write_interp(T* val, double x, double y);
	template<typename T> void write_interp(T* val, double x, double y, double z);
};
	
	
#pragma mark --------------------------------------
/*
 
 ********* INLINE IMPLEMENTATION BELOW ***********
 
*/

inline void Array::define(AlloArrayHeader& h, int components, AlloTy ty, int dimx) {
	Array::defineAligned(h, components, ty, dimx, AL_ARRAY_DEFAULT_ALIGNMENT);
}
inline void Array::define(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy) {
	Array::defineAligned(h, components, ty, dimx, dimy, AL_ARRAY_DEFAULT_ALIGNMENT);
}
inline void Array::define(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy, int dimz) {
	Array::defineAligned(h, components, ty, dimx, dimy, dimz, AL_ARRAY_DEFAULT_ALIGNMENT);
}

inline void Array::defineAligned(AlloArrayHeader& h, int components, AlloTy ty, int dimx, size_t align) {
	h.type = ty;
	h.components = components;
	h.dimcount = 1;
	h.dim[0] = dimx;
	Array::deriveStride(h, align);
}

inline void Array::defineAligned(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy, size_t align) {
	h.type = ty;
	h.components = components;
	h.dimcount = 2;
	h.dim[0] = dimx;
	h.dim[1] = dimy;
	Array::deriveStride(h, align);
}

inline void Array::defineAligned(AlloArrayHeader& h, int components, AlloTy ty, int dimx, int dimy, int dimz, size_t align) {
	h.type = ty;
	h.components = components;
	h.dimcount = 3;
	h.dim[0] = dimx;
	h.dim[1] = dimy;
	h.dim[2] = dimz;
	Array::deriveStride(h, align);
}

/*
 Set stride factors based on a specific byte alignment
 */
inline void Array::deriveStride(AlloArrayHeader& h, size_t alignSize) {
	unsigned typeSize = allo_type_size(h.type);
	unsigned numDims = h.dimcount;
	h.stride[0] = h.components * typeSize;
	
	if(numDims>1){
		h.stride[1] = h.stride[0] * h.dim[0];		// compute ideal row stride amount
		unsigned remain = h.stride[1] % alignSize;		// compute pad bytes
		if(remain){ h.stride[1] += alignSize - remain;}// add pad bytes (if any)
		
		unsigned i=2;
		for(; i<numDims; ++i){ h.stride[i] = h.stride[i-1] * h.dim[i-1]; }
	}
}
	
/*
	Type traits by partial specialization:
 */
template<> inline AlloTy Array::type<uint8_t>() { return AlloUInt8Ty; }
template<> inline AlloTy Array::type<uint16_t>() { return AlloUInt16Ty; }
template<> inline AlloTy Array::type<uint32_t>() { return AlloUInt32Ty; }
template<> inline AlloTy Array::type<uint64_t>() { return AlloUInt64Ty; }
template<> inline AlloTy Array::type<int8_t>() { return AlloSInt8Ty; }
template<> inline AlloTy Array::type<int16_t>() { return AlloSInt16Ty; }
template<> inline AlloTy Array::type<int32_t>() { return AlloSInt32Ty; }
template<> inline AlloTy Array::type<int64_t>() { return AlloSInt64Ty; }
template<> inline AlloTy Array::type<float>() { return AlloFloat32Ty; }
template<> inline AlloTy Array::type<double>() { return AlloFloat64Ty; }
template<> inline AlloTy Array::type<AlloArray>() { return AlloArrayTy; }
	
template<typename T> inline T * Array::cell(int x) const {
	int fieldstride_x = header.stride[0];
	return (T *)(data.ptr + x*fieldstride_x);
}
template<typename T> inline T * Array::cell(int x, int y) const {
	int fieldstride_x = header.stride[0];
	int fieldstride_y = header.stride[1];
	return (T *)(data.ptr + x*fieldstride_x + y*fieldstride_y);
}
template<typename T> inline T * Array::cell(int x, int y, int z) const {
	int fieldstride_x = header.stride[0];
	int fieldstride_y = header.stride[1];
	int fieldstride_z = header.stride[2];
	return (T *)(data.ptr + x*fieldstride_x + y*fieldstride_y + z*fieldstride_z);
}


// read the plane values from array into val array (no bounds checking)
template<typename T> inline void Array::read(T* val, int x) const {
	T * paaa = cell<T>(x);
	for (uint8_t p=0; p<header.components; p++) {		
		val[p] = paaa[p];
	}
}
template<typename T> inline void Array::read(T* val, int x, int y) const {
	T * paaa = cell<T>(x, y);
	for (uint8_t p=0; p<header.components; p++) {		
		val[p] = paaa[p];
	}
}
template<typename T> inline void Array::read(T* val, int x, int y, int z) const {
	T * paaa = cell<T>(x, y, z);
	for (uint8_t p=0; p<header.components; p++) {		
		val[p] = paaa[p];
	}
}

// read the plane values from array into val array (wraps periodically at bounds)
template<typename T> inline void Array::read_wrap(T* val, int x) const {
	read(val, wrap<int>(x, header.dim[0], 0));
}
template<typename T> inline void Array::read_wrap(T* val, int x, int y) const {
	read(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0));
}
template<typename T> inline void Array::read_wrap(T* val, int x, int y, int z) const {
	read(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0), wrap<int>(z, header.dim[2], 0));
}

// linear interpolated lookup (virtual array index)
// reads the linearly interpolated plane values into val array
template<typename T> inline void Array::read_interp(T * val, double x) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	int xa = (int)DOUBLE_FLOOR(x);	
	int xb = (int)DOUBLE_CEIL (x);				
	// get the normalized 0..1 interp factors, of x,y,z:
	double faaa = DOUBLE_FRAC(x);
	double fbaa = 1.f - faaa;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa); 
	T * pbaa = cell<T>(xb); 
	// for each plane of the field, do the interp:
	for (uint8_t p=0; p<header.components; p++) {	
		val[p] =	(paaa[p] * faaa) + (pbaa[p] * fbaa);
	}
}

template<typename T> inline void Array::read_interp(T * val, double x, double y) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	int xa = (int)DOUBLE_FLOOR(x);	
	int xb = (int)DOUBLE_CEIL (x);
	int ya = (int)DOUBLE_FLOOR(y);	
	int yb = (int)DOUBLE_CEIL (y);				
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
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

template<typename T> inline void Array::read_interp(T * val, double x, double y, double z) const {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	z = wrap<double>(z, (double)header.dim[2], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	int xa = (int)DOUBLE_FLOOR(x);	
	int xb = (int)DOUBLE_CEIL (x);
	int ya = (int)DOUBLE_FLOOR(y);	
	int yb = (int)DOUBLE_CEIL (y);
	int za = (int)DOUBLE_FLOOR(z);	
	int zb = (int)DOUBLE_CEIL (z);				
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
	double yaf = 1.f - ybf;
	double zbf = DOUBLE_FRAC(z);
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
	for (uint8_t p=0; p<header.components; p++) {	
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
template<typename T> inline void Array::write(T* val, double x) {
	T * paaa = cell<T>(x);
	for (uint8_t p=0; p<header.components; p++) {		
		paaa[p] = val[p];
	}
}
template<typename T> inline void Array::write(T* val, double x, double y) {
	T * paaa = cell<T>(x, y);
	for (uint8_t p=0; p<header.components; p++) {		
		paaa[p] = val[p];
	}
}
template<typename T> inline void Array::write(T* val, double x, double y, double z) {
	T * paaa = cell<T>(x, y, z);
	for (uint8_t p=0; p<header.components; p++) {		
		paaa[p] = val[p];
	}
}

// write plane values from val array into array (wraps periodically at bounds)
template<typename T> inline void Array::write_wrap(T* val, int x) {
	write(val, wrap<int>(x, header.dim[0], 0));
}
template<typename T> inline void Array::write_wrap(T* val, int x, int y) {
	write(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0));
}
template<typename T> inline void Array::write_wrap(T* val, int x, int y, int z) {
	write(val, wrap<int>(x, header.dim[0], 0), wrap<int>(y, header.dim[1], 0), wrap<int>(z, header.dim[2], 0));
}

// linear interpolated write (virtual array index)
// writes the linearly interpolated plane values from val array into array
template<typename T> inline void Array::write_interp(T* val, double x) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	int xa = (int)DOUBLE_FLOOR(x);	
	int xb = (int)DOUBLE_CEIL (x);				
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
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
template<typename T> inline void Array::write_interp(T* val, double x, double y) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	int xa = (int)DOUBLE_FLOOR(x);	
	int xb = (int)DOUBLE_CEIL (x);
	int ya = (int)DOUBLE_FLOOR(y);	
	int yb = (int)DOUBLE_CEIL (y);				
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
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
template<typename T> inline void Array::write_interp(T* val, double x, double y, double z) {
	x = wrap<double>(x, (double)header.dim[0], 0.);
	y = wrap<double>(y, (double)header.dim[1], 0.);
	z = wrap<double>(z, (double)header.dim[2], 0.);
	// convert 0..1 field indices to 0..(d-1) cell indices
	int xa = (int)DOUBLE_FLOOR(x);	
	int xb = (int)DOUBLE_CEIL (x);
	int ya = (int)DOUBLE_FLOOR(y);	
	int yb = (int)DOUBLE_CEIL (y);
	int za = (int)DOUBLE_FLOOR(z);	
	int zb = (int)DOUBLE_CEIL (z);				
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = DOUBLE_FRAC(x);
	double xaf = 1.f - xbf;
	double ybf = DOUBLE_FRAC(y);
	double yaf = 1.f - ybf;
	double zbf = DOUBLE_FRAC(z);
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

	
#undef DOUBLE_FLOOR
#undef DOUBLE_CEIL
#undef DOUBLE_FRAC

} // ::al::

#endif /* INCLUDE_ALLO_TYPES_CPP_H */
