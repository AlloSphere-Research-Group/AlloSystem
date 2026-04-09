#include <string.h> // memcpy
#include <stdlib.h> // malloc
#include "allocore/types/al_Array.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * allo_type_name(AlloTy t){
	switch(t){
	case AlloUInt8Ty:		return "uint8_t";
	case AlloUInt16Ty:		return "uint16_t";
	case AlloUInt32Ty:		return "uint32_t";
	case AlloUInt64Ty:		return "uint64_t";
	case AlloSInt8Ty:		return "int8_t";
	case AlloSInt16Ty:		return "int16_t";
	case AlloSInt32Ty:		return "int32_t";
	case AlloSInt64Ty:		return "int64_t";
	case AlloFloat32Ty:		return "float";
	case AlloFloat64Ty:		return "double";
	case AlloArrayTy:		return "AlloArray";
	case AlloPointer32Ty:	return "int32_t";
	case AlloPointer64Ty:	return "int64_t";
	default:				return "";
	}
}

int allo_array_equal_headers(const AlloArrayHeader * h1, const AlloArrayHeader * h2){
	if(
		(h1->components == h2->components) &&
		(h1->type == h2->type) &&
		(h1->dimcount == h2->dimcount)
	){
		unsigned i;
		for(i=0; i<h1->dimcount; ++i){
			if( (h1->dim[i] != h2->dim[i]) || (h1->stride[i] != h2->stride[i]) )
				return 0;
		}
		return 1; /* == true */
	}
	else{
		return 0;
	}
}

void allo_array_setheader(AlloArray * dst, const AlloArrayHeader * src){
	memcpy(&dst->header, src, sizeof(AlloArrayHeader));
}

void allo_array_resetdims(AlloArrayHeader * h, unsigned startDim){
	unsigned i;
	for(i=startDim; i<ALLO_ARRAY_MAX_DIMS; i++)
		h->dim[i] = i!=0;
}

void allo_array_setdim1d(AlloArrayHeader * h, uint32_t nx){
	h->dimcount	= 1;
	h->dim[0] = nx;
	allo_array_resetdims(h, 1);
}

void allo_array_setdim2d(AlloArrayHeader * h, uint32_t nx, uint32_t ny){
	h->dimcount	= 2;
	h->dim[0] = nx;
	h->dim[1] = ny;
	allo_array_resetdims(h, 2);
}

void allo_array_setdim3d(AlloArrayHeader * h, uint32_t nx, uint32_t ny, uint32_t nz){
	h->dimcount	= 3;
	h->dim[0] = nx;
	h->dim[1] = ny;
	h->dim[2] = nz;
	allo_array_resetdims(h, 3);
}

void allo_array_setdimNd(AlloArrayHeader * h, uint32_t * n, uint8_t numDims){
	if(numDims > ALLO_ARRAY_MAX_DIMS) numDims = ALLO_ARRAY_MAX_DIMS;
	h->dimcount	= numDims;
	uint8_t i;
	for(i=0; i<numDims; i++) h->dim[i] = n[i];
	allo_array_resetdims(h, numDims);
}

void allo_array_setstride(AlloArrayHeader * h, unsigned alignSize){
	unsigned typeSize = allo_type_size(h->type);
	unsigned numDims = h->dimcount;
	h->stride[0] = h->components * typeSize;

	if(numDims>1){
		h->stride[1] = h->stride[0] * h->dim[0]; /* compute ideal row stride amount */

		/* Pad rows to make multiple of alignment */
		if(alignSize > 1){
			unsigned remain = h->stride[1] % alignSize;		/* compute pad bytes */
			if(remain){ h->stride[1] += alignSize - remain;}/* add pad bytes (if any) */
		}

		unsigned i=2;
		for(; i<numDims; ++i){ h->stride[i] = h->stride[i-1] * h->dim[i-1]; }
	}
}

void allo_array_header_clear(AlloArrayHeader * h){
	memset(h, 0, sizeof(AlloArrayHeader));
}

void allo_array_clear(AlloArray * a){
	allo_array_header_clear(&a->header);
	a->data.ptr = NULL;
}

void allo_array_free(AlloArray * a){
	if(NULL != a->data.ptr) free(a->data.ptr);
	a->data.ptr = NULL;
}

void allo_array_destroy(AlloArray * a){
	if(NULL != a->data.ptr){
		allo_array_free(a);
		allo_array_clear(a);
	}
}

void allo_array_allocate(AlloArray * a){
	a->data.ptr = (char *)calloc(1, allo_array_size(a));
}

void allo_array_create(AlloArray * a, const AlloArrayHeader * h){
	allo_array_destroy(a);
	allo_array_setheader(a, h);
	allo_array_allocate(a);
}

void allo_array_create1d(
	AlloArray * arr,
	uint8_t components,
	AlloTy type,
	uint32_t dimx,
	size_t align
){
	AlloArrayHeader header;
	header.type = type;
	header.components = components;
	allo_array_setdim1d(&header, dimx);
	allo_array_setstride(&header, align);
	allo_array_create(arr, &header);
}

void allo_array_create2d(
	AlloArray * arr,
	uint8_t components,
	AlloTy type,
	uint32_t dimx,
	uint32_t dimy,
	size_t align
){
	AlloArrayHeader header;
	header.type = type;
	header.components = components;
	allo_array_setdim2d(&header, dimx, dimy);
	allo_array_setstride(&header, align);
	allo_array_create(arr, &header);
}

void allo_array_adapt(AlloArray * a, const AlloArrayHeader * h){
	if(!allo_array_equal_headers(&a->header, h)){
		allo_array_create(a, h);
	}
}

void allo_array_adapt2d(
	AlloArray * arr,
	uint8_t components,
	AlloTy type,
	uint32_t dimx,
	uint32_t dimy,
	size_t align
){
	AlloArrayHeader header;
	header.type = type;
	header.components = components;
	allo_array_setdim2d(&header, dimx, dimy);
	allo_array_setstride(&header, align);
	allo_array_adapt(arr, &header);
}

void allo_array_copy(AlloArray * dst, AlloArray * src){
	allo_array_adapt(dst, &(src->header));
	memcpy(dst->data.ptr, src->data.ptr, allo_array_size(src));
}




AlloArrayWrapper * allo_array_wrapper_new(){
	return (AlloArrayWrapper *)malloc(sizeof(AlloArrayWrapper));
}

void allo_array_wrapper_free(AlloArrayWrapper * w){
	free(w);
}

void allo_array_wrapper_setup(AlloArrayWrapper * w){
	allo_array_clear(&w->array);
	w->refs = 0;
}

void allo_array_wrapper_retain(AlloArrayWrapper * w){
	w->refs++;
}

void allo_array_wrapper_release(AlloArrayWrapper * w){
	w->refs--;
	if(w->refs <= 0) {
		allo_array_destroy(&w->array);
		w->refs = 0;
		allo_array_wrapper_free(w);
	}
}

#ifdef __cplusplus
} /*extern "C"*/
#endif
