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

#include <vector>
#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

/// Generic buffer object
/// @ingroup allocore

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

	/// Buffer access mode
	enum AccessMode{
		READ_ONLY				= GL_READ_ONLY,
		WRITE_ONLY				= GL_WRITE_ONLY,
		READ_WRITE				= GL_READ_WRITE
	};

	/// Array type
	enum ArrayType{
		VERTEX_ARRAY			= GL_VERTEX_ARRAY,
		NORMAL_ARRAY			= GL_NORMAL_ARRAY,
		COLOR_ARRAY				= GL_COLOR_ARRAY,
		INDEX_ARRAY				= GL_INDEX_ARRAY,
		TEXTURE_COORD_ARRAY		= GL_TEXTURE_COORD_ARRAY,
		EDGE_FLAG_ARRAY			= GL_EDGE_FLAG_ARRAY
	};

	/// Buffer type
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
		//STREAM_READ		= GL_STREAM_READ,
		//STREAM_COPY		= GL_STREAM_COPY,
		STATIC_DRAW		= GL_STATIC_DRAW,
		//STATIC_READ		= GL_STATIC_READ,
		//STATIC_COPY		= GL_STATIC_COPY,
		DYNAMIC_DRAW	= GL_DYNAMIC_DRAW,
		//DYNAMIC_READ	= GL_DYNAMIC_READ,
		//DYNAMIC_COPY	= GL_DYNAMIC_COPY
	};

	/// @param[in] bufType	buffer type (array, element, etc.)
	/// @param[in] bufUsage	buffer usage (stream, static, etc.)
	BufferObject(BufferType bufType, BufferUsage bufUsage);

	virtual ~BufferObject();


	/// Get buffer store size, in bytes
	int size() const;


	/// Set buffer type
	void bufferType(BufferType v);

	/// Set buffer usage
	void usage(BufferUsage v);

	/// Set size, in bytes, of buffer without sending data
	void resize(int numBytes);

	/// Set buffer data store and copy over client data
	void data(const void * src, Graphics::DataType dataType, int numElems, int numComps=1);

	/// Set buffer data store and copy over client data
	template <class T>
	void data(const T * src, int numElems, int numComps=1);

	/// Set buffer data store without copying client data
	void data(Graphics::DataType dataType, int numElems, int numComps=1);

	/// Set buffer data store using cached values
	void data();

	/// Set subregion of buffer store
	template <class T>
	int subData(const T * src, int numElems, int byteOffset=0);


	/// Bind buffer
	void bind();

	/// Unbind buffer
	void unbind() const;

	void operator()();
	void send();


	void print() const;


	#ifdef AL_GRAPHICS_USE_OPENGL
	/* Warning: these are not supported in OpenGL ES */

	/// Set map mode
	void mapMode(AccessMode v);

	/// Map data store to client address space

	/// If successful, returns a valid pointer to the data, otherwise, it returns
	/// NULL.
	/// After using the pointer, call unmap() as soon as possible
	void * map();

	/// Map data store to client address space

	/// If successful, returns true and sets argument to address of data,
	/// otherwise, returns false and leaves argument unaffected.
	/// After using the pointer, call unmap() as soon as possible
	template <class T>
	bool map(T *& buf){
		if(Graphics::toDataType<T>() == mDataType){
			void * r = map();
			if(r){ buf=(T *)r; return true; }
		}
		return false;
	}

	/// Unmaps data store from client address
	/// After unmap(), the mapped pointer is invalid
	bool unmap();
	#endif

protected:
	AccessMode mMapMode;
	BufferType mType;
	BufferUsage mUsage;
	Graphics::DataType mDataType;
	int mNumComps;
	int mNumElems;
	void * mData;
	struct SubData{
		SubData(const void * data_=NULL, unsigned size_=0, unsigned offset_=0)
		:	data(data_), size(size_), offset(offset_){}
		const void * data;
		unsigned size;
		unsigned offset;
	};
	std::vector<SubData> mSubData;

	virtual void onCreate();
	virtual void onDestroy();
	virtual void onPointerFunc(){};
};


/// Vertex buffer object
class VBO : public BufferObject {
public:
	VBO(BufferUsage usage=STREAM_DRAW);

	static void enable();
	static void disable();

protected:
	virtual void onPointerFunc();
};


/// Color buffer object
class CBO : public BufferObject {
public:
	CBO(BufferUsage usage=STREAM_DRAW);

	static void enable();
	static void disable();

protected:
	virtual void onPointerFunc();
};


/// Pixel buffer object
class PBO : public BufferObject {
public:
	PBO(bool packMode, BufferUsage usage=STREAM_DRAW);

protected:
	virtual void onPointerFunc();
};


/// Element object buffer
class EBO : public BufferObject {
public:
	EBO(Graphics::Primitive prim=Graphics::POINTS, BufferUsage usage=STATIC_DRAW);

	EBO& primitive(Graphics::Primitive v);
	EBO& range(int start, int end);

protected:
	Graphics::Primitive mPrim;
	int mStart, mEnd;

	virtual void onPointerFunc();
};

const char * toString(BufferObject::BufferType v);
const char * toString(BufferObject::BufferUsage v);

// IMPLEMENTATION --------------------------------------------------------------

template <class T>
void BufferObject::data(const T * src, int numElems, int numComps){
	data(src, Graphics::toDataType<T>(), numElems, numComps);
}

template <class T>
int BufferObject::subData(const T * src, int numElems, int byteOffset){
	if(numElems){
		bind();
		glBufferSubData(mType, byteOffset, numElems*sizeof(T), src);
		unbind();
		mSubData.push_back(SubData(src, numElems*sizeof(T), byteOffset));
	}
	return numElems*sizeof(T) + byteOffset;
}



} // al::
#endif
