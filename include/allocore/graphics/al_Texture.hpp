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
	
	/// debug printing
	void print();
	
	/// mark pixels as dirtied (e.g. if modified array contents)
	/// to ensure they get submitted on the next bind():
	void dirty() { mPixelsUpdated = true; }

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

inline Texture& Texture :: wrap(Wrap S, Wrap T, Wrap R){
	if(S!=mWrapS || T!=mWrapT || R!=mWrapR){
		mWrapS = S; mWrapT = T; mWrapR = R;
		mParamsUpdated = true;
	}
	return *this;
}

inline void Texture :: print() {
	std::string target = "?";
	std::string format = "?";
	std::string type = "?";
	
	switch (mTarget) {
		case TEXTURE_1D: 
			target = "GL_TEXTURE_1D"; 
			printf("Texture target=%s, %d(%d), ", target.c_str(), width(), mArray.width());
			break;
		case TEXTURE_2D: 
			target = "GL_TEXTURE_2D";
			printf("Texture target=%s, %dx%d(%dx%d), ", target.c_str(), width(), height(), mArray.width(), mArray.height()); 
			break;
		case TEXTURE_3D: 
			target = "GL_TEXTURE_3D"; 
			printf("Texture target=%s, %dx%dx%d(%dx%dx%d), ", target.c_str(), width(), height(), depth(), mArray.width(), mArray.height(), mArray.depth());
			break;
		default: break;
	}
	switch (mFormat) {
		case Graphics::DEPTH_COMPONENT: format="GL_DEPTH_COMPONENT"; break;
		case Graphics::LUMINANCE: format="GL_LUMINANCE"; break;
		case Graphics::LUMINANCE_ALPHA: format="GL_LUMINANCE_ALPHA"; break;
		case Graphics::RED: format="GL_RED"; break;
		case Graphics::GREEN: format="GL_GREEN"; break;
		case Graphics::BLUE: format="GL_BLUE"; break;
		case Graphics::ALPHA: format="GL_ALPHA"; break;
		case Graphics::RGB: format="GL_RGB"; break;
		case Graphics::RGBA: format="GL_RGBA"; break;
		case Graphics::BGRA: format="GL_BGRA"; break;		
		default: break;
	}	
	switch (mType) {
		case GL_BYTE: type = "GL_BYTE"; break;
		case GL_UNSIGNED_BYTE: type = "GL_UNSIGNED_BYTE"; break;
		case GL_SHORT: type = "GL_SHORT"; break;
		case GL_UNSIGNED_SHORT: type = "GL_UNSIGNED_SHORT"; break;
		case GL_INT: type = "GL_INT"; break;
		case GL_UNSIGNED_INT: type = "GL_UNSIGNED_INT"; break;
		case GL_FLOAT: type = "GL_FLOAT"; break;
		case GL_DOUBLE: type = "GL_DOUBLE"; break;
		default: break;
	}
	
	printf("type=%s(%s), format=%s(%d), unpack=%d(align=%d))\n", type.c_str(), allo_type_name(mArray.type()), format.c_str(), mArray.components(), mUnpack, mArray.alignment());
	//mArray.print();

}

} // al::

#endif
