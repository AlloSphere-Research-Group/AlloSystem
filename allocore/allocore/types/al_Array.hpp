#ifndef INC_AL_ARRAY_HPP
#define INC_AL_ARRAY_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	C++ wrapper and utilities for AlloArray

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <cmath> // fmod
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
	template <unsigned i> uint32_t dim() const {
		static_assert(i<=ALLO_ARRAY_MAX_DIMS, "Invalid index");
		return dim(i);
	}
	unsigned width() const { return dim<0>(); }					///< Get size of first dimension
	unsigned height() const { return dim<1>(); }					///< Get size of second dimension
	unsigned depth() const { return dim<2>(); }					///< Get size of third dimension
	uint32_t stride(int i=0) const { return header.stride[i]; }	///< Get stride of dimension, in bytes
	template <unsigned i> uint32_t stride() const {
		static_assert(i<=ALLO_ARRAY_MAX_DIMS, "Invalid index");
		return stride(i);
	}

	/// Returns the maximum possible byte alignment of the rows (1, 2, 4 or 8 byte)
	uint32_t alignment() const;

	/// Returns the total memory footprint, in bytes
	size_t size() const { return allo_array_size(this); }

	/// Returns number of cells in the Array:
	unsigned cells() const { return allo_array_elements(this); }

	/// Change the format without de/re/allocating:
	void configure(const AlloArrayHeader& h);

	///	Change the format (header/layout) of the Array reallocating if necessary
	void format(const AlloArrayHeader& h);

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

	/// Set 1D source array to reference
	template <class T>
	void ref(T * src, int comps, uint32_t dimx){
		uint32_t dims[] = {dimx};
		ref(src, comps, dims,1);
	}

	/// Set 2D source array to reference
	template <class T>
	void ref(T * src, int comps, uint32_t dimx, uint32_t dimy){
		uint32_t dims[] = {dimx, dimy};
		ref(src, comps, dims,2);
	}

	/// Set 3D source array to reference
	template <class T>
	void ref(T * src, int comps, uint32_t dimx, uint32_t dimy, uint32_t dimz){
		uint32_t dims[] = {dimx, dimy, dimz};
		ref(src, comps, dims,3);
	}

	template <class T>
	void ref(T * src, int comps, uint32_t * dims, int numDims){
		dataFree();
		data.ptr = decltype(data.ptr)(src);
		mIsRef = true;
		configure(getHeader(comps, type<T>(), dims,numDims, 1));
	}

	/// Check if this Array conforms to an ArrayHeader format
	bool isFormat(const AlloArrayHeader& h2) const;
	bool isFormat(const Array& src) const { return isFormat(src.header); }

	/// Returns true if there is no data
	bool empty() const { return NULL == data.ptr; }

	/// Returns true if there is data
	bool hasData() const { return !empty(); }

	/// Allocate memory for the given header.

	/// Warning: does not check if memory was already allocated;
	/// Call dataFree() first if you know it will be safe to do so.
	void dataCalloc();

	/// Free memory and set internal data pointer to null
	void dataFree();

	/// Set all data to zero
	void zero();


	/// Get mutable component using 1-D index
	template <class T> T& elem(size_t ic, size_t ix)
		{ return cell<T>(ix)[ic]; }

	/// Get mutable component using 2-D index
	template <class T> T& elem(size_t ic, size_t ix, size_t iy)
		{ return cell<T>(ix,iy)[ic]; }

	/// Get mutable component using 3-D index
	template <class T> T& elem(size_t ic, size_t ix, size_t iy, size_t iz)
		{ return cell<T>(ix,iy,iz)[ic]; }

	/// Get const component using 1-D index
	template <class T> const T& elem(size_t ic, size_t ix) const
		{ return cell<T>(ix)[ic]; }

	/// Get const component using 2-D index
	template <class T> const T& elem(size_t ic, size_t ix, size_t iy) const
		{ return cell<T>(ix,iy)[ic]; }

	/// Get const component using 3-D index
	template <class T> const T& elem(size_t ic, size_t ix, size_t iy, size_t iz) const
		{ return cell<T>(ix,iy,iz)[ic]; }


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
	bool mIsRef = false;

	AlloArrayHeader getHeader(int comps, AlloTy ty, uint32_t * dims, int numDims, size_t align);

	void formatAlignedGeneral(int comps, AlloTy ty, uint32_t * dims, int numDims, size_t align);

public:	// temporarily made public, because protected broke some other project code -gw
	Array(const Array&);
	Array& operator= (const Array&);
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
namespace{
template<int NumBytes> constexpr AlloTy ptrType();
template<> constexpr AlloTy ptrType<4>(){ return AlloPointer32Ty; }
template<> constexpr AlloTy ptrType<8>(){ return AlloPointer64Ty; }
}
template<> constexpr AlloTy Array::type<void *>(){ return ptrType<sizeof(void*)>(); }


/*template<> constexpr AlloTy Array::type<void *>(){
	switch(sizeof(void *)) {
		case 4: return AlloPointer32Ty;
		case 8: return AlloPointer64Ty;
	}
	return 0;
}*/

template<class T> inline T * Array::cell(size_t x) const {
	return (T *)(data.ptr + x*stride<0>());
}
template<class T> inline T * Array::cell(size_t x, size_t y) const {
	return (T *)(data.ptr + x*stride<0>() + y*stride<1>());
}
template<class T> inline T * Array::cell(size_t x, size_t y, size_t z) const {
	return (T *)(data.ptr + x*stride<0>() + y*stride<1>() + z*stride<2>());
}


// read the plane values from array into val array (no bounds checking)
template<class T> inline void Array::read(T * val, int x) const {
	T * c = cell<T>(x);
	for(uint8_t i=0; i<components(); i++) val[i] = c[i];
}
template<class T> inline void Array::read(T * val, int x, int y) const {
	T * c = cell<T>(x, y);
	for(uint8_t i=0; i<components(); i++) val[i] = c[i];
}
template<class T> inline void Array::read(T * val, int x, int y, int z) const {
	T * c = cell<T>(x, y, z);
	for(uint8_t i=0; i<components(); i++) val[i] = c[i];
}

#define AL_ARRAY_FLOOR(v) ( (long)(v) - ((v)<0. && (v)!=(long)(v)) )
#define AL_ARRAY_FRAC(v) ( ((v)>=0.) ? (v)-(long)(v) : (-v)-(long)(v) )

// linear interpolated lookup (virtual array index)
// reads the linearly interpolated plane values into val array
template<class T> inline void Array::read_interp(T * val, double x) const {
	x = std::fmod<double>(x, width());
	// convert 0..1 field indices to 0..(d-1) cell indices
	unsigned xa = AL_ARRAY_FLOOR(x);
	unsigned xb = xa+1;	if(xb == width()) xb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double faaa = AL_ARRAY_FRAC(x);
	double fbaa = 1. - faaa;
	// get the cell addresses for each neighbor:
	T * paaa = cell<T>(xa);
	T * pbaa = cell<T>(xb);
	// for each plane of the field, do the interp:
	for(uint8_t i=0; i<components(); i++)
		val[i] = paaa[i]*faaa + pbaa[i]*fbaa;
}

template<class T> inline void Array::read_interp(T * val, double x, double y) const {
	x = std::fmod<double>(x, width());
	y = std::fmod<double>(y, height());
	unsigned xa = AL_ARRAY_FLOOR(x);
	unsigned ya = AL_ARRAY_FLOOR(y);
	unsigned xb = xa+1;	if(xb == width()) xb = 0;
	unsigned yb = ya+1;	if(yb ==height()) yb = 0;
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1. - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1. - ybf;
	double faaa = xaf * yaf;
	double faba = xaf * ybf;
	double fbaa = xbf * yaf;
	double fbba = xbf * ybf;
	T * paaa = cell<T>(xa, ya);
	T * paba = cell<T>(xa, yb);
	T * pbaa = cell<T>(xb, ya);
	T * pbba = cell<T>(xb, yb);
	for(uint8_t i=0; i<components(); i++)
		val[i] = 	paaa[i]*faaa + pbaa[i]*fbaa +
					paba[i]*faba + pbba[i]*fbba;
}

template<class T> inline void Array::read_interp(T * val, double x, double y, double z) const {
	x = std::fmod<double>(x, width());
	y = std::fmod<double>(y, height());
	z = std::fmod<double>(z, depth());
	unsigned xa = AL_ARRAY_FLOOR(x);
	unsigned ya = AL_ARRAY_FLOOR(y);
	unsigned za = AL_ARRAY_FLOOR(z);
	unsigned xb = xa+1;	if(xb == width()) xb = 0;
	unsigned yb = ya+1;	if(yb ==height()) yb = 0;
	unsigned zb = za+1;	if(zb == depth()) zb = 0;
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1. - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1. - ybf;
	double zbf = AL_ARRAY_FRAC(z);
	double zaf = 1. - zbf;
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
	for (size_t i=0; i<components(); i++)
		val[i] =	paaa[i] * faaa + pbaa[i] * fbaa +
					paba[i] * faba + paab[i] * faab +
					pbab[i] * fbab + pabb[i] * fabb +
					pbba[i] * fbba + pbbb[i] * fbbb;
}

// write plane values from val array into array (no bounds checking)
template<class T> inline void Array::write(const T * val, int x) {
	T * c = cell<T>(x);
	for(uint8_t i=0; i<components(); ++i) c[i] = val[i];
}
template<class T> inline void Array::write(const T * val, int x, int y) {
	T * c = cell<T>(x, y);
	for(uint8_t i=0; i<components(); ++i) c[i] = val[i];
}
template<class T> inline void Array::write(const T * val, int x, int y, int z) {
	T * c = cell<T>(x, y, z);
	for(uint8_t i=0; i<components(); ++i) c[i] = val[i];
}

// linear interpolated write (virtual array index)
// writes the linearly interpolated plane values from val array into array
template<class T> inline void Array::write_interp(const T* val, double x) {
	x = std::fmod<double>(x, width());
	unsigned xa = AL_ARRAY_FLOOR(x);
	unsigned xb = xa+1;	if(xb == width()) xb = 0;
	// get the normalized 0..1 interp factors, of x,y,z:
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1. - xbf;
	// get the interpolation corner weights:
	double faaa = xaf;
	double fbaa = xbf;
	T * paaa = cell<T>(xa);
	T * pbaa = cell<T>(xb);
	// for each plane of the field, do the 3D interp:
	for(uint8_t i=0; i<components(); i++){
		T tmp = val[i];
		paaa[i] += tmp * faaa;
		pbaa[i] += tmp * fbaa;
	}
}
template<class T> inline void Array::write_interp(const T* val, double x, double y) {
	x = std::fmod<double>(x, width());
	y = std::fmod<double>(y, height());
	unsigned xa = AL_ARRAY_FLOOR(x);
	unsigned ya = AL_ARRAY_FLOOR(y);
	unsigned xb = xa+1;	if(xb == width()) xb = 0;
	unsigned yb = ya+1;	if(yb ==height()) yb = 0;
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1. - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1. - ybf;
	double faaa = xaf * yaf;
	double faba = xaf * ybf;
	double fbaa = xbf * yaf;
	double fbba = xbf * ybf;
	T * paaa = cell<T>(xa, ya);
	T * paba = cell<T>(xa, yb);
	T * pbaa = cell<T>(xb, ya);
	T * pbba = cell<T>(xb, yb);
	for(uint8_t i=0; i<components(); i++){
		T tmp = val[i];
		paaa[i] += tmp * faaa;
		paba[i] += tmp * faba;
		pbaa[i] += tmp * fbaa;
		pbba[i] += tmp * fbba;
	}
}

template<class T> inline void Array::write_interp(const T* val, double x0, double y0, double z0) {
	double x = std::fmod<double>(x0, width());
	double y = std::fmod<double>(y0, height());
	double z = std::fmod<double>(z0, depth());
	unsigned xa = AL_ARRAY_FLOOR(x);
	unsigned ya = AL_ARRAY_FLOOR(y);
	unsigned za = AL_ARRAY_FLOOR(z);
	unsigned xb = xa+1;	if(xb == width()) xb = 0;
	unsigned yb = ya+1;	if(yb ==height()) yb = 0;
	unsigned zb = za+1;	if(zb == depth()) zb = 0;
	double xbf = AL_ARRAY_FRAC(x);
	double xaf = 1. - xbf;
	double ybf = AL_ARRAY_FRAC(y);
	double yaf = 1. - ybf;
	double zbf = AL_ARRAY_FRAC(z);
	double zaf = 1. - zbf;
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
	for(uint8_t i=0; i<components(); i++){
		T tmp = val[i];
		paaa[i] += tmp * faaa;
		paab[i] += tmp * faab;
		paba[i] += tmp * faba;
		pabb[i] += tmp * fabb;
		pbaa[i] += tmp * fbaa;
		pbab[i] += tmp * fbab;
		pbba[i] += tmp * fbba;
		pbbb[i] += tmp * fbbb;
	}
}

#undef AL_ARRAY_FLOOR
#undef AL_ARRAY_FRAC

template<class T> void Array::fill(void (*func)(T * values, double normx)) {
	unsigned d0 = dim<0>();
	double inv_d0 = 1./d0;

	T * vals = (T *)(data.ptr);
	for(unsigned x=0; x < d0; x++){
		func(vals, inv_d0 * x);
		vals += components();
	}
}

template<class T> void Array::fill(void (*func)(T * values, double normx, double normy)) {
	unsigned d0 = dim<0>();
	unsigned d1 = dim<1>();
	unsigned s1 = stride<1>();
	double inv_d0 = 1./d0;
	double inv_d1 = 1./d1;

	for(unsigned y=0; y < d1; y++){
		T * vals = (T *)(data.ptr + s1*y);
		for(unsigned x=0; x < d0; x++){
			func(vals, inv_d0 * x, inv_d1 * y);
			vals += components();
		}
	}
}

template<class T> void Array::fill(void (*func)(T * values, double normx, double normy, double normz)) {
	unsigned d0 = dim<0>();
	unsigned d1 = dim<1>();
	unsigned d2 = dim<2>();
	unsigned s1 = stride<1>();
	unsigned s2 = stride<2>();
	double inv_d0 = 1./d0;
	double inv_d1 = 1./d1;
	double inv_d2 = 1./d2;

	for(unsigned z=0; z < d2; z++){
		for(unsigned y=0; y < d1; y++){
			T * vals = (T *)(data.ptr + s1*y + s2*z);
			for(unsigned x=0; x < d0; x++){
				func(vals, inv_d0 * x, inv_d1 * y, inv_d2 * z);
				vals += components();
			}
		}
	}
}

template<class T> void Array::setall(T value) {
	unsigned d0 = dim<0>();
	unsigned d1 = dim<1>();
	//unsigned d2 = dim<2>();
	unsigned s0 = stride<0>();
	unsigned s1 = stride<1>();
	unsigned s2 = stride<2>();

	switch(dimcount()){
	case 3:
		for(unsigned z=0; z < d1; z++){
			for(unsigned y=0; y < d1; y++){
				T * vals = (T *)(data.ptr + s1*y + s2*z);
				for(unsigned x=0; x < d0; x++){
					for(unsigned i=0; i<components(); i++)
						vals[i] = value;
				}
			}
		}
		break;
	case 2:
		for(unsigned y=0; y < d1; y++) {
			for(unsigned x=0; x < d0; x++) {
				T * vals = (T *)(data.ptr + s0*x + s1*y);
				for(unsigned i=0; i<components(); i++)
					vals[i] = value;
			}
		}
		break;
	case 1:{
		T * vals = (T *)(data.ptr);
		for(unsigned x=0; x < d0; x++){
			for(unsigned i=0; i<components(); i++)
				vals[i] = value;
		}
		} break;
	default:
		break;
	}

}

template<class T> void Array::set1d(T * cell) {
	unsigned d0 = dim<0>();
	unsigned s0 = stride<0>();

	for(unsigned x=0; x < d0; x++){
		T * vals = (T *)(data.ptr + s0*x);
		for(unsigned i=0; i<components(); i++)
			vals[i] = cell[i];
	}
}

template<class T> void Array::set2d(T * cell) {
	unsigned d0 = dim<0>();
	unsigned d1 = dim<1>();
	unsigned s0 = stride<0>();
	unsigned s1 = stride<1>();

	for(unsigned y=0; y < d1; y++){
		for(unsigned x=0; x < d0; x++){
			T * vals = (T *)(data.ptr + s0*x + s1*y);
			for(unsigned i=0; i<components(); i++)
				vals[i] = cell[i];
		}
	}
}

template<class T> void Array::set3d(T * cell) {
	unsigned d0 = dim<0>();
	unsigned d1 = dim<1>();
	unsigned d2 = dim<1>();
	unsigned s0 = stride<0>();
	unsigned s1 = stride<1>();
	unsigned s2 = stride<2>();

	for(unsigned z=0; z < d2; z++){
		for(unsigned y=0; y < d1; y++){
			for(unsigned x=0; x < d0; x++){
				T * vals = (T *)(data.ptr + s0*x + s1*y + s2*z);
				for(unsigned i=0; i<components(); i++)
					vals[i] = cell[i];
			}
		}
	}
}

} // al::
#endif
