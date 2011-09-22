#ifndef INCLUDE_AL_GRAPHICS_TEXTURE_HPP
#define INCLUDE_AL_GRAPHICS_TEXTURE_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Helper object for Graphics Textures
	
	File author(s):
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

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

	typedef Graphics::Format Format;
	typedef Graphics::DataType DataType;

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
		NEAREST_MIPMAP_NEAREST	= GL_NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST	= GL_LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR	= GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR	= GL_LINEAR_MIPMAP_LINEAR,
	};
	
	/**
		Construct a 2D Texture object.
	*/
	Texture(unsigned width=512, unsigned height=512, Graphics::Format format=Graphics::RGBA, Graphics::DataType type=Graphics::UBYTE);
	
	/**
		Construct a 3D Texture object.
	*/
	Texture(unsigned width, unsigned height, unsigned depth, Graphics::Format format=Graphics::RGBA, Graphics::DataType type=Graphics::UBYTE);
	
	virtual ~Texture(){}

	Filter filter() const { return mFilter; }
	Format format() const { return mFormat; }
	Target target() const { return mTarget; }
	DataType type() const { return mType; }	

	unsigned width() const { return mWidth; }
	unsigned height() const { return mHeight; }
	unsigned depth() const { return mDepth; }

	/// Return number of components per pixel
	unsigned numComponents() const { return Graphics::numComponents(format()); }

	/// Return total number of elements (components x width x height x depth)
	unsigned numElems() const {
		return numPixels() * numComponents();
	}
	
	unsigned numPixels() const {
		return width() * (height()?height():1) * (depth()?depth():1);
	}

	Texture& filter(Filter v){ return update(v, mFilter, mParamsUpdated); }
	Texture& format(Format v){ return update(v, mFormat, mPixelsUpdated); }
	Texture& target(Target v){ return update(v, mTarget, mPixelsUpdated); }
	Texture& type(DataType v){ return update(v, mType  , mPixelsUpdated); }

	Texture& width (unsigned v){ return update(v, mWidth, mPixelsUpdated); }
	Texture& height(unsigned v){ return update(v, mHeight,mPixelsUpdated); }
	Texture& depth (unsigned v){ return update(v, mDepth ,mPixelsUpdated); }

	Texture& wrap(Wrap v){ return wrap(v,v,v); }
	Texture& wrap(Wrap S, Wrap T){ return wrap(S,T,mWrapR); }
	Texture& wrap(Wrap S, Wrap T, Wrap R);

	/// bind the texture (to a multitexture unit)
	void bind(int unit = 0);
	/// unbind the texture (from a multitexture unit)
	void unbind(int unit = 0);
	
	/// render the texture onto a quad on the XY plane
	void quad(Graphics& gl, double w=1, double h=1, double x=0, double y=0);
	
	/// return reference to the internal CPU-side cache
	/// DO NOT MODIFY THE LAYOUT OR DIMENSIONS OF THIS ARRAY
	Array& array() { return mArray; }
	
	/// Submit the texture using an Array as source 
	/// NOTE: the graphics context (e.g. Window) must have been created
	/// if reconfigure is true, 
	/// it will attempt to derive size & layout from the array
	void submit(const Array& src, bool reconfigure=false);
		
	/// Resize texture data on GPU and copy over pixels
	/// NOTE: the graphics context (e.g. Window) must have been created
	/// If pixels is NULL, then the only effect is to resize the texture
	/// remotely.
	virtual void submit(const void * pixels=NULL, uint32_t align=4);

	/// allocate the internal Array for a CPU-side cache, copying from src
	void allocate(const Array& src, bool reconfigure=true);
	/// allocate memory for a CPU copy
	/// reconfigures the internal array 
	void allocate(unsigned align=1);
	
	void deallocate();

protected:
//	GLint mLevel;	// TODO: on a rainy day...
//	GLint mBorder;
	Target mTarget;				// TEXTURE_1D, TEXTURE_2D, etc. 
	Format mFormat;				// RGBA, ALPHA, etc.
	DataType mType;				// UBYTE, FLOAT, etc.
	Wrap mWrapS, mWrapT, mWrapR;	
	Filter mFilter;
	unsigned mWidth, mHeight, mDepth;
	GLint mUnpack;
	
	void * mPixels;				// pointer to client-side pixel data (0 if none)
	Array mArray;				// Array representation of client-side pixel data
//	void * mBuffer;				// internally allocated pixel buffer
	bool mParamsUpdated;
	bool mPixelsUpdated;
	
	
	virtual void onCreate(){
		//printf("Texture onCreate\n");
		glGenTextures(1, (GLuint *)&mID);
		sendParams();
		sendPixels();
		Graphics::error("creating texture");
	}
	
	virtual void onDestroy(){
		glDeleteTextures(1, (GLuint *)&mID);
	}
	
	/// ensures that the internal Array format matches the texture format
	void resetArray(unsigned align);

	void sendParams(bool force=true){
		if(mParamsUpdated || force){
			glBindTexture(target(), id());
			glTexParameterf(target(), GL_TEXTURE_MAG_FILTER, filter());
			glTexParameterf(target(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(target(), GL_TEXTURE_WRAP_S, mWrapS);
			glTexParameterf(target(), GL_TEXTURE_WRAP_T, mWrapT);
			glTexParameterf(target(), GL_TEXTURE_WRAP_R, mWrapR);
			glTexParameteri(target(), GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			glBindTexture(target(), 0);
			mParamsUpdated = false;
		}
	}
	
	void sendPixels(bool force=true){
		if(mPixelsUpdated || force){
			submit(mPixels);
			mPixelsUpdated = false;
		}
	}

	/// determines target (e.g. GL_TEXTURE_2D) from the dimensions
	void determineTarget();

	// Pattern for setting a variable that when changed sets a notification flag
	template<class T>
	Texture& update(const T& v, T& var, bool& flag){
		if(v!=var){ var=v; flag=true; } 
		return *this; 
	}

//	Format toFormat(const Array& src) {
//		switch(src.header.components) {
//			case 1:	return Graphics::LUMINANCE;
//			case 2:	return Graphics::LUMINANCE_ALPHA;
//			case 3:	return Graphics::RGB;
//			case 4:	return Graphics::RGBA;
//			default:
//				printf("warning: unknown type\n");
//				return Graphics::RGBA;
//		}
//	}
//	
//	DataType toDataType(const Array& src){
//		return Graphics::toDataType(src.header.type); }
//	
//	Target toTarget(const Array& src) {
//		switch(src.header.dimcount) {
//			case 1:		return TEXTURE_1D;
//			case 2:		return TEXTURE_2D;
//			case 3:		return TEXTURE_3D;
//			default:
//				printf("warning: unknown dimcount\n");
//				return TEXTURE_2D;
//		}
//	}
};




inline void Texture :: bind(int unit) {
	// ensure it is created:
	validate(); 
	sendParams(false);
	sendPixels(false);
	
	// multitexturing:
	glActiveTextureARB(GL_TEXTURE0_ARB + unit);
	
	// bind:
	glEnable(target());
	glBindTexture(target(), id());
	
	Graphics::error("binding texture");
}

inline void Texture :: unbind(int unit) {		
	// multitexturing:
	glActiveTextureARB(GL_TEXTURE0_ARB + unit);
	glBindTexture(target(), 0);
	glDisable(target());
}

inline void Texture :: resetArray(unsigned align) {
	mArray.dataFree();
	
	// reconfigure the internal array according to the current settings:
	switch (mTarget) {
		case TEXTURE_1D:
			mArray.header.type = Graphics::toAlloTy(mType);
			mArray.header.components = Graphics::numComponents(mFormat);
			mArray.header.dimcount = 1;
			mArray.header.dim[0] = mWidth;
			mArray.deriveStride(mArray.header, align);
			break;
			
		case TEXTURE_3D:
			mArray.header.type = Graphics::toAlloTy(mType);
			mArray.header.components = Graphics::numComponents(mFormat);
			mArray.header.dimcount = 3;
			mArray.header.dim[0] = mWidth;
			mArray.header.dim[1] = mHeight;
			mArray.header.dim[3] = mDepth;
			mArray.deriveStride(mArray.header, align);
			break;
			
		case TEXTURE_2D:
		default:
			mArray.header.type = Graphics::toAlloTy(mType);
			mArray.header.components = Graphics::numComponents(mFormat);
			mArray.header.dimcount = 2;
			mArray.header.dim[0] = mWidth;
			mArray.header.dim[1] = mHeight;
			mArray.deriveStride(mArray.header, align);
			break;
	}
	
	// if using array:
	uint32_t rowsize = (mArray.stride(1) * Graphics::numBytes(type()) * numComponents());
	mUnpack = (rowsize % 4 == 0) ? 4 : 1;
	
}

inline void Texture :: allocate(unsigned align) {
	deallocate();
	resetArray(align);
	mArray.dataCalloc();
	mPixels = mArray.data.ptr;
	mPixelsUpdated = true;
	
	//mBuffer = malloc(numElems() * Graphics::numBytes(type()));
	//mPixels = mBuffer;
}

inline void Texture :: allocate(const Array& src, bool reconfigure) {
	
	if (reconfigure) {
		// reconfigure texture from array:
		switch (src.dimcount()) {
			case 1: target(TEXTURE_1D); break;
			case 2: target(TEXTURE_2D); break;
			case 3: target(TEXTURE_3D); break;
			default:
				printf("invalid array dimensions for texture\n");
				return;
		}
		
		// reconfigure size:
		switch (src.dimcount()) {
			case 3:	depth(src.depth());
			case 2:	height(src.height());
			case 1:	width(src.width()); break;
		}
		
		// reconfigure components 
		// (only if necessary - no need to lose e.g. 
		// mFormat = DEPTH_COMPONENT if we are only changing size)
		if (Graphics::numComponents(mFormat) != src.components()) {
			switch (src.components()) {
				case 1:	
					format(Graphics::LUMINANCE); break; // alpha or luminance?
				case 2:	
					format(Graphics::LUMINANCE_ALPHA); break;
				case 3:	
					format(Graphics::RGB); break;
				case 4:	
					format(Graphics::RGBA); break;
				default:
					printf("invalid array component count for texture\n");
					return;
			}
		}
		
		// re-allocate array:
		allocate(src.alignment());
		
	} else {
		
		// TODO: read the source into the dst without changing dst layout
		
		// ensure that array matches texture:
		if (!src.isFormat(mArray.header)) {
			printf("couldn't allocate array, mismatch format\n");
			mArray.print();
			src.print();
			return;
		}
		
		// re-allocate array:
		allocate();
	}
	
	// copy data:
	memcpy(mArray.data.ptr, src.data.ptr, src.size());
	mPixels = mArray.data.ptr;
}

inline void Texture :: deallocate() {
//	if(mBuffer){
//		free(mBuffer);
//		mBuffer=0;
//		mPixels=0;
//	} 
	mArray.dataFree();
	mPixels=0;
}

inline Texture& Texture :: wrap(Wrap S, Wrap T, Wrap R){
		if(S!=mWrapS || T!=mWrapT || R!=mWrapR){
			mWrapS = S; mWrapT = T; mWrapR = R;
			mParamsUpdated = true;
		}
		return *this;
	}

} // al::

#endif
