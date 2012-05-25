#ifndef INCLUDE_AL_GRAPHICS_BUFFEROBJECT_HPP
#define INCLUDE_AL_GRAPHICS_BUFFEROBJECT_HPP

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
	GPU buffer object helper

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

/// Generic buffer object

/**
Vertex Buffer Objects (VBOs) create buffer memory for vertex attributes in 
high-performance memory (in contrast, Vertex Arrays store buffer memory in the 
client CPU, incurring the overhead of data transfer). If the buffer object is 
used to store pixel data, it is called Pixel Buffer Object (PBO).

VBOs provide an interface to access these buffers in a similar fashion to vertex 
arrays. Hints of 'target' and 'mode' help the implementation determine whether 
to use system, AGP or video memory. 

Unlike display lists, the data in vertex buffer object can be read and updated 
by mapping the buffer into client's memory space.

Another important advantage of VBO is sharing the buffer objects with many 
clients, like display lists and textures. Since VBO is on the server's side, 
multiple clients will be able to access the same buffer with the corresponding 
identifier.
*/
class BufferObject : public GPUObject {
public:

	enum AccessMode{
		READ_ONLY				= GL_READ_ONLY,
		WRITE_ONLY				= GL_WRITE_ONLY, 
		READ_WRITE				= GL_READ_WRITE
	};

	enum ArrayType{
		VERTEX_ARRAY			= GL_VERTEX_ARRAY,
		NORMAL_ARRAY			= GL_NORMAL_ARRAY,
		COLOR_ARRAY				= GL_COLOR_ARRAY,
		INDEX_ARRAY				= GL_INDEX_ARRAY,
		TEXTURE_COORD_ARRAY		= GL_TEXTURE_COORD_ARRAY,
		EDGE_FLAG_ARRAY			= GL_EDGE_FLAG_ARRAY
	};

	enum BufferType{
		ARRAY_BUFFER			= GL_ARRAY_BUFFER,
		ELEMENT_ARRAY_BUFFER	= GL_ELEMENT_ARRAY_BUFFER,
		PIXEL_PACK_BUFFER		= GL_PIXEL_PACK_BUFFER,			/**< Transfer to PBO */
		PIXEL_UNPACK_BUFFER		= GL_PIXEL_UNPACK_BUFFER		/**< Transfer from PBO */
	};


	/*
	"Static" means the data in VBO will not be changed, "Dynamic" means the data
	will be changed frequently, and "Stream" means the data will be changed every
	frame. "Draw" means the data will be sent to the GPU from the application,
	"Read" means the data will be read by the application from the GPU, and "Copy"
	means the data will be copied internally on the GPU.
	*/
	enum BufferUsage{
		STREAM_DRAW		= GL_STREAM_DRAW,
//		STREAM_READ		= GL_STREAM_READ,
//		STREAM_COPY		= GL_STREAM_COPY,
		STATIC_DRAW		= GL_STATIC_DRAW,
//		STATIC_READ		= GL_STATIC_READ,
//		STATIC_COPY		= GL_STATIC_COPY,
		DYNAMIC_DRAW	= GL_DYNAMIC_DRAW,
//		DYNAMIC_READ	= GL_DYNAMIC_READ,
//		DYNAMIC_COPY	= GL_DYNAMIC_COPY
	};

	enum DataType{
		BYTE			= GL_BYTE,
		UNSIGNED_BYTE	= GL_UNSIGNED_BYTE,
		SHORT			= GL_SHORT,
		UNSIGNED_SHORT	= GL_UNSIGNED_SHORT,
		INT				= GL_INT,
		UNSIGNED_INT	= GL_UNSIGNED_INT,
		FLOAT			= GL_FLOAT,
		BYTES_2			= GL_2_BYTES,                  
		BYTES_3			= GL_3_BYTES,
		BYTES_4			= GL_4_BYTES,
		DOUBLE			= GL_DOUBLE,
		UNKNOWN
	};

	template<class T> DataType asDataType();	///< Get DataType associated with built in type
	int numBytes(DataType type);				///< Get size of DataType, in bytes

	BufferObject(BufferType bufType, BufferUsage bufUsage)
	:	mMapMode(READ_WRITE), mType(bufType), mUsage(bufUsage),
		mDataType(UNKNOWN), mNumComps(0), mNumElems(0), mData(0)
	{}
	
	virtual ~BufferObject(){ destroy(); }

	void bufferType(BufferType v){ mType=v; }
	void mapMode(AccessMode v){ mMapMode=v; }
	
	void operator()(){
		//data(); onPointerFunc(); unbind();
		bind(); onPointerFunc();
	}
	void send() { (*this)(); }
	
	void bind(){ validate(); glBindBuffer(mType, id()); }
	void unbind() const { glBindBuffer(mType, 0); }

#ifdef AL_GRAPHICS_USE_OPENGL
	/* Warning: these are not supported in OpenGL ES */
	
	/// Map data store to client address space
	
	/// If successful, returns a valid pointer to the data, otherwise, it returns
	/// NULL.
	/// After using the pointer, call unmap() as soon as possible
	void * map(){ bind(); return glMapBuffer(mType, mMapMode); }

	/// Map data store to client address space.
	
	/// If successful, returns true and sets argument to address of data,
	/// otherwise, returns false and leaves argument unaffected.
	/// After using the pointer, call unmap() as soon as possible
	template <class T>
	bool map(T *& buf){
		if(asDataType<T>() == mDataType){
			void * r = map();
			if(r){ buf=(T *)r; return true; }
		}
		return false;
	}
	
	/// Unmaps data store from client address
	/// After unmap(), the mapped pointer is invalid
	bool unmap(){ return glUnmapBuffer(mType)==GL_TRUE; }
#endif

	/// Set buffer data store and copy over client data
	void data(const void * src, DataType dataType, int numElems, int numComps=1){
		mData = (void *)src;
		mDataType = dataType;
		mNumElems = numElems;
		mNumComps = numComps;
		data();
	}

	/// Set buffer data store and copy over client data
	template <class T>
	void data(const T * src, int numElems, int numComps=1){
		data(src, asDataType<T>(), numElems, numComps);
	}
	
	/// Set buffer data store without copying client data
	void data(DataType dataType, int numElems, int numComps=1){
		data(0, dataType, numElems, numComps);
	}

	/// Set buffer data store using cached values
	void data(){
		bind();
		glBufferData(mType, numBytes(mDataType)*mNumElems*mNumComps, mData, mUsage);
		unbind();
	}


protected:
	AccessMode mMapMode;
	BufferType mType;
	BufferUsage mUsage;
	DataType mDataType;
	int mNumComps;
	int mNumElems;
	void * mData;

	virtual void onCreate(){ glGenBuffers(1, (GLuint*)&mID); }
	virtual void onDestroy(){ glDeleteBuffers(1, (GLuint*)&mID); }
	virtual void onPointerFunc() = 0;
};



/// Vertex buffer object
class VBO : public BufferObject {
public:
	VBO(BufferUsage usage=STREAM_DRAW)
	:	BufferObject(ARRAY_BUFFER, usage){}
	
	static void enable(){ glEnableClientState(VERTEX_ARRAY); }
	static void disable(){ glDisableClientState(VERTEX_ARRAY); }

protected:
	virtual void onPointerFunc(){ glVertexPointer(mNumComps, mDataType, 0, 0); }
};


/// Color buffer object
class CBO : public BufferObject {
public:
	CBO(BufferUsage usage=STREAM_DRAW)
	:	BufferObject(ARRAY_BUFFER, usage){}

	static void enable(){ glEnableClientState(COLOR_ARRAY); }
	static void disable(){ glDisableClientState(COLOR_ARRAY); }

protected:
	virtual void onPointerFunc(){ glColorPointer(mNumComps, mDataType, 0, 0); }
};



/// Pixel buffer object
class PBO : public BufferObject {
public:
	PBO(bool packMode, BufferUsage usage=STREAM_DRAW)
	:	BufferObject(packMode ? PIXEL_PACK_BUFFER : PIXEL_UNPACK_BUFFER, usage){}
	
//	static void enable(){ glEnableClientState(ArrayType::Vertex); }
//	static void disable(){ glDisableClientState(ArrayType::Vertex); }

protected:
	virtual void onPointerFunc(){ glVertexPointer(mNumComps, mDataType, 0, 0); }
};



/// Element object buffer
class EBO : public BufferObject {
public:
	EBO(Graphics::Primitive prim=Graphics::POINTS, BufferUsage usage=STATIC_DRAW)
	:	BufferObject(ELEMENT_ARRAY_BUFFER, usage), mPrim(prim), mStart(0), mEnd(0)
	{}

	EBO& prim(Graphics::Primitive v){ mPrim=v; return *this; }
	EBO& range(int start, int end){ mStart=start; mEnd=end; return *this; }

protected:
	Graphics::Primitive mPrim;
	int mStart, mEnd;
	
	virtual void onPointerFunc(){
		if(mEnd)	glDrawRangeElements(mPrim, mStart, mEnd, mNumElems, mDataType, 0);
		else		glDrawRangeElements(mPrim, 0, mNumElems, mNumElems, mDataType, 0);
	}
};

/*
class BufferObjects : public GPUObject, public Drawable {
public:
	
	BufferObjects() {};

	virtual ~BufferObjects() {};
	virtual void draw(Graphics& gl);
	
	Mesh& data() { return *mData; }

protected:	

	virtual void onCreate() {};
	virtual void onDestroy() {};
	
	Mesh mData;

	VBO mVBO;
	CBO mCBO;
	EBO mEBO;
};
*/


template<class T> inline BufferObject::DataType BufferObject::asDataType(){ return UNKNOWN; }
template<> inline BufferObject::DataType BufferObject::asDataType<char>(){ return BYTE; }
template<> inline BufferObject::DataType BufferObject::asDataType<unsigned char>(){ return UNSIGNED_BYTE; }
template<> inline BufferObject::DataType BufferObject::asDataType<short>(){ return SHORT; }
template<> inline BufferObject::DataType BufferObject::asDataType<unsigned short>(){ return UNSIGNED_SHORT; }
template<> inline BufferObject::DataType BufferObject::asDataType<int>(){ return INT; }
template<> inline BufferObject::DataType BufferObject::asDataType<unsigned int>(){ return UNSIGNED_INT; }
template<> inline BufferObject::DataType BufferObject::asDataType<float>(){ return FLOAT; }
template<> inline BufferObject::DataType BufferObject::asDataType<double>(){ return DOUBLE; }

inline int BufferObject::numBytes(BufferObject::DataType type){
	#define CS(a,b) case a: return sizeof(b); 
	switch(type){
		CS(BYTE, char)
		CS(UNSIGNED_BYTE, unsigned char)
		CS(SHORT, short)
		CS(UNSIGNED_SHORT, unsigned short)
		CS(INT, int)
		CS(UNSIGNED_INT, unsigned int)
		CS(FLOAT, float)
		CS(DOUBLE, double)
		default: return 0;
	};
	#undef CS
}

} // al::
#endif
