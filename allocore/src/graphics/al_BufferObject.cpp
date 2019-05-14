#include "allocore/graphics/al_BufferObject.hpp"

namespace al{

BufferObject::BufferObject(BufferType bufType, BufferUsage bufUsage)
:	mType(bufType), mUsage(bufUsage)
{}

BufferObject::~BufferObject(){
	destroy();
}

int BufferObject::size() const {
	return mNumElems*mNumComps*Graphics::numBytes(mDataType);
}

BufferObject& BufferObject::bufferType(BufferType v){ mType=v; return update(); }

BufferObject& BufferObject::usage(BufferUsage v){ mUsage=v; return update(); }

void BufferObject::operator()(){
	//data(); onAction(); unbind();
	bind(); onAction();
}

BufferObject& BufferObject::update(){
	mDataChanged = true;
	mSubDataChanged = true;
	return *this;
}

void BufferObject::bind(){
	validate();
	glBindBuffer(mType, id());
	if(mDataChanged){
		mDataChanged = false;
		glBufferData(mType, size(), mData, mUsage);
	}
	if(mSubDataChanged){
		mSubDataChanged = false;
		for(const auto& sd : mSubData){
			glBufferSubData(mType, sd.offset, sd.size, sd.data);
		}
	}
}

void BufferObject::unbind() const {
	glBindBuffer(mType, 0);
}

BufferObject& BufferObject::resize(int numBytes){
	// does remote resize without data upload
	return data((const char *)NULL, numBytes, 1);
}

BufferObject& BufferObject::data(const void * src, Graphics::DataType dataType, int numElems, int numComps){
	mData = (void *)src;
	mDataType = dataType;
	mNumElems = numElems;
	mNumComps = numComps;
	mDataChanged = true;
	return *this;
}

BufferObject& BufferObject::data(Graphics::DataType dataType, int numElems, int numComps){
	return data(0, dataType, numElems, numComps);
}

void BufferObject::onCreate(){
	glGenBuffers(1, (GLuint*)&mID);
	if(size() > 0) update(); // handle potential GL context change
}

void BufferObject::onDestroy(){
	glDeleteBuffers(1, (GLuint*)&mID);
}

void BufferObject::print() const {
	printf("%s: %s %s (%d comps %d elems [%.1f kB])\n", toString(mType), toString(mUsage), toString(mDataType), mNumComps, mNumElems, size()/1000.);
}

#ifdef AL_GRAPHICS_SUPPORTS_MAP_BUFFER
void BufferObject::mapMode(AccessMode v){ mMapMode=v; }

void * BufferObject::map(){
	bind();
	return glMapBuffer(mType, mMapMode);
}

bool BufferObject::unmap(){
	return glUnmapBuffer(mType)==GL_TRUE;
}
#endif


VBO::VBO(BufferUsage usage)
:	BufferObject(ARRAY_BUFFER, usage)
{}

void VBO::onAction(){
	if(true){ // using shader attributes
		#ifdef AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
		glVertexAttribPointer(VERTEX_ARRAY, mNumComps, mDataType, GL_FALSE, 0, 0);
		#endif
	} else { // not using shader attributes
		#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
		glVertexPointer(mNumComps, mDataType, 0, 0);
		#endif
	}
}

CBO::CBO(BufferUsage usage)
:	BufferObject(ARRAY_BUFFER, usage)
{}

EBO::EBO(Graphics::Primitive prim, BufferUsage usage)
:	BufferObject(ELEMENT_ARRAY_BUFFER, usage), mPrim(prim)
{}

EBO& EBO::primitive(Graphics::Primitive v){ mPrim=v; return *this; }
EBO& EBO::count(int v){ mCount=v; return *this; }
EBO& EBO::range(int start, int end){ mStart=start; mEnd=end; return *this; }

#ifdef AL_GRAPHICS_SUPPORTS_DRAW_RANGE
void EBO::onAction(){
	int Ne = mCount<0 ? mNumElems+mCount+1 : mCount;
	if(mEnd)	glDrawRangeElements(mPrim, mStart, mEnd, Ne, mDataType, 0);
	else		glDrawRangeElements(mPrim, 0, mNumElems-1, Ne, mDataType, 0);
}
#else
void EBO::onAction(){
	int Ne = mCount<0 ? mNumElems+mCount+1 : mCount;
	glDrawElements(mPrim, Ne, mDataType, 0);
}
#endif

#ifdef AL_GRAPHICS_USE_PROG_PIPELINE
void VBO::enable(){ glEnableVertexAttribArray(VERTEX_ARRAY); }
void VBO::disable(){ glDisableVertexAttribArray(VERTEX_ARRAY); }
//void VBO::onAction(){ glVertexAttribPointer(VERTEX_ARRAY, mNumComps, mDataType, GL_FALSE, 0, 0); }

void CBO::enable(){ glEnableVertexAttribArray(COLOR_ARRAY); }
void CBO::disable(){ glDisableVertexAttribArray(COLOR_ARRAY); }
void CBO::onAction(){ glVertexAttribPointer(COLOR_ARRAY, mNumComps, mDataType, GL_FALSE, 0, 0); }
#else
void VBO::enable(){ glEnableClientState(VERTEX_ARRAY); }
void VBO::disable(){ glDisableClientState(VERTEX_ARRAY); }
//void VBO::onAction(){ glVertexPointer(mNumComps, mDataType, 0, 0); }

void CBO::enable(){ glEnableClientState(COLOR_ARRAY); }
void CBO::disable(){ glDisableClientState(COLOR_ARRAY); }
void CBO::onAction(){ glColorPointer(mNumComps, mDataType, 0, 0); }
#endif

#ifdef AL_GRAPHICS_SUPPORTS_PBO
PBO::PBO(bool packMode, BufferUsage usage)
:	BufferObject(packMode ? PIXEL_PACK_BUFFER : PIXEL_UNPACK_BUFFER, usage)
{}

//void PBO::enable(){ glEnableClientState(ArrayType::Vertex); }
//void PBO::disable(){ glDisableClientState(ArrayType::Vertex); }
void PBO::onAction(){ glVertexPointer(mNumComps, mDataType, 0, 0); }
#endif


#define CS(t) case BufferObject::t: return #t;
const char * toString(BufferObject::BufferType v){
	switch(v){
		CS(ARRAY_BUFFER) CS(ELEMENT_ARRAY_BUFFER) CS(PIXEL_PACK_BUFFER) CS(PIXEL_UNPACK_BUFFER)
		default: return "";
	}
}

const char * toString(BufferObject::BufferUsage v){
	switch(v){
		CS(STATIC_DRAW) CS(DYNAMIC_DRAW)
		#ifdef AL_GRAPHICS_SUPPORTS_STREAM_DRAW
			CS(STREAM_DRAW)
		#endif
		default: return "";
	}
}
#undef CS

} // al::
