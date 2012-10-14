#include <stdio.h>
#include "allocore/types/al_Array.hpp"

namespace al{

Array::Array(){
	data.ptr = NULL;
	header.type= 0;
	header.components = 1;
	header.dimcount = 0;
	for(int i=0; i<ALLO_ARRAY_MAX_DIMS; ++i) header.dim[i]=0;
}

Array::Array(const AlloArray& cpy){
	data.ptr = 0;
    (*this) = cpy;
}
Array::Array(const Array& cpy) {
	data.ptr = 0;
    (*this) = cpy;
}
Array::Array(const AlloArrayHeader& h2){
	allo_array_clear(this);
	format(h2);
}

Array::Array(int comps, AlloTy ty, uint32_t dimx){
	allo_array_clear(this);
	format(comps, ty, dimx);
}

Array::Array(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy){
	allo_array_clear(this);
	format(comps, ty, dimx, dimy);
}

Array::Array(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz){
	allo_array_clear(this);
	format(comps, ty, dimx, dimy, dimz);
}

Array::~Array(){ dataFree(); }

Array& Array::operator= (const Array& cpy) {
	if(&cpy != this){
		format(cpy.header);
		if (cpy.data.ptr) {
			memcpy(data.ptr, cpy.data.ptr, size());
		}
	}
	return *this;
}

Array& Array::operator= (const AlloArray& cpy) {
	if(&cpy != this){
		format(cpy.header);
		if (cpy.data.ptr) {
			memcpy(data.ptr, cpy.data.ptr, size());
		}
	}
	return *this;
}

bool Array::isFormat(const AlloArrayHeader& h2) const {
	for(int i=0; i<header.dimcount; ++i){
		if( (header.dim[i] != h2.dim[i]) || (header.stride[i] != h2.stride[i]) )
			return false;
	}
	return (header.components == h2.components)
		&& (header.type == h2.type)
		&& (header.dimcount == h2.dimcount);
}

void Array::configure(const AlloArrayHeader& h2) {
	header.type = h2.type;
	header.components = h2.components;
	header.dimcount = h2.dimcount;
	for(int i=0; i < ALLO_ARRAY_MAX_DIMS; ++i) {
		if (i < header.dimcount) {
			header.dim[i] = h2.dim[i];
			header.stride[i] = h2.stride[i];
		} else {
			header.dim[i] = 1;
			header.stride[i] = h2.stride[i-1];
		}
	}
}

void Array::format(const AlloArrayHeader& h2) {
	if(!isFormat(h2)) {
		dataFree();
		configure(h2);
		dataCalloc();
	}
}

void Array::formatAlignedGeneral(int comps, AlloTy ty, uint32_t * dims, int numDims, size_t align) {
	if(numDims > ALLO_ARRAY_MAX_DIMS) numDims = ALLO_ARRAY_MAX_DIMS;
	AlloArrayHeader hh;
	hh.type = ty;
	hh.components = comps;
	hh.dimcount = numDims;
	int i=0;
	for(; i<numDims; ++i)				hh.dim[i] = dims[i];
	for(; i<ALLO_ARRAY_MAX_DIMS; ++i)	hh.dim[i] = 0;
	deriveStride(hh, align);
	format(hh);
}

void Array::print() const {
	printf("Array %p type %s components %d %d-D: ( ", this, allo_type_name(type()), components(), dimcount());
	for (int i=0; i<dimcount(); i++) printf("%d(stride %d) ", dim(i), stride(i));
	printf(") %d bytes, data: %p)\n", int(size()), data.ptr);
}

} // al::
