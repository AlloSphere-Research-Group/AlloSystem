#include "allocore/graphics/al_BufferObject.hpp"

namespace al{

BufferObject::BufferObject(BufferType bufType, BufferUsage bufUsage)
:	mMapMode(READ_WRITE), mType(bufType), mUsage(bufUsage),
	mDataType(Graphics::FLOAT), mNumComps(0), mNumElems(0), mData(0)
{}

BufferObject::~BufferObject(){
	destroy();
}

int BufferObject::size() const {
	return mNumElems*mNumComps*Graphics::numBytes(mDataType);
}

void BufferObject::bufferType(BufferType v){ mType=v; }

void BufferObject::usage(BufferUsage v){ mUsage=v; }

void BufferObject::operator()(){
	//data(); onPointerFunc(); unbind();
	bind(); onPointerFunc();
}

void BufferObject::send(){
	(*this)();
}

void BufferObject::bind(){
	validate();
	glBindBuffer(mType, id());
}

void BufferObject::unbind() const {
	glBindBuffer(mType, 0);
}

#ifdef AL_GRAPHICS_USE_OPENGL
void BufferObject::mapMode(AccessMode v){ mMapMode=v; }

void * BufferObject::map(){
	bind();
	return glMapBuffer(mType, mMapMode);
}

bool BufferObject::unmap(){
	return glUnmapBuffer(mType)==GL_TRUE;
}
#endif

void BufferObject::resize(int numBytes){
	mData = NULL;
	mDataType = Graphics::BYTE;
	mNumElems = numBytes;
	mNumComps = 1;
	data();
}

void BufferObject::data(const void * src, Graphics::DataType dataType, int numElems, int numComps){
	mData = (void *)src;
	mDataType = dataType;
	mNumElems = numElems;
	mNumComps = numComps;
	data();
}

void BufferObject::data(Graphics::DataType dataType, int numElems, int numComps){
	data(0, dataType, numElems, numComps);
}

void BufferObject::data(){
	bind();
	glBufferData(mType, size(), mData, mUsage);
	unbind();
}

void BufferObject::onCreate(){
	glGenBuffers(1, (GLuint*)&mID);
	if(size() > 0){
		data();
		if(mSubData.size() > 0){
			bind();
			for(unsigned i=0; i<mSubData.size(); ++i){
				const SubData& sd = mSubData[i];
				glBufferSubData(mType, sd.offset, sd.size, sd.data);
			}
			unbind();
		}
	}
}

void BufferObject::onDestroy(){
	glDeleteBuffers(1, (GLuint*)&mID);
}

void BufferObject::print() const {
	printf("%s: %s %s (%d comps %d elems [%d bytes])\n", toString(mType), toString(mUsage), toString(mDataType), mNumComps, mNumElems, size());
}


VBO::VBO(BufferUsage usage)
:	BufferObject(ARRAY_BUFFER, usage)
{}

void VBO::enable(){ glEnableClientState(VERTEX_ARRAY); }
void VBO::disable(){ glDisableClientState(VERTEX_ARRAY); }
void VBO::onPointerFunc(){ glVertexPointer(mNumComps, mDataType, 0, 0); }



CBO::CBO(BufferUsage usage)
:	BufferObject(ARRAY_BUFFER, usage)
{}

void CBO::enable(){ glEnableClientState(COLOR_ARRAY); }
void CBO::disable(){ glDisableClientState(COLOR_ARRAY); }
void CBO::onPointerFunc(){ glColorPointer(mNumComps, mDataType, 0, 0); }



PBO::PBO(bool packMode, BufferUsage usage)
:	BufferObject(packMode ? PIXEL_PACK_BUFFER : PIXEL_UNPACK_BUFFER, usage)
{}

//void PBO::enable(){ glEnableClientState(ArrayType::Vertex); }
//void PBO::disable(){ glDisableClientState(ArrayType::Vertex); }
void PBO::onPointerFunc(){ glVertexPointer(mNumComps, mDataType, 0, 0); }



EBO::EBO(Graphics::Primitive prim, BufferUsage usage)
:	BufferObject(ELEMENT_ARRAY_BUFFER, usage), mPrim(prim), mStart(0), mEnd(0)
{}

EBO& EBO::primitive(Graphics::Primitive v){ mPrim=v; return *this; }
EBO& EBO::range(int start, int end){ mStart=start; mEnd=end; return *this; }

void EBO::onPointerFunc(){
	if(mEnd)	glDrawRangeElements(mPrim, mStart, mEnd, mNumElems, mDataType, 0);
	else		glDrawRangeElements(mPrim, 0, mNumElems, mNumElems, mDataType, 0);
}

#define CS(t) case BufferObject::t: return #t;
const char * toString(BufferObject::BufferType v){
	switch(v){
		CS(ARRAY_BUFFER) CS(ELEMENT_ARRAY_BUFFER) CS(PIXEL_PACK_BUFFER) CS(PIXEL_UNPACK_BUFFER)
		default: return "";
	}
}

const char * toString(BufferObject::BufferUsage v){
	switch(v){
		CS(STREAM_DRAW) CS(STATIC_DRAW) CS(DYNAMIC_DRAW)
		default: return "";
	}
}
#undef CS

} // al::
