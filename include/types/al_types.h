/*
 *  allo.h
 *  A set of definitions for data interchange amenable to a C ABI.
 *
 *  AlloSphere, Media Arts & Technology, UCSB
 *
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

#ifndef INCLUDE_ALLO_TYPES_H
#define INCLUDE_ALLO_TYPES_H 1

#include "system/al_Config.h"
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#pragma mark AlloTy
/*
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
	AlloLatticeTy		= 0x1A2C,	/* 2C == 44 bytes, sizeof AlloLattice */
	/* AlloGraphTy		= 0x1B00, */
	
	/* pointer types */
	AlloPointer32Ty		= 0x2F04,
	AlloPointer64Ty		= 0x2F08
};

typedef uint16_t AlloTy;

#pragma mark AlloLattice
/*
	AlloLattice is a multidimensional array.
	It is a pointer to data followed by meta-data to describe its type and layout.
	
	Separating the data from the header meta-data allows logic to performed on layouts without data 
		- a specific layout can be defined and checked against
	
*/
/*
	Maximum number of dimensions a lattice may represent
	To model higher dimensional spaces, use a nested lattice descriptor
*/
#define ALLO_LATTICE_MAX_DIMS (4)
typedef struct {
	
	/* The type of data stored (see enums above) */
	AlloTy type;
	
	/*
		The number of values per cell
		Aka components, planecount, parts, rank, order...
	*/
	uint8_t components;
	
	/* The number of dimensions (actually should not be > ALLO_LATTICE_MAX_DIMS!) */
	uint8_t dimcount;
	
	/* The size of each dimension */
	uint32_t dim[ALLO_LATTICE_MAX_DIMS];
	
	/* 
		# of bytes between elements of that dimension
	*/
	uint32_t stride[ALLO_LATTICE_MAX_DIMS];
	
	/* 
		TODO:	should we store whether this is row, column major etc? 
				or should we enforce row-major and offer transpose operators?
				(row, column, pillar, file)
	
		LJP:
					0			1			2			3			4
		Specifier	row			column		pillar		file
		Tensor		scalar		vector		matrix		
		Sound		sample		time		channel		pattern
		Polytope	point		line		polygon		polyhedron	polychoron
		n-Hypercube	point		line		square		cube		tesseract
		Boundary	none		vertex		edge		face		cell
		Movement	position	velocity	accel.		jerk		snap
	*/
	
} AlloLatticeHeader;

typedef struct {
	/*
		The data encapsulated.
		First member of the struct to permit easy dereferencing without pointer adjustment.
		Anonymous union with 64-bit padding for portability
		Serialized data should be little-endian, but in-memory data should match to the native endianness
		Stride is necessary for lattices that are 4 or 8 byte aligned
	*/
	union{
		char * ptr;
		uint64_t pad;
	} data;
	
	/*
		The description of this data
	*/
	AlloLatticeHeader header;
		
} AlloLattice;

/*
	An extended lattice (wrapper)
*/
typedef struct AlloLatticeWrapper {

	AlloLattice lattice;
	
	/*
		The reference count:
	*/
	int refs;
	
} AlloLatticeWrapper;

#pragma mark AlloGraph
/*
	AlloGraph type.
	Stores a set of nodes (vertices) and a set of arcs (edges) as node pairs.
	Graphs may be directed or undirected. 
	Graphs can be translated into 2D adjacency matrices, of dimensions NxN (where N = no. of nodes)
*/
typedef struct {
	uint16_t type;
	union{
		void * ptr;
		uint64_t pad;
	} data;
} AlloGraphNode;

typedef struct {
	uint16_t a, b;
} AlloGraphEdge;

typedef struct {
	/* the number of nodes */
	uint16_t nodecount;
	
	/* the number of edges */
	uint16_t edgecount; 
	
	AlloGraphNode * nodes;
	AlloGraphEdge * edges;

} AlloGraph;

#pragma mark -
/*
********** INLINE IMPLEMENTATION BELOW ***********
*/

/*
	Return the size for a given type
*/
static inline size_t allo_type_size(const AlloTy ty) {
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
		case AlloLatticeTy:		return sizeof(AlloLattice);
		//case AlloGraphTy:		return sizeof(AlloGraph);
		case AlloPointer32Ty:	return sizeof(int32_t);
		case AlloPointer64Ty:	return sizeof(int64_t);
		default:				return 0;
	}
}

/*
	Return the number of elements (cells) in a lattice
*/
static inline uint32_t allo_lattice_elements(const AlloLattice * lat) {
	uint32_t i, elements = 1;
	for (i=0; i<lat->header.dimcount; i++) 
		elements *= lat->header.dim[i];
	return elements;
}

/*
	Return the memory footprint of a lattice
*/
static inline size_t allo_lattice_size(const AlloLattice * lat) {
	int idx = lat->header.dimcount-1;
	return lat->header.stride[idx] * lat->header.dim[idx];
}

/*
	Set a lattice header, e.g. just after allocating
*/
static inline void allo_lattice_setheader(AlloLattice * lat, const AlloLatticeHeader * header) {
	memcpy(&lat->header, header, sizeof(AlloLatticeHeader));
}

/*
	Set stride factors based on a specific byte alignment
*/
static inline void allo_lattice_setstride(AlloLatticeHeader * h, unsigned alignSize){
	unsigned typeSize = allo_type_size(h->type);
	unsigned numDims = h->dimcount;
	h->stride[0] = h->components * typeSize;
	
	if(numDims>1){
		h->stride[1] = h->stride[0] * h->dim[0];		// compute ideal row stride amount
		unsigned remain = h->stride[1] % alignSize;		// compute pad bytes
		if(remain){ h->stride[1] += alignSize - remain;}// add pad bytes (if any)
		
		unsigned i=2;
		for(; i<numDims; ++i){ h->stride[i] = h->stride[i-1] * h->dim[i-1]; }
	}
}

static inline int allo_lattice_equal_headers(AlloLatticeHeader *h1, AlloLatticeHeader *h2) {
	int equiv =	h1->components == h2->components && 
				h1->type == h2->type && 
				h1->dimcount == h2->dimcount;
					
	for(int i=0; i < h1->dimcount; i++) {
		equiv = equiv && h1->dim[i] == h2->dim[i];
		equiv = equiv && h1->stride[i] == h2->stride[i];
	}

	return equiv;
}


static inline void allo_lattice_header_clear(AlloLatticeHeader *h) {
	memset(h, '\0', sizeof(AlloLatticeHeader));
}

static inline void allo_lattice_clear(AlloLattice *lat) {
	allo_lattice_header_clear( &(lat->header) );
	lat->data.ptr = 0;
}

static inline void allo_lattice_destroy(AlloLattice *lat) {
	if(lat->data.ptr) {
		if(lat->data.ptr) {
			free(lat->data.ptr);
			allo_lattice_clear(lat);
		}
	}
}

static inline void allo_lattice_create(AlloLattice *lat, AlloLatticeHeader *h) {
	allo_lattice_destroy(lat);
	allo_lattice_setheader(lat, h);
	lat->data.ptr = (char *)calloc(1, allo_lattice_size(lat));
}

static inline void allo_lattice_create1d(
	AlloLattice *lat, 
	uint8_t components, 
	AlloTy type, 
	uint32_t dimx, 
	size_t align
) {
	AlloLatticeHeader header;
	header.type = type;
	header.components = components;
	header.dimcount = 1;
	header.dim[0] = dimx;
	allo_lattice_setstride(&header, align);
	allo_lattice_create(lat, &header);
}

static inline void allo_lattice_create2d(
	AlloLattice *lat, 
	uint8_t components, 
	AlloTy type, 
	uint32_t dimx, 
	uint32_t dimy, 
	size_t align
) {
	AlloLatticeHeader header;
	header.type = type;
	header.components = components;
	header.dimcount = 2;
	header.dim[0] = dimx;
	header.dim[1] = dimy;
	allo_lattice_setstride(&header, align);
	allo_lattice_create(lat, &header);
}


/*
	Adapt a latticle to another size
*/
static inline void allo_lattice_adapt(AlloLattice *lat, AlloLatticeHeader *h) {
	if(! allo_lattice_equal_headers( &(lat->header), h)) {
		allo_lattice_create(lat, h);
	}
}

static inline void allo_lattice_adapt2d(
	AlloLattice *lat, 
	uint8_t components, 
	AlloTy type, 
	uint32_t dimx, 
	uint32_t dimy, 
	size_t align
) {
	AlloLatticeHeader header;
	header.type = type;
	header.components = components;
	header.dimcount = 2;
	header.dim[0] = dimx;
	header.dim[1] = dimy;
	allo_lattice_setstride(&header, align);
	allo_lattice_adapt(lat, &header);
}

/*
	Copy a lattice into another lattice
*/
static inline void allo_lattice_copy(AlloLattice *dst, AlloLattice *src){
	allo_lattice_adapt(dst, &(src->header));
	memcpy(dst->data.ptr, src->data.ptr, allo_lattice_size(src));
}


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ALLO_H */
