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

#define INCLUDE_ALLO_TYPES_VERSION 0.001

#if defined(WIN32) || defined(__WINDOWS_MM__)
	#define ALLO_WIN32
	#include <windows.h>
	#include "pstdint.h"
#elif defined( __APPLE__ ) && defined( __MACH__ )
	#define ALLO_OSX
	#include <Carbon/Carbon.h>
	#include <stdint.h>
#else
	#define ALLO_LINUX
	#include <stdint.h>
#endif

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
	AlloPointer64Ty		= 0x2F08,
};

typedef uint16_t AlloTy;

#pragma mark AlloLattice
/*
	AlloLattice is a general purpose descriptor of data in a regular dimensional layout
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
		The step advancement for each dimension (loop pointer increment) 
		stride is typically the dim * type size * components
	*/
	uint32_t stride[ALLO_LATTICE_MAX_DIMS];
	
	/* 
		TODO:	should we store whether this is row, column major etc? 
				or should we enforce row-major and offer transpose operators?
				(row, column, pillar, file)
	
		LJP:
					0			1			2			3			4
		Specifier	row			column		pillar		file
		Mathematic	element		array		matrix
		Audio		sample		time		channel		pattern
		Visual		pixel		line		rect		time
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
		void * ptr;
		uint64_t pad;
	} data;
	
	/*
		The description of this data
	*/
	AlloLatticeHeader header;
		
} AlloLattice;

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
inline size_t allo_type_size(const AlloTy ty) {
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
inline uint32_t allo_lattice_elements(const AlloLattice * lat) {
	uint32_t i, elements = 1;
	for (i=0; i<lat->header.dimcount; i++) 
		elements *= lat->header.dim[i];
	return elements;
}

/*
	Return the memory footprint of a lattice
*/
inline size_t allo_lattice_size(const AlloLattice * lat) {
	size_t i; 
	size_t sz = 1;
	for (i=0; i < lat->header.dimcount; i++) {
		//printf("%i %i %i\n", lat->header.stride[i], i, sz);
		sz *= lat->header.stride[i];
	}
	return sz;
}

/*
	Set a lattice header, e.g. just after allocating
*/
inline void allo_lattice_setheader(AlloLattice * lat, const AlloLatticeHeader * header) {
	memcpy(&lat->header, header, sizeof(AlloLatticeHeader));
}

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_ALLO_H */
