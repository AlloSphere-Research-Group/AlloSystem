#ifndef GLW_BUFFEROBJECT_H_INC
#define GLW_BUFFEROBJECT_H_INC

#include "GPUObject.h"

namespace glw{

/// Generic buffer object
class BufferObject : public GPUObject {
public:
	BufferObject(BufferType::t bufType, BufferUsage::t bufUsage)
	:	mMapMode(AccessMode::ReadWrite), mType(bufType), mUsage(bufUsage),
		mDataType(DataType::Unknown), mNumComps(0), mNumElems(0), mData(0)
	{}
	
	virtual ~BufferObject(){ destroy(); }

	void bufferType(BufferType::t v){ mType=v; }
	void mapMode(AccessMode::t v){ mMapMode=v; }
	

	void operator()(){
		//data(); onPointerFunc(); unbind();
		bind(); onPointerFunc();
	}

	void bind() const { glBindBuffer(mType, id()); }
	void unbind() const { glBindBuffer(mType, 0); }

	/// Map data store to client address space
	
	/// If successful, returns a valid pointer to the data, otherwise, it returns
	/// NULL.
	void * map(){ bind(); return glMapBuffer(mType, mMapMode); }

	
	/// Map data store to client address space.
	
	/// If successful, returns true and sets argument to address of data,
	/// otherwise, returns false and leaves argument unaffected.
	template <class T>
	bool map(T *& buf){
		if(asType<T>() == mDataType){
			void * r = map();
			if(r){ buf=(T *)r; return true; }
		}
		return false;
	}
	
	/// Unmaps data store from client address
	bool unmap(){ return glUnmapBuffer(mType)==GL_TRUE; }

	/// Set buffer data store and copy over client data
	void data(const void * src, DataType::t dataType, sizei_t numElems, int_t numComps=1){
		mData = (void *)src;
		mDataType = dataType;
		mNumElems = numElems;
		mNumComps = numComps;
		data();
	}

	/// Set buffer data store and copy over client data
	template <class T>
	void data(const T * src, sizei_t numElems, int_t numComps=1){
		data(src, asType<T>(), numElems, numComps);
	}
	
	/// Set buffer data store without copying client data
	void data(DataType::t dataType, sizei_t numElems, int_t numComps=1){
		data(0, dataType, numElems, numComps);
	}

	/// Set buffer data store using cached values
	void data() const {
		bind();
		glBufferData(mType, numBytes(mDataType)*mNumElems*mNumComps, mData, mUsage);
		unbind();
	}


protected:
	AccessMode::t mMapMode;
	BufferType::t mType;
	BufferUsage::t mUsage;
	DataType::t mDataType;
	int_t mNumComps;
	size_t mNumElems;
	void * mData;

	virtual void onCreate(){ glGenBuffers(1, &mID); }
	virtual void onDestroy(){ glDeleteBuffers(1, &mID); }
	virtual void onPointerFunc() = 0;
};



/// Vertex buffer object
class VBO : public BufferObject {
public:
	VBO(BufferUsage::t usage=glw::StreamDraw)
	:	BufferObject(BufferType::ArrayBuffer, usage){}
	
	static void enable(){ glEnableClientState(ArrayType::VertexArray); }
	static void disable(){ glDisableClientState(ArrayType::VertexArray); }

protected:
	virtual void onPointerFunc(){ glVertexPointer(mNumComps, mDataType, 0, 0); }
};


/// Color buffer object
class CBO : public BufferObject {
public:
	CBO(BufferUsage::t usage=glw::StreamDraw)
	:	BufferObject(BufferType::ArrayBuffer, usage){}

	static void enable(){ glEnableClientState(ArrayType::ColorArray); }
	static void disable(){ glDisableClientState(ArrayType::ColorArray); }

protected:
	virtual void onPointerFunc(){ glColorPointer(mNumComps, mDataType, 0, 0); }
};



/// Pixel buffer object
class PBO : public BufferObject {
public:
	PBO(bool packMode, BufferUsage::t usage=glw::StreamDraw)
	:	BufferObject(packMode ? BufferType::PixelPack : BufferType::PixelUnpack, usage){}
	
//	static void enable(){ glEnableClientState(ArrayType::Vertex); }
//	static void disable(){ glDisableClientState(ArrayType::Vertex); }

protected:
	virtual void onPointerFunc(){ glVertexPointer(mNumComps, mDataType, 0, 0); }
};



/// Element object buffer
class EBO : public BufferObject {
public:
	EBO(Prim::t prim, BufferUsage::t usage=glw::StaticDraw)
	:	BufferObject(BufferType::ElementArray, usage), mPrim(prim), mStart(0), mEnd(0)
	{}

	EBO& prim(Prim::t v){ mPrim=v; return *this; }
	EBO& range(int_t start, int_t end){ mStart=start; mEnd=end; return *this; }

protected:
	Prim::t mPrim;
	int_t mStart, mEnd;
	
	virtual void onPointerFunc(){
		if(mEnd)	glDrawRangeElements(mPrim, mStart, mEnd, mNumElems, mDataType, 0);
		else		glDrawRangeElements(mPrim, 0, mNumElems, mNumElems, mDataType, 0);
	}
};


} // glw::
#endif
