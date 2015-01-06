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
	AlloArray is a multidimensional array.
	It is a pointer to data followed by meta-data to describe its type and layout.

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#ifndef INCLUDE_ALLO_ARRAY_H
#define INCLUDE_ALLO_ARRAY_H 1

#include "allocore/system/al_Config.h"
#include <limits.h> /* ULONG_MAX */


/*
 Maximum number of dimensions a array may represent
 To model higher dimensional spaces, use a nested array descriptor
 */
#define ALLO_ARRAY_MAX_DIMS (4)

#ifdef __cplusplus
extern "C" {
#endif


#pragma mark AlloTy
/**
	Unique identifiers for principal types
		(inspired by SDIF; higher bits represent semantics, lower bits represent size)
		(unlike SDIF, assumption is little-endian; and 'byte' is represented as AlloUInt8Ty)
*/
enum {
	/* type with no size */
	AlloVoidTy			= 0x0000,

	/* floating point numbers */
	AlloFloat32Ty		= 0x0004,
	AlloFloat64Ty		= 0x0008,

	/* signed integers */
	AlloSInt8Ty			= 0x0101,
	AlloSInt16Ty		= 0x0102,
	AlloSInt32Ty		= 0x0104,
	AlloSInt64Ty		= 0x0108,

	/* unsigned integers */
	AlloUInt8Ty			= 0x0201,
	AlloUInt16Ty		= 0x0202,
	AlloUInt32Ty		= 0x0204,
	AlloUInt64Ty		= 0x0208,

	/* structural types */
	AlloArrayTy			= 0x1A2C,	/* 2C == 44 bytes, sizeof AlloArray */

	/* pointer types */
	AlloPointer32Ty		= 0x2F04,
	AlloPointer64Ty		= 0x2F08
};


typedef uint16_t AlloTy;


/** Return the size, in bytes, for a given type
*/
static size_t allo_type_size(const AlloTy ty);

/** Return a human-readable string for a given type
*/
const char * allo_type_name(const AlloTy ty);

/** Converts any format value into a double in [0, 1].
	Useful for converting numeric formats.
*/
static double allo_type_tonumber(AlloTy ty, const char * ptr);

/** Converts a double in [0, 1] to a specified AlloTy.
	Useful for converting numeric formats.
*/
static void allo_type_fromnumber(AlloTy ty, double number, char * dst);


/** Description a dynamically-typed, multidimensional array */
typedef struct AlloArrayHeader {

	/** The type of data stored (see enums above) */
	AlloTy type;

	/**
		The number of values per cell
		Aka components, planecount, parts, rank, order...
	*/
	uint8_t components;

	/** The number of dimensions (actually should not be > ALLO_ARRAY_MAX_DIMS!) */
	uint8_t dimcount;

	/** The size of each dimension */
	uint32_t dim[ALLO_ARRAY_MAX_DIMS];

	/** The number of bytes between elements of that dimension */
	uint32_t stride[ALLO_ARRAY_MAX_DIMS];

} AlloArrayHeader;


/** Dynamically-typed, multidimensional array */
typedef struct AlloArray {
	/**
		The data encapsulated.
		First member of the struct to permit easy dereferencing without pointer adjustment.
		Anonymous union with 64-bit padding for portability
		Serialized data should be little-endian, but in-memory data should match to the native endianness
		Stride is necessary for arrays that are 4 or 8 byte aligned
	*/
	union{
		char * ptr;
		uint64_t pad;
	} data;

	/** The description of this data */
	AlloArrayHeader header;

} AlloArray;


/** Returns the number of elements (cells) in an array
*/
static uint32_t allo_array_elements(const AlloArray * arr);

/** Returns the total memory footprint, in bytes, based on the array header
*/
static size_t allo_array_size_from_header(const AlloArrayHeader * h);

/** Returns the total memory footprint, in bytes
*/
static size_t allo_array_size(const AlloArray * arr);


/** Set an array header, e.g. just after allocating
*/
void allo_array_setheader(AlloArray * dst, const AlloArrayHeader * src);


/** Set dimension attributes without modifying memory
*/
void allo_array_setdim1d(AlloArrayHeader * h, uint32_t nx);

/** Set dimension attributes without modifying memory
*/
void allo_array_setdim2d(AlloArrayHeader * h, uint32_t nx, uint32_t ny);

/** Set stride factors based on a specific byte alignment
*/
void allo_array_setstride(AlloArrayHeader * h, unsigned alignSize);


/** Checks if headers are equivalent.

@return 1 if they are equivalent, otherwise 0.
*/
int allo_array_equal_headers(const AlloArrayHeader * h1, const AlloArrayHeader * h2);


/** Allocate new memory based on current size in header
*/
void allo_array_allocate(AlloArray * arr);

/** Create a new array based on a header
*/
void allo_array_create(AlloArray * arr, const AlloArrayHeader *h);

/** Create a new 1D array
*/
void allo_array_create1d(
	AlloArray * arr,
	uint8_t components,
	AlloTy type,
	uint32_t dimx,
	size_t align
);

/** Create a new 2D array
*/
void allo_array_create2d(
	AlloArray * arr,
	uint8_t components,
	AlloTy type,
	uint32_t dimx,
	uint32_t dimy,
	size_t align
);

/** Adapt an array to another size
*/
void allo_array_adapt(AlloArray * arr, const AlloArrayHeader *h);

/** Adapt an array to 2D
*/
void allo_array_adapt2d(
	AlloArray * arr,
	uint8_t components,
	AlloTy type,
	uint32_t dimx,
	uint32_t dimy,
	size_t align
);

/** Set header attributes to zero
*/
void allo_array_header_clear(AlloArrayHeader *h);

/** Set all attributes, including header, to zero
*/
void allo_array_clear(AlloArray * arr);

/** Free memory
*/
void allo_array_free(AlloArray * arr);

/** Free memory and zero attributes
*/
void allo_array_destroy(AlloArray * arr);

/** Copy a array into another array
*/
void allo_array_copy(AlloArray *dst, AlloArray *src);



/* An extended array (wrapper)
*/
typedef struct AlloArrayWrapper {

	AlloArray array;

	/* The reference count */
	int refs;

} AlloArrayWrapper;

AlloArrayWrapper * allo_array_wrapper_new();

void allo_array_wrapper_free(AlloArrayWrapper *w);

void allo_array_wrapper_setup(AlloArrayWrapper *wrap);

void allo_array_wrapper_retain(AlloArrayWrapper *wrap);

void allo_array_wrapper_release(AlloArrayWrapper *wrap);



/*
 *
 ********* INLINE IMPLEMENTATION BELOW ***********
 *
 */
#pragma mark -------------------------------------

static inline size_t allo_type_size(AlloTy ty) {
	switch(ty) {
		case AlloUInt8Ty:		return sizeof(uint8_t);
		case AlloUInt16Ty:		return sizeof(uint16_t);
		case AlloUInt32Ty:		return sizeof(uint32_t);
		case AlloUInt64Ty:		return sizeof(uint64_t);
		case AlloSInt8Ty:		return sizeof(int8_t);
		case AlloSInt16Ty:		return sizeof(int16_t);
		case AlloSInt32Ty:		return sizeof(int32_t);
		case AlloSInt64Ty:		return sizeof(int64_t);
		case AlloFloat32Ty:		return sizeof(float);
		case AlloFloat64Ty:		return sizeof(double);
		case AlloArrayTy:		return sizeof(AlloArray);
		case AlloPointer32Ty:	return sizeof(int32_t);
		case AlloPointer64Ty:	return sizeof(int64_t);
		default:				return 0;
	}
}

static inline double allo_type_tonumber(AlloTy ty, const char * ptr) {
	switch(ty) {
		case AlloUInt8Ty:		return (double)(((uint8_t *)ptr)[0])/255.;	/* UCHAR_MAX */
		case AlloUInt16Ty:		return (double)(((uint8_t *)ptr)[0])/65535.;
		case AlloUInt32Ty:		return (double)(((uint8_t *)ptr)[0])/(double)0xffffffff;
		case AlloUInt64Ty:		return (double)(((uint8_t *)ptr)[0])/(double)ULONG_MAX;
		case AlloSInt8Ty:		return 0.5+(double)(((uint8_t *)ptr)[0])/255.;	/* UCHAR_MAX */
		case AlloSInt16Ty:		return 0.5+(double)(((uint8_t *)ptr)[0])/65535.;
		case AlloSInt32Ty:		return 0.5+(double)(((uint8_t *)ptr)[0])/(double)0xffffffff;
		case AlloSInt64Ty:		return 0.5+(double)(((uint8_t *)ptr)[0])/(double)ULONG_MAX;
		case AlloFloat32Ty:		return (double)(((float *)ptr)[0]);
		case AlloFloat64Ty:		return ((double *)ptr)[0];
		default:				return 0;
	}
}

static inline void allo_type_fromnumber(AlloTy ty, double number, char * dst) {
	switch(ty) {
		case AlloUInt8Ty:		*((uint8_t *)dst) = (uint8_t)(number * 255.); break;
		case AlloUInt16Ty:		*((uint16_t *)dst) = (uint16_t)(number * 65535.); break;
		case AlloUInt32Ty:		*((uint32_t *)dst) = (uint32_t)(number * (double)(0xffffffff)); break;
		case AlloUInt64Ty:		*((uint64_t *)dst) = (uint64_t)(number * (double)(ULONG_MAX)); break;

		case AlloSInt8Ty:		*((int8_t *)dst) = (int8_t)((number-0.5) * 255.); break;
		case AlloSInt16Ty:		*((int16_t *)dst) = (int16_t)((number-0.5) * 65535.); break;
		case AlloSInt32Ty:		*((int32_t *)dst) = (int32_t)((number-0.5) * (double)(0xffffffff)); break;
		case AlloSInt64Ty:		*((int64_t *)dst) = (int64_t)((number-0.5) * (double)(ULONG_MAX)); break;
		case AlloFloat32Ty:		*((float *)dst) = (float)number; break;
		case AlloFloat64Ty:		*((double *)dst) = number; break;
		default:				*dst = 0;
	}
}

static inline uint32_t allo_array_elements(const AlloArray * arr) {
	uint32_t i, elements = 1;
	for (i=0; i<arr->header.dimcount; i++)
		elements *= arr->header.dim[i];
	return elements;
}

static inline size_t allo_array_size_from_header(const AlloArrayHeader * h) {
	if(h->dimcount != 0){
		int idx = h->dimcount-1;
		return h->stride[idx] * h->dim[idx];
	}
	return 0;
}

static inline size_t allo_array_size(const AlloArray * arr) {
	return allo_array_size_from_header(&arr->header);
}


/* Multidimensional terminology:
	# Dimensions	0			1			2			3			4
	----------------------------------------------------------------------------
	Index						row			column		pillar		file
	Tensor						scalar		vector		matrix
	Sound						sample		time		channel		pattern
	Polytope		point		line		polygon		polyhedron	polychoron
	n-cube			point		line		square		cube		tesseract
	n-sphere					interval	circle		sphere		hypersphere
	Boundary		none		vertex		edge		face		cell
	Movement		position	velocity	accel.		jerk		snap
*/


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ALLO_H */
