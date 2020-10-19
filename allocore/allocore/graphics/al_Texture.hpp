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
	Lance Putnam, 2015, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
*/

#include <cstring> // memcpy
#include <functional>
#include "allocore/types/al_Array.hpp"
#include "allocore/types/al_Color.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{

/// A simple wrapper around an OpenGL Texture
/// @ingroup allocore
class Texture : public GPUObject {
public:

	typedef Graphics::Format	Format;
	typedef Graphics::DataType	DataType;

	enum Target {
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
		TEXTURE_1D				= GL_TEXTURE_1D,
		#endif
		TEXTURE_2D				= GL_TEXTURE_2D,
		#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
		TEXTURE_3D				= GL_TEXTURE_3D,
		#endif
		NO_TARGET				= 0
	};

	enum Wrap {
		REPEAT					= GL_REPEAT,
		CLAMP_TO_EDGE			= GL_CLAMP_TO_EDGE,
		#ifdef AL_GRAPHICS_SUPPORTS_WRAP_EXTRA
		MIRRORED_REPEAT			= GL_MIRRORED_REPEAT,
		CLAMP					= GL_CLAMP,
		CLAMP_TO_BORDER			= GL_CLAMP_TO_BORDER,
		#endif
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


	/// Construct an unsized Texture
	Texture();

	#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_1D
	/// Construct a 1D Texture object

	/// @param[in] width		width, in pixels
	/// @param[in] format		format of pixel data
	/// @param[in] type			data type of pixel data
	/// @param[in] clientAlloc	allocate data on the client
	Texture(
		unsigned width,
		Graphics::Format format=Graphics::RGBA,
		Graphics::DataType type=Graphics::UBYTE,
		bool clientAlloc=true
	);
	#endif

	/// Construct a 2D Texture object

	/// @param[in] width		width, in pixels
	/// @param[in] height		height, in pixels
	/// @param[in] format		format of pixel data
	/// @param[in] type			data type of pixel data
	/// @param[in] clientAlloc	allocate data on the client
	Texture(
		unsigned width, unsigned height,
		Graphics::Format format=Graphics::RGBA,
		Graphics::DataType type=Graphics::UBYTE,
		bool clientAlloc=true
	);

	#ifdef AL_GRAPHICS_SUPPORTS_TEXTURE_3D
	/// Construct a 3D Texture object

	/// @param[in] width		width, in pixels
	/// @param[in] height		height, in pixels
	/// @param[in] depth		depth, in pixels
	/// @param[in] format		format of pixel data
	/// @param[in] type			data type of pixel data
	/// @param[in] clientAlloc	allocate data on the client
	Texture(
		unsigned width, unsigned height, unsigned depth,
		Graphics::Format format=Graphics::RGBA,
		Graphics::DataType type=Graphics::UBYTE,
		bool clientAlloc=true
	);
	#endif

	/// Construct a Texture object from an Array header
	Texture(AlloArrayHeader& header);

	virtual ~Texture();


	/// Set shape (size, format, type, etc.) from array header

	/// @param[in] hdr		Array header from which to match shape
	/// @param[in] realloc	If true, then the texture's internal memory will
	///						be reallocated as necessary.
	void shapeFrom(const AlloArrayHeader& hdr, bool realloc=false);

	/// Set shape (size, format, type, etc.) from internal array

	/// This call can be used to ensure that the texture shape matches the
	/// internal array.
	void shapeFromArray();


	/// Get pixel (color) format
	Format format() const { return mFormat; }

	/// Get texel (color) format
	int texelFormat() const { return mTexelFormat; }

	/// Get target type (e.g., TEXTURE_2D)
	Target target() const { return mTarget; }

	/// Get pixel component data type
	DataType type() const { return mType; }

	/// Get width, in pixels
	unsigned width() const { return mWidth; }

	/// Get height, in pixels
	unsigned height() const { return mHeight; }

	/// Get depth, in pixels
	unsigned depth() const { return mDepth; }

	/// Whether the dimensions, format, or type have changed
	bool shapeUpdated() const { return mShapeUpdated; }

	/// Get minification filter type
	Filter filterMin() const { return mFilterMin; }

	/// Get magnification filter type
	Filter filterMag() const { return mFilterMag; }

	/// Get number of components per pixel
	unsigned numComponents() const { return Graphics::numComponents(format()); }

	/// Get total number of elements (components x width x height x depth)
	unsigned numElems() const {
		return numPixels() * numComponents();
	}

	/// Get total number of pixels
	unsigned numPixels() const {
		return width() * (height()?height():1) * (depth()?depth():1);
	}


	/// Set pixel (color) format
	Texture& format(Format v){ return update(v, mFormat, mShapeUpdated); }

	/// Set texel (color) format
	Texture& texelFormat(int v){ return update(v, mTexelFormat, mShapeUpdated); }

	/// Set target type (e.g., TEXTURE_2D)
	Texture& target(Target v){ return update(v, mTarget, mShapeUpdated); }

	/// Set pixel component data type
	Texture& type(DataType v){ return update(v, mType, mShapeUpdated); }

	/// Set width, in pixels
	Texture& width (unsigned v);

	/// Set height, in pixels
	Texture& height(unsigned v);

	/// Set depth, in pixels
	Texture& depth (unsigned v);

	/// Resize 1D texture
	Texture& resize(unsigned w){ return width(w); }

	/// Resize 2D texture
	Texture& resize(unsigned w, unsigned h){ return width(w).height(h); }

	/// Resize 3D texture
	Texture& resize(unsigned w, unsigned h, unsigned d){ return width(w).height(h).depth(d); }

	/// Set minification and magnification filter types
	Texture& filter(Filter v){ return filterMin(v).filterMag(v); }

	/// Set minification filter type
	Texture& filterMin(Filter v);

	/// Set magnification filter type
	Texture& filterMag(Filter v);

	/// Set wrapping mode for all dimensions
	Texture& wrap(Wrap v){ return wrap(v,v,v); }

	/// Set 2D wrapping modes
	Texture& wrap(Wrap S, Wrap T){ return wrap(S,T,mWrapR); }

	/// Set 3D wrapping modes
	Texture& wrap(Wrap S, Wrap T, Wrap R);


	/// Bind the texture (to a multitexture unit)
	void bind(int unit = 0);

	/// Unbind the texture (from a multitexture unit)
	void unbind(int unit = 0);

	/// Render the texture onto a quad on the XY plane
	void quad(Graphics& gl, double w=1, double h=1, double x=0, double y=0, double z=0);

	/// Render the texture onto a quad filling current viewport
	void quadViewport(
		Graphics& g, const Color& color = Color(1),
		double w=2, double h=2, double x=-1, double y=-1, double z=0);


	/// Get mutable reference to the internal pixel data
	/// DO NOT MODIFY THE LAYOUT OR DIMENSIONS OF THIS ARRAY
	Array& array() { mArrayDirty=true; return mArray; }

	/// Get read-only reference to internal pixel data
	const Array& array() const { return mArray; }

	/// Get raw pointer to internal pixel data
	template<class T> T * data(){ return (T*)(data()); }
	char * data(){ mArrayDirty=true; return array().data.ptr; }

	template<class T> const T * data() const { return (const T*)(data()); }
	const char * data() const { return array().data.ptr; }

	/// Get reference to a pixel
	template<class T> T& at(unsigned x, unsigned y){ return array().as<T>(x,y); }
	template<class T> const T& at(unsigned x, unsigned y) const { return array().as<T>(x,y); }

	/// Assign pixel values using a visitor function

	/// onPixel is called for each pixel in the texture.
	/// (i,j) is the pixel column and row.
	/// rgba should be filled with the desired values. The pixel color array has
	/// the default value (0,0,0,1).
	void assign(const std::function<void(int i, int j, float * rgba)>& onPixel);

	template <class Trgba>
	void assign(const std::function<Trgba (int i, int j)>& onPixel){
		static_assert(sizeof(Trgba)/sizeof(typename Trgba::value_type)>=4,
			"Requires 4 element RGBA array");
		assign([&onPixel](int i, int j, float * rgba){
			auto trgba = onPixel(i,j);
			for(int i=0; i<4; ++i) rgba[i] = trgba[i];
		});
	}

	/// Assign pixel values using a visitor function
	void assignFromTexCoord(const std::function<void(float s, float t, float * rgba)>& onPixel);
		// Note: Through a C++ quirk, cannot overload on std::function

	template <class Trgba>
	void assignFromTexCoord(const std::function<Trgba (float s, float t)>& onPixel){
		const auto rw = 1.f/(width()-1);
		const auto rh = 1.f/(height()-1);
		assign<Trgba>([rw,rh,&onPixel](int i, int j){
			return onPixel(float(i)*rw, float(j)*rh);
		});
	}

	/// Flags resubmission of pixel data upon next bind

	/// Calling this ensures that pixels get submitted on the next bind().
	///internal array
	Texture& dirty(){ mPixelsUpdated=true; return *this; }

	/// Submit the texture to GPU using an Array as source

	/// NOTE: the graphics context (e.g. Window) must have been created.
	/// If reconfigure is true, it will attempt to derive size & layout from the
	/// array.
	void submit(const Array& src, bool reconfigure=false);

	/// Copy client pixels to GPU texels

	/// NOTE: the graphics context (e.g. Window) must have been created
	/// If pixels is NULL, then the only effect is to resize the texture
	/// remotely.
	void submit(const void * pixels, uint32_t align=4);

	/// Submit the client texture state to GPU

	/// If the client pixels have been allocated, then they will be sent if
	/// marked dirty. Otherwise, the texture is simply reconfigured on the GPU.
	void submit();

	/// Copy pixels from current frame buffer to texture texels

	/// @param[in] w		width of region to copy; w<0 uses w + 1 + texture.width
	/// @param[in] h		height of region to copy; h<0 uses h + 1 + texture.height
	/// @param[in] fbx		pixel offset from left edge of frame buffer
	/// @param[in] fby		pixel offset from bottom edge of frame buffer
	/// @param[in] texx		texel offset in x direction
	/// @param[in] texy		texel offset in y direction (2D/3D only)
	/// @param[in] texz		texel offset in z direction (3D only)
	void copyFrameBuffer(
		int w=-1, int h=-1,
		int fbx=0, int fby=0,
		int texx=0, int texy=0, int texz=0
	);

	/// Allocate the internal Array for a client-side cache, copying from src
	void allocate(const Array& src, bool reconfigure=true);

	/// Allocate client-side texture memory using current shape
	void allocate(unsigned align=1);

	/// Allocate client-side texture memory, copying from src
	template <class T>
	void allocate(const T * src, unsigned w, Graphics::Format format);

	/// Allocate client-side texture memory, copying from src
	template <class T>
	void allocate(const T * src, unsigned w, unsigned h, Graphics::Format format);

	/// Allocate client-side texture memory, copying from src
	template <class T>
	void allocate(const T * src, unsigned w, unsigned h, unsigned d, Graphics::Format format);

	template <class T>
	void allocate(const T * src, unsigned w, unsigned c);
	template <class T>
	void allocate(const T * src, unsigned w, unsigned h, unsigned c);
	template <class T>
	void allocate(const T * src, unsigned w, unsigned h, unsigned d, unsigned c);

	/// Deallocate any allocated client-side memory
	void deallocate();


	/// Print information about texture
	void print();

protected:
	//int mLevel;	// TODO: on a rainy day...
	//int mBorder;
	Target mTarget;				// TEXTURE_1D, TEXTURE_2D, etc.
	Format mFormat;				// RGBA, ALPHA, etc.
	int mTexelFormat=0;			// default is 0 = auto
	DataType mType;				// UBYTE, FLOAT, etc.
	Wrap mWrapS, mWrapT, mWrapR;
	Filter mFilterMin, mFilterMag;
	unsigned mWidth=0, mHeight=0, mDepth=0;

	Array mArray;				// Array representation of client-side pixel data
	bool mParamsUpdated=true;	// Flags change in texture params (wrap, filter)
	bool mPixelsUpdated=true;	// Flags change in pixel data
	bool mShapeUpdated=true;	// Flags change in size, format, type, etc.
	bool mArrayDirty=false;
	bool mMipmap;

	virtual void onCreate();
	virtual void onDestroy();

	void init();
	void deriveTarget();

	// ensures that the internal Array format matches the texture format
	void resetArray(unsigned align);

	// send any pending parameter updates to GPU or do immediately if forced
	void sendParams(bool force=true);

	// send any pending pixels updates to GPU or do immediately if forced
	void sendPixels(bool force=true);
	void sendPixels(const void * pixels, unsigned align);

	// send any pending shape updates to GPU or do immediately if forced
	void sendShape(bool force=true);

	bool tryBind();

	// Pattern for setting a variable that when changed sets a notification flag
	template<class T>
	Texture& update(const T& v, T& var, bool& flag){
		if(v!=var){ var=v; flag=true; }
		return *this;
	}

public:
	Texture& updatePixels(); /// \deprecated use dirty() instead
	void configure(AlloArrayHeader& header); /// \deprecated use shapeFrom() instead
};


// Implementation --------------------------------------------------------------

const char * toString(Texture::Target v);
const char * toString(Texture::Wrap v);
const char * toString(Texture::Filter v);

template <class T>
void Texture::allocate(const T * src, unsigned w, Graphics::Format format_ ){
	allocate(src, w,0,0, format_);
}

template <class T>
void Texture::allocate(const T * src, unsigned w, unsigned h, Graphics::Format format_){
	allocate(src, w,h,0, format_);
}

template <class T>
void Texture::allocate(const T * src, unsigned w, unsigned h, unsigned d, Graphics::Format format_){
	type(Graphics::toDataType<T>());
	format(format_);
	resize(w, h, d);
	deriveTarget();
	allocate();
	memcpy(mArray.data.ptr, src, mArray.size());
}

template <class T>
void Texture::allocate(const T * src, unsigned w, unsigned c){ allocate(src, w,0,0,c); }

template <class T>
void Texture::allocate(const T * src, unsigned w, unsigned h, unsigned c){ allocate(src, w,h,0,c); }

template <class T>
void Texture::allocate(const T * src, unsigned w, unsigned h, unsigned d, unsigned c){
	static Graphics::Format fmts[] = {Graphics::LUMINANCE, Graphics::LUMINANCE_ALPHA, Graphics::RGB, Graphics::RGBA};
	if(1 <= c && c <= 4) allocate(src, w,h,d, fmts[c-1]);
}

} // al::

#endif
