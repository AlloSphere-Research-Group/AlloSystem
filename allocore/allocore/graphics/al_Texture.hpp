#ifndef INCLUDE_AL_GRAPHICS_TEXTURE_HPP
#define INCLUDE_AL_GRAPHICS_TEXTURE_HPP

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
	Helper object for Graphics Textures
	
	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/system/al_Printing.hpp"
#include "allocore/types/al_Array.hpp"
#include "allocore/types/al_Color.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{

/**
	A simple wrapper around OpenGL Texture
*/
class Texture : public GPUObject {
public:

	typedef Graphics::Format	Format;
	typedef Graphics::DataType	DataType;

	enum Target {
		TEXTURE_1D				= GL_TEXTURE_1D,
		TEXTURE_2D				= GL_TEXTURE_2D,
		TEXTURE_3D				= GL_TEXTURE_3D
	};

	enum Wrap {
		CLAMP					= GL_CLAMP,
		CLAMP_TO_BORDER			= GL_CLAMP_TO_BORDER,
		CLAMP_TO_EDGE			= GL_CLAMP_TO_EDGE,
		MIRRORED_REPEAT			= GL_MIRRORED_REPEAT,
		REPEAT					= GL_REPEAT
	};

	enum Filter {
		NEAREST					= GL_NEAREST,
		LINEAR					= GL_LINEAR,
		// first term is within mipmap level, second term is between mipmap levels:
		NEAREST_MIPMAP_NEAREST	= GL_NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST	= GL_LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR	= GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR	= GL_LINEAR_MIPMAP_LINEAR,
	};


	/// Construct a 2D Texture object

	/// @param[in] width		width, in pixels
	/// @param[in] height		height, in pixels
	/// @param[in] format		format of pixel data
	/// @param[in] type			data type of pixel data
	/// @param[in] clientAlloc	allocate data on the client
	Texture(
		unsigned width=512, unsigned height=512,
		Graphics::Format format=Graphics::RGBA, Graphics::DataType type=Graphics::UBYTE,
		bool clientAlloc=true
	);
	

	/// Construct a 3D Texture object

	/// @param[in] width		width, in pixels
	/// @param[in] height		height, in pixels
	/// @param[in] depth		depth, in pixels
	/// @param[in] format		format of pixel data
	/// @param[in] type			data type of pixel data
	/// @param[in] clientAlloc	allocate data on the client
	Texture(
		unsigned width, unsigned height, unsigned depth,
		Graphics::Format format=Graphics::RGBA, Graphics::DataType type=Graphics::UBYTE,
		bool clientAlloc=true
	);
	

	/// Construct a Texture object from an Array header:
	Texture(AlloArrayHeader& header);
	
	void configure(AlloArrayHeader& header);
	
	virtual ~Texture();

	Format format() const { return mFormat; }
	int texelFormat() const { return mTexelFormat; }
	Target target() const { return mTarget; }
	DataType type() const { return mType; }	

	unsigned width() const { return mWidth; }
	unsigned height() const { return mHeight; }
	unsigned depth() const { return mDepth; }

	Filter filterMin() const { return mFilterMin; }
	Filter filterMag() const { return mFilterMag; }
	
	/// Return number of components per pixel
	unsigned numComponents() const { return Graphics::numComponents(format()); }

	/// Return total number of elements (components x width x height x depth)
	unsigned numElems() const {
		return numPixels() * numComponents();
	}
	
	unsigned numPixels() const {
		return width() * (height()?height():1) * (depth()?depth():1);
	}

	Texture& format(Format v){ return update(v, mFormat, mPixelsUpdated); }
	Texture& texelFormat(int v){ return update(v, mTexelFormat, mPixelsUpdated); }
	Texture& target(Target v){ return update(v, mTarget, mPixelsUpdated); }
	Texture& type(DataType v){ return update(v, mType, mPixelsUpdated); }

	Texture& width (unsigned v){ return update(v, mWidth, mPixelsUpdated); }
	Texture& height(unsigned v){ return update(v, mHeight,mPixelsUpdated); }
	Texture& depth (unsigned v){ return update(v, mDepth ,mPixelsUpdated); }

	Texture& resize(unsigned w){ return width(w); }
	Texture& resize(unsigned w, unsigned h){ return width(w).height(h); }
	Texture& resize(unsigned w, unsigned h, unsigned d){ return width(w).height(h).depth(d); }

	Texture& filterMin(Filter v){ return update(v, mFilterMin, mParamsUpdated); }
	Texture& filterMag(Filter v){ return update(v, mFilterMag, mParamsUpdated); }
	Texture& wrap(Wrap v){ return wrap(v,v,v); }
	Texture& wrap(Wrap S, Wrap T){ return wrap(S,T,mWrapR); }
	Texture& wrap(Wrap S, Wrap T, Wrap R);

	/// Bind the texture (to a multitexture unit)
	void bind(int unit = 0);
	
	/// Unbind the texture (from a multitexture unit)
	void unbind(int unit = 0);
	
	/// Render the texture onto a quad on the XY plane
	void quad(Graphics& gl, double w=1, double h=1, double x=0, double y=0);

	/// Render the texture onto a quad filling current viewport
	void quadViewport(
		Graphics& g, const Color& color = Color(1),
		double w=2, double h=2, double x=-1, double y=-1);

	/// return reference to the internal CPU-side cache
	/// DO NOT MODIFY THE LAYOUT OR DIMENSIONS OF THIS ARRAY
	Array& array() { return mArray; }

	/// Get raw pointer to client-side pixel data
	template<class T> T * data(){ return (T*)(data()); }
	char * data(){ return array().data.ptr; }

	/// Flags resubmission of pixel data upon next bind
	
	/// Calling this ensures that pixels get submitted on the next bind().
	///
	Texture& dirty(){ mPixelsUpdated=true; return *this; }

	/// Submit the texture using an Array as source

	/// NOTE: the graphics context (e.g. Window) must have been created
	/// if reconfigure is true, 
	/// it will attempt to derive size & layout from the array
	void submit(const Array& src, bool reconfigure=false);
		
	/// Resize texture data on GPU and copy over pixels

	/// NOTE: the graphics context (e.g. Window) must have been created
	/// If pixels is NULL, then the only effect is to resize the texture
	/// remotely.
	void submit(const void * pixels, uint32_t align=4);
	
	/// NOTE: only valid when the graphics context is valid:
	Texture& generateMipmap() { bind(); glGenerateMipmapEXT(target()); unbind(); return *this; }

	/// Allocate the internal Array for a CPU-side cache, copying from src
	void allocate(const Array& src, bool reconfigure=true);

	/// Allocate memory for a CPU copy (reconfigures the internal array)
	void allocate(unsigned align=1);
	
	/// Deallocate internal memory
	void deallocate();
	
	/// debug printing
	void print();

protected:
//	int mLevel;	// TODO: on a rainy day...
//	int mBorder;
	Target mTarget;				// TEXTURE_1D, TEXTURE_2D, etc. 
	Format mFormat;				// RGBA, ALPHA, etc.
	int mTexelFormat;			// default is 0 = auto
	DataType mType;				// UBYTE, FLOAT, etc.
	Wrap mWrapS, mWrapT, mWrapR;	
	Filter mFilterMin, mFilterMag;
	unsigned mWidth, mHeight, mDepth;
	int mUnpack;
	
	// redundant; use mArray.data.ptr instead:
	//void * mPixels;				// pointer to client-side pixel data (0 if none)
	Array mArray;				// Array representation of client-side pixel data
//	void * mBuffer;				// internally allocated pixel buffer
	bool mParamsUpdated;
	bool mPixelsUpdated;
	bool mOwnsData;
	

	virtual void onCreate();
	virtual void onDestroy();
	
	// ensures that the internal Array format matches the texture format
	void resetArray(unsigned align);

	// send any pending parameter updates to GPU or do immediately if forced
	void sendParams(bool force=true);
	
	// send any pending pixels updates to GPU or do immediately if forced
	void sendPixels(bool force=true);

	/// determines target (e.g. GL_TEXTURE_2D) from the dimensions
	void determineTarget();

	// Pattern for setting a variable that when changed sets a notification flag
	template<class T>
	Texture& update(const T& v, T& var, bool& flag){
		if(v!=var){ var=v; flag=true; } 
		return *this; 
	}

public:
	Texture& updatePixels(); /// \deprecated use dirty() instead
	void submit(); /// \deprecated use dirty() instead
};




inline Texture& Texture :: wrap(Wrap S, Wrap T, Wrap R){
	if(S!=mWrapS || T!=mWrapT || R!=mWrapR){
		mWrapS = S; mWrapT = T; mWrapR = R;
		mParamsUpdated = true;
	}
	return *this;
}

} // al::

#endif
