#ifndef INC_AL_ARRAY_HPP
#define INC_AL_ARRAY_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	C++ wrapper and utilities for AlloArray

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <cmath> // floor
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


	/// Fill with the same component values throughout
	template<class T> void set1d(const T * comps);
	template<class T> void set2d(const T * comps);
	template<class T> void set3d(const T * comps);

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

	struct Lookup{
		Lookup(double pos, unsigned len){
			auto q = std::floor(pos);
			i = (int(q)%len + len) % len; // % allowing left operand to be negative
			j = i+1; if(j==len) j=0;
			f = pos - q;
		}
		int i,j;
		double f; // fraction in [0,1]
		template <class T>
		T get(T a, T b) const { return (b-a)*f + a; }
		template <class T>
		void set(T v, T& a, T& b) const { a=v*(1.-f); b=v*f; }
		template <class T>
		void add(T v, T& a, T& b) const { a+=v*(1.-f); b+=v*f; }
	};

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

// linear interpolated lookup (virtual array index)
// reads the linearly interpolated plane values into val array
template<class T> inline void Array::read_interp(T * val, double x) const {
	Lookup lx(x, width());
	const T * abc = cell<T>(lx.i);
	const T * Abc = cell<T>(lx.j);
	for(uint8_t i=0; i<components(); i++)
		val[i] = lx.get(abc[i], Abc[i]);
}

template<class T> inline void Array::read_interp(T * val, double x, double y) const {
	Lookup lx(x, width());
	Lookup ly(y, height());
	const T * abc = cell<T>(lx.i, ly.i);
	const T * Abc = cell<T>(lx.j, ly.i);
	const T * aBc = cell<T>(lx.i, ly.j);
	const T * ABc = cell<T>(lx.j, ly.j);
	for(uint8_t i=0; i<components(); i++)
		val[i] = ly.get(
			lx.get(abc[i], Abc[i]),
			lx.get(aBc[i], ABc[i])
		);
}

template<class T> inline void Array::read_interp(T * val, double x, double y, double z) const {
	Lookup lx(x, width());
	Lookup ly(y, height());
	Lookup lz(z, depth());
	const T * abc = cell<T>(lx.i, ly.i, lz.i);
	const T * Abc = cell<T>(lx.j, ly.i, lz.i);
	const T * aBc = cell<T>(lx.i, ly.j, lz.i);
	const T * ABc = cell<T>(lx.j, ly.j, lz.i);
	const T * abC = cell<T>(lx.i, ly.i, lz.j);
	const T * AbC = cell<T>(lx.j, ly.i, lz.j);
	const T * aBC = cell<T>(lx.i, ly.j, lz.j);
	const T * ABC = cell<T>(lx.j, ly.j, lz.j);
	for (size_t i=0; i<components(); i++)
		val[i] = lz.get(
			ly.get(
				lx.get(abc[i], Abc[i]),
				lx.get(aBc[i], ABc[i])
			),
			ly.get(
				lx.get(abC[i], AbC[i]),
				lx.get(aBC[i], ABC[i])
			)
		);
}

// write plane values from val array into array (no bounds checking)
template<class T> inline void Array::write(const T * src, int x) {
	T * c = cell<T>(x);
	for(uint8_t i=0; i<components(); ++i) c[i] = src[i];
}
template<class T> inline void Array::write(const T * src, int x, int y) {
	T * c = cell<T>(x, y);
	for(uint8_t i=0; i<components(); ++i) c[i] = src[i];
}
template<class T> inline void Array::write(const T * src, int x, int y, int z){
	T * c = cell<T>(x, y, z);
	for(uint8_t i=0; i<components(); ++i) c[i] = src[i];
}

// linear interpolated write (virtual array index)
// writes the linearly interpolated plane values from val array into array
template<class T> inline void Array::write_interp(const T * val, double x){
	Lookup lx(x, width());
	T * abc = cell<T>(lx.i);
	T * Abc = cell<T>(lx.j);
	for(uint8_t i=0; i<components(); i++){
		lx.add(val[i], abc[i], Abc[i]);
	}
}
template<class T> inline void Array::write_interp(const T * val, double x, double y){
	Lookup lx(x, width());
	Lookup ly(y, height());
	T * abc = cell<T>(lx.i, ly.i);
	T * Abc = cell<T>(lx.j, ly.i);
	T * aBc = cell<T>(lx.i, ly.j);
	T * ABc = cell<T>(lx.j, ly.j);
	for(uint8_t i=0; i<components(); i++){
		T b, B;
		ly.set(val[i], b, B);
		lx.add(b, abc[i], Abc[i]);
		lx.add(B, aBc[i], ABc[i]);
	}
}

template<class T> inline void Array::write_interp(const T * val, double x, double y, double z){
	Lookup lx(x, width());
	Lookup ly(y, height());
	Lookup lz(z, depth());
	T * abc = cell<T>(lx.i, ly.i, lz.i);
	T * Abc = cell<T>(lx.j, ly.i, lz.i);
	T * aBc = cell<T>(lx.i, ly.j, lz.i);
	T * ABc = cell<T>(lx.j, ly.j, lz.i);
	T * abC = cell<T>(lx.i, ly.i, lz.j);
	T * AbC = cell<T>(lx.j, ly.i, lz.j);
	T * aBC = cell<T>(lx.i, ly.j, lz.j);
	T * ABC = cell<T>(lx.j, ly.j, lz.j);
	for(uint8_t i=0; i<components(); i++){
		T c, C;
		lz.set(val[i], c, C);
		T bc, Bc, bC, BC;
		ly.set(c, bc, Bc);
		ly.set(C, bC, BC);
		lx.add(bc, abc[i], Abc[i]);
		lx.add(Bc, aBc[i], ABc[i]);
		lx.add(bC, abC[i], AbC[i]);
		lx.add(BC, aBC[i], ABC[i]);
	}
}

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

template<class T> void Array::setall(T value){
	switch(dimcount()){
	case 3:
		for(unsigned z=0; z < dim<2>(); z++){
			for(unsigned y=0; y < dim<1>(); y++){
				for(unsigned x=0; x < dim<0>(); x++){
					T * vals = cell<T>(x,y,z);
					for(unsigned i=0; i<components(); i++)
						vals[i] = value;
				}
			}
		}
		break;
	case 2:
		for(unsigned y=0; y < dim<1>(); y++) {
			for(unsigned x=0; x < dim<0>(); x++) {
				T * vals = cell<T>(x,y);
				for(unsigned i=0; i<components(); i++)
					vals[i] = value;
			}
		}
		break;
	case 1:
		for(unsigned x=0; x < dim<0>(); x++){
			T * vals = cell<T>(x);
			for(unsigned i=0; i<components(); i++)
				vals[i] = value;
		}
		break;
	default:
		break;
	}

}

template<class T> void Array::set1d(const T * src) {
	for(unsigned x=0; x < dim<0>(); x++)
		write(src, x);
}

template<class T> void Array::set2d(const T * src) {
	for(unsigned y=0; y < dim<1>(); y++){
		for(unsigned x=0; x < dim<0>(); x++)
			write(src, x,y);
	}
}

template<class T> void Array::set3d(const T * src) {
	for(unsigned z=0; z < dim<2>(); z++){
		for(unsigned y=0; y < dim<1>(); y++){
			for(unsigned x=0; x < dim<0>(); x++)
				write(src, x,y,z);
		}
	}
}

} // al::
#endif
