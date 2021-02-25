#include <cstring> // memcpy, memset
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

uint32_t Array::alignment() const {
	uint32_t i = stride(1);	// row stride
	if (i % 2 > 0) return 1;
	if (i % 4 > 0) return 2;
	if (i % 8 > 0) return 4;
	return 8;
}

bool Array::isFormat(const AlloArrayHeader& h2) const {
	return allo_array_equal_headers(&header, &h2);
}

void Array::configure(const AlloArrayHeader& h2) {
	allo_array_setheader(this, &h2);
}

void Array::dataCalloc() { allo_array_allocate(this); }

void Array::dataFree() { allo_array_free(this); }

void Array::deriveStride(AlloArrayHeader& h, size_t alignSize) {
	allo_array_setstride(&h, alignSize);
}

void Array::format(const AlloArrayHeader& h2) {
	if(!isFormat(h2)) {
		if(size() != allo_array_size_from_header(&h2)){
			dataFree();
			configure(h2);
			dataCalloc();
		}
		else{
			configure(h2);
			zero();
		}
	}
}

void Array::format(int comps, AlloTy ty, uint32_t dimx) {
	formatAligned(comps, ty, dimx, AL_ARRAY_DEFAULT_ALIGNMENT);
}

void Array::format(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy) {
	formatAligned(comps, ty, dimx, dimy, AL_ARRAY_DEFAULT_ALIGNMENT);
}

void Array::format(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz) {
	formatAligned(comps, ty, dimx, dimy, dimz, AL_ARRAY_DEFAULT_ALIGNMENT);
}

void Array::formatAligned(int comps, AlloTy ty, uint32_t dimx, size_t align) {
	uint32_t dims[] = {dimx};
	formatAlignedGeneral(comps, ty, dims,1, align);
}

void Array::formatAligned(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, size_t align) {
	uint32_t dims[] = {dimx,dimy};
	formatAlignedGeneral(comps, ty, dims,2, align);
}

void Array::formatAligned(int comps, AlloTy ty, uint32_t dimx, uint32_t dimy, uint32_t dimz, size_t align) {
	uint32_t dims[] = {dimx,dimy,dimz};
	formatAlignedGeneral(comps, ty, dims,3, align);
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

void Array::zero(){
	if(hasData())
		memset(data.ptr, 0, size());
}

void Array::print(FILE * fp) const {
	/*printf("Array %p type %s components %d %d-D: ( ", this, allo_type_name(type()), components(), dimcount());
	for (int i=0; i<dimcount(); i++) printf("%d(stride %d) ", dim(i), stride(i));
	printf(") %d bytes, data: %p)\n", int(size()), data.ptr);*/
	int Ndim = dimcount();
	fprintf(fp,"Array %p:\n", this);
	fprintf(fp,"  comp:   %d %s\n", components(), allo_type_name(type()));
	fprintf(fp,"  dim:    ");
	for(int i=0; i<Ndim; i++){
		fprintf(fp,"%u%s", dim(i), (i!=(Ndim-1)) ? " x " : "");
	}
	fprintf(fp," (%dD)\n", Ndim);
	fprintf(fp,"  stride: ");
	for(int i=0; i<Ndim; i++){
		fprintf(fp,"%u%s", stride(i), (i!=(Ndim-1)) ? " x " : "");
	}
	fprintf(fp," bytes\n");
	fprintf(fp,"  data:   %p, %u bytes\n", data.ptr, (unsigned)size());
}

} // al::
