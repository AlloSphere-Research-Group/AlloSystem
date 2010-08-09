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

#include "al_types.h"
#include "math/al_Functions.hpp"	
#include <stdlib.h>

namespace al {

/*
	Partial specialization function to get type
	This demonstrates the principle by which a runtime type can be understood by templates
*/
template<typename T> inline AlloTy getType() { return 0; }
template<> inline AlloTy getType<uint8_t>() { return AlloUInt8Ty; }
template<> inline AlloTy getType<uint16_t>() { return AlloUInt16Ty; }
template<> inline AlloTy getType<uint32_t>() { return AlloUInt32Ty; }
template<> inline AlloTy getType<uint64_t>() { return AlloUInt64Ty; }
template<> inline AlloTy getType<int8_t>() { return AlloSInt8Ty; }
template<> inline AlloTy getType<int16_t>() { return AlloSInt16Ty; }
template<> inline AlloTy getType<int32_t>() { return AlloSInt32Ty; }
template<> inline AlloTy getType<int64_t>() { return AlloSInt64Ty; }
template<> inline AlloTy getType<float>() { return AlloFloat32Ty; }
template<> inline AlloTy getType<double>() { return AlloFloat64Ty; }
template<> inline AlloTy getType<AlloLattice>() { return AlloLatticeTy; }
// TODO: #define for platform ptrsize
//template<> AlloTy getType<void *>() { return AlloPointer32Ty; }
//template<> AlloTy getType<void *>() { return AlloPointer32Ty; }

/*
	E.g., verify a type:
*/
template<typename T> inline bool checkType(AlloTy ty) { return getType<T>() && ty == getType<T>(); }


/*
	Derived type
		N.B. methods and static members only... no additional instance member data!
*/
class LatticeHeader : public AlloLatticeHeader {
public:
	void define1d(uint32_t components, AlloTy ty, uint32_t dimx, size_t align = 4) {
		type = ty;
		this->components = components;
		dimcount = 1;
		dim[0] = dimx;
		allo_lattice_setstride(this, align);
	}
	
	void define2d(uint32_t components, AlloTy ty, uint32_t dimx, uint32_t dimy, size_t align = 4) {
		type = ty;
		this->components = components;
		dimcount = 2;
		dim[0] = dimx;
		dim[1] = dimy;
		allo_lattice_setstride(this, align);
	}
	
	void define3d(uint32_t components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, size_t align = 4) {
		type = ty;
		this->components = components;
		dimcount = 3;
		dim[0] = dimx;
		dim[1] = dimy;
		dim[2] = dimz;
		allo_lattice_setstride(this, align);
	}
};

/*
	Derived type
		N.B. methods and static members only... no additional instance member data!
*/
class Lattice : public AlloLattice {
public:

	Lattice() {
		data.ptr = 0;
		header.dimcount = 0;
		
		create1d(3, AlloPointer32Ty, 1, 0);
		data_calloc();
	}

	size_t size() { return allo_lattice_size(this); }
	
	void data_calloc() {
		data.ptr = (char *)calloc(1, size());
	}
	void data_free() {
		if(data.ptr) {
			free(data.ptr);
		}
	}
	
	bool equal(AlloLatticeHeader &h2) {
		bool equiv =	header.components == h2.components && 
						header.type == h2.type && 
						header.dimcount == h2.dimcount;
						
		for(int i=0; i < header.dimcount; i++) {
			equiv &= header.dim[i] == h2.dim[i];
			equiv &= header.stride[i] == h2.stride[i];
		}

		return equiv;
	}
	
	void create1d(int components, AlloTy type, int w, size_t align = 4) {
		define1d(components, type, w, align);
		data_calloc();
	}
	
	void create2d(int components, AlloTy type, int w, int h, size_t align = 4) {
		define2d(components, type, w, h, align);
		data_calloc();
	}
	
	void create3d(int components, AlloTy type, int w, int h, int d, size_t align = 4) {
		define3d(components, type, w, h, d, align);
		data_calloc();
	}
	
	void create(AlloLatticeHeader &h) {
		define(h);
		data_calloc();
	}

	void adapt1d(int components, AlloTy type, int w, size_t align = 4) {
		AlloLatticeHeader h;
		h.type = type;
		h.components = components;
		h.dimcount = 1;
		h.dim[0] = w;
		allo_lattice_setstride(&h, align);
		adapt(h);
	}
	
	void adapt2d(int components, AlloTy type, int w, int h, size_t align = 4) {
		AlloLatticeHeader hh;
		hh.type = type;
		hh.components = components;
		hh.dimcount = 2;
		hh.dim[0] = w;
		hh.dim[1] = h;
		allo_lattice_setstride(&hh, align);
		adapt(hh);
	}
	
	void adapt3d(int components, AlloTy type, int w, int h, int d, size_t align = 4) {
		AlloLatticeHeader hh;
		hh.type = type;
		hh.components = components;
		hh.dimcount = 3;
		hh.dim[0] = w;
		hh.dim[1] = h;
		hh.dim[2] = d;
		allo_lattice_setstride(&hh, align);
		adapt(hh);
	}
	
	void adapt(AlloLatticeHeader &h) {
		if(! equal(h)) {
			data_free();
			define(h);
			data_calloc();
		}
	}
	
	void adapt(AlloLattice *lattice) {
		if(! equal(lattice->header)) {
			data_free();
			define(lattice->header);
			data_calloc();
		}
	}
	
	void define(AlloLatticeHeader &h2) {
		header.components = h2.components;
		header.type = h2.type;
		header.dimcount = h2.dimcount;
		for(int i=0; i < header.dimcount; i++) {
			header.dim[i] = h2.dim[i];
			header.stride[i] = h2.stride[i];
		}
	}

	void define1d(uint32_t components, AlloTy ty, uint32_t dimx, size_t align = 4) {
		header.type = ty;
		header.components = components;
		header.dimcount = 1;
		header.dim[0] = dimx;
		allo_lattice_setstride(&header, align);
	}
	
	void define2d(uint32_t components, AlloTy ty, uint32_t dimx, uint32_t dimy, size_t align = 4) {
		header.type = ty;
		header.components = components;
		header.dimcount = 2;
		header.dim[0] = dimx;
		header.dim[1] = dimy;
		allo_lattice_setstride(&header, align);
	}
	
	void define3d(uint32_t components, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, size_t align = 4) {
		header.type = ty;
		header.components = components;
		header.dimcount = 3;
		header.dim[0] = dimx;
		header.dim[1] = dimy;
		header.dim[2] = dimz;
		allo_lattice_setstride(&header, align);
	}
	
	/*
		Use a function to fill a lattice with data:
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
	
	template<typename T> T * data1d(uint32_t x) {
		uint32_t fieldstride_x = header.stride[0];
		return (T *)(data.ptr + x*fieldstride_x);
	}
	
	
	// check whether the internal lattice data is of type T:
	template<typename T> bool checkType() { return al::checkType<T>(header.type); }
	
	// linear interpolated lookup (virtual lattice index x, y)
	// writes the linearly interpolated plane values into val array
	template<typename T> void interp1d(T* val, double x) {
	
		x = wrap<double>(x, (double)header.dim[0], 0.);
		
		#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
		#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
		#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )
		
		// convert 0..1 field indices to 0..(d-1) cell indices
		uint32_t xa = (uint32_t)DOUBLE_FLOOR(x);	
		uint32_t xb = (uint32_t)DOUBLE_CEIL (x);			
		
		// get the normalized 0..1 interp factors, of x,y,z:
		double fa = DOUBLE_FRAC(x);
		double fb = 1.f - fa;

		// get the cell addresses for each neighbor:
		uint32_t fieldstride_x = header.stride[0];
		T * pa = (T *)(data.ptr + xa*fieldstride_x); 
		T * pb = (T *)(data.ptr + xb*fieldstride_x); 

		// for each plane of the field, do the 3D interp:
		for (uint8_t p=0; p<header.components; p++) {		
			val[p] =	(pa[p] * fa) +
						(pb[p] * fb);
		}
		
		#undef DOUBLE_FLOOR
		#undef DOUBLE_CEIL
		#undef DOUBLE_FRAC
	}
	
	// linear interpolated write (virtual lattice index x, y)
	// writes the linearly interpolated plane values from val array into lattice
	template<typename T> void writeInterp1d(T* val, double x) {
	
		x = wrap<double>(x, (double)header.dim[0], 0.);
		
		#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
		#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
		#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )
		
		// convert 0..1 field indices to 0..(d-1) cell indices
		uint32_t xa = (uint32_t)DOUBLE_FLOOR(x);	
		uint32_t xb = (uint32_t)DOUBLE_CEIL (x);			
		
		// get the normalized 0..1 interp factors, of x,y,z:
		double fa = DOUBLE_FRAC(x);
		double fb = 1.f - fa;

		// get the cell addresses for each neighbor:
		uint32_t fieldstride_x = header.stride[0];
		T * pa = (T *)(data.ptr + xa*fieldstride_x); 
		T * pb = (T *)(data.ptr + xb*fieldstride_x); 

		// for each plane of the field, do the 3D interp:
		for (uint8_t p=0; p<header.components; p++) {		
			T tmp = val[p];
			pa[p] += tmp * fa;
			pb[p] += tmp * fb;
		}
		
		#undef DOUBLE_FLOOR
		#undef DOUBLE_CEIL
		#undef DOUBLE_FRAC
	}
	
	// bilinear interpolated lookup (virtual lattice index x, y)
	// writes the linearly interpolated plane values into val array
	template<typename T> void interp2d(T* val, double x, double y) {
	
		x = wrap<double>(x, (double)header.dim[0], 0.);
		y = wrap<double>(y, (double)header.dim[1], 0.);
		
		#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
		#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
		#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )
		
		// convert 0..1 field indices to 0..(d-1) cell indices
		uint32_t xa = (uint32_t)DOUBLE_FLOOR(x);	
		uint32_t xb = (uint32_t)DOUBLE_CEIL (x);
		uint32_t ya = (uint32_t)DOUBLE_FLOOR(y);	
		uint32_t yb = (uint32_t)DOUBLE_CEIL (y);			
		
		// get the normalized 0..1 interp factors, of x,y,z:
		double xbf = DOUBLE_FRAC(x);
		double xaf = 1.f - xbf;
		double ybf = DOUBLE_FRAC(y);
		double yaf = 1.f - ybf;
		
		// get the interpolation corner weights:
		double faa = xaf * yaf;
		double fab = xaf * ybf;
		double fba = xbf * yaf;
		double fbb = xbf * ybf;

		// get the cell addresses for each neighbor:
		uint32_t fieldstride_x = header.stride[0];
		uint32_t fieldstride_y = header.stride[1];
		T * paa = (T *)(data.ptr + xa*fieldstride_x + ya*fieldstride_y); 
		T * pab = (T *)(data.ptr + xa*fieldstride_x + yb*fieldstride_y); 
		T * pba = (T *)(data.ptr + xb*fieldstride_x + ya*fieldstride_y); 
		T * pbb = (T *)(data.ptr + xb*fieldstride_x + yb*fieldstride_y); 

		// for each plane of the field, do the 3D interp:
		for (uint8_t p=0; p<header.components; p++) {		
			val[p] =	(paa[p] * faa) +
						(pba[p] * fba) + 
						(pab[p] * fab) + 
						(pbb[p] * fbb);
		}
		
		#undef DOUBLE_FLOOR
		#undef DOUBLE_CEIL
		#undef DOUBLE_FRAC
	}
	
	// bilinear interpolated write (virtual lattice index x, y)
	// writes the linearly interpolated plane values from val array into lattice
	template<typename T> void writeInterp2d(T* val, double x, double y) {
	
		x = wrap<double>(x, (double)header.dim[0], 0.);
		y = wrap<double>(y, (double)header.dim[1], 0.);
		
		#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
		#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
		#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )
		
		// convert 0..1 field indices to 0..(d-1) cell indices
		uint32_t xa = (uint32_t)DOUBLE_FLOOR(x);	
		uint32_t xb = (uint32_t)DOUBLE_CEIL (x);
		uint32_t ya = (uint32_t)DOUBLE_FLOOR(y);	
		uint32_t yb = (uint32_t)DOUBLE_CEIL (y);			
		
		// get the normalized 0..1 interp factors, of x,y,z:
		double xbf = DOUBLE_FRAC(x);
		double xaf = 1.f - xbf;
		double ybf = DOUBLE_FRAC(y);
		double yaf = 1.f - ybf;
		
		// get the interpolation corner weights:
		double faa = xaf * yaf;
		double fab = xaf * ybf;
		double fba = xbf * yaf;
		double fbb = xbf * ybf;

		// get the cell addresses for each neighbor:
		uint32_t fieldstride_x = header.stride[0];
		uint32_t fieldstride_y = header.stride[1];
		T * paa = (T *)(data.ptr + xa*fieldstride_x + ya*fieldstride_y); 
		T * pab = (T *)(data.ptr + xa*fieldstride_x + yb*fieldstride_y); 
		T * pba = (T *)(data.ptr + xb*fieldstride_x + ya*fieldstride_y); 
		T * pbb = (T *)(data.ptr + xb*fieldstride_x + yb*fieldstride_y); 

		// for each plane of the field, do the 3D interp:
		for (uint8_t p=0; p<header.components; p++) {		
			T tmp = val[p];
			paa[p] += tmp * faa;
			pab[p] += tmp * fab;
			pba[p] += tmp * fba;
			pbb[p] += tmp * fbb;
		}
		
		#undef DOUBLE_FLOOR
		#undef DOUBLE_CEIL
		#undef DOUBLE_FRAC
	}


	// trilinear interpolated lookup (virtual lattice index x, y, z)
	// writes the linearly interpolated plane values into val array
	template<typename T> void interp3d(T* val, double x, double y, double z) {
	
		x = wrap<double>(x, (double)header.dim[0], 0.);
		y = wrap<double>(y, (double)header.dim[1], 0.);
		z = wrap<double>(z, (double)header.dim[2], 0.);
		
		#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
		#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
		#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )
		
		// convert 0..1 field indices to 0..(d-1) cell indices
		uint32_t xa = (uint32_t)DOUBLE_FLOOR(x);	
		uint32_t xb = (uint32_t)DOUBLE_CEIL (x);
		uint32_t ya = (uint32_t)DOUBLE_FLOOR(y);	
		uint32_t yb = (uint32_t)DOUBLE_CEIL (y);
		uint32_t za = (uint32_t)DOUBLE_FLOOR(z);	
		uint32_t zb = (uint32_t)DOUBLE_CEIL (z);				
		
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
		uint32_t fieldstride_x = header.stride[0];
		uint32_t fieldstride_y = header.stride[1];
		uint32_t fieldstride_z = header.stride[2];
		T * paaa = (T *)(data.ptr + xa*fieldstride_x + ya*fieldstride_y + za*fieldstride_z); 
		T * paab = (T *)(data.ptr + xa*fieldstride_x + ya*fieldstride_y + zb*fieldstride_z); 
		T * paba = (T *)(data.ptr + xa*fieldstride_x + yb*fieldstride_y + za*fieldstride_z); 
		T * pabb = (T *)(data.ptr + xa*fieldstride_x + yb*fieldstride_y + zb*fieldstride_z); 
		T * pbaa = (T *)(data.ptr + xb*fieldstride_x + ya*fieldstride_y + za*fieldstride_z); 
		T * pbab = (T *)(data.ptr + xb*fieldstride_x + ya*fieldstride_y + zb*fieldstride_z); 
		T * pbba = (T *)(data.ptr + xb*fieldstride_x + yb*fieldstride_y + za*fieldstride_z); 
		T * pbbb = (T *)(data.ptr + xb*fieldstride_x + yb*fieldstride_y + zb*fieldstride_z); 

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
		
		#undef DOUBLE_FLOOR
		#undef DOUBLE_CEIL
		#undef DOUBLE_FRAC
	}
	
	// trilinear interpolated write (virtual lattice index x, y, z)
	// writes the linearly interpolated plane values from val array into lattice
	template<typename T> void writeInterp3d(T* val, double x, double y, double z) {
	
		x = wrap<double>(x, (double)header.dim[0], 0.);
		y = wrap<double>(y, (double)header.dim[1], 0.);
		z = wrap<double>(z, (double)header.dim[2], 0.);
		
		#define DOUBLE_FLOOR(v) ( (long)(v) - ((v)<0.0 && (v)!=(long)(v)) )
		#define DOUBLE_CEIL(v) ( (((v)>0.0)&&((v)!=(long)(v))) ? 1+(v) : (v) )
		#define DOUBLE_FRAC(v) ( ((v)>=0.0) ? (v)-(long)(v) : (-v)-(long)(v) )
		
		// convert 0..1 field indices to 0..(d-1) cell indices
		uint32_t xa = (uint32_t)DOUBLE_FLOOR(x);	
		uint32_t xb = (uint32_t)DOUBLE_CEIL (x);
		uint32_t ya = (uint32_t)DOUBLE_FLOOR(y);	
		uint32_t yb = (uint32_t)DOUBLE_CEIL (y);
		uint32_t za = (uint32_t)DOUBLE_FLOOR(z);	
		uint32_t zb = (uint32_t)DOUBLE_CEIL (z);				
		
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
		uint32_t fieldstride_x = header.stride[0];
		uint32_t fieldstride_y = header.stride[1];
		uint32_t fieldstride_z = header.stride[2];
		T * paaa = (T *)(data.ptr + xa*fieldstride_x + ya*fieldstride_y + za*fieldstride_z); 
		T * paab = (T *)(data.ptr + xa*fieldstride_x + ya*fieldstride_y + zb*fieldstride_z); 
		T * paba = (T *)(data.ptr + xa*fieldstride_x + yb*fieldstride_y + za*fieldstride_z); 
		T * pabb = (T *)(data.ptr + xa*fieldstride_x + yb*fieldstride_y + zb*fieldstride_z); 
		T * pbaa = (T *)(data.ptr + xb*fieldstride_x + ya*fieldstride_y + za*fieldstride_z); 
		T * pbab = (T *)(data.ptr + xb*fieldstride_x + ya*fieldstride_y + zb*fieldstride_z); 
		T * pbba = (T *)(data.ptr + xb*fieldstride_x + yb*fieldstride_y + za*fieldstride_z); 
		T * pbbb = (T *)(data.ptr + xb*fieldstride_x + yb*fieldstride_y + zb*fieldstride_z); 

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
		
		#undef DOUBLE_FLOOR
		#undef DOUBLE_CEIL
		#undef DOUBLE_FRAC
	}
	
	
};

} // ::al::

#endif /* INCLUDE_ALLO_TYPES_CPP_H */
