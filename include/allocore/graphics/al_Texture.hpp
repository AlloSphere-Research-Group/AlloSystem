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

/*!
	A simple wrapper around OpenGL Textures, 
	using al::Array as a CPU-side interface for configuring & submitting
	
	TODO: lift out common features of Texture and CubeMapTexture into a 
	generic superclass ?
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


	Texture(unsigned width=512, unsigned height=512, unsigned depth=0)
	:	GPUObject(),
		mTarget(TEXTURE_2D),
		mFormat(Graphics::RGBA),
		mType(Graphics::UBYTE),
		mWrapS(CLAMP_TO_EDGE),
		mWrapT(CLAMP_TO_EDGE),
		mWrapR(CLAMP_TO_EDGE),
		mFilter(LINEAR),
//		mLevel(0),
//		mBorder(0),
//		mAlignment(4),
		mWidth(width),
		mHeight(height),
		mDepth(depth),
		mPixels(0), mBuffer(0),
		mParamsUpdated(true),
		mPixelsUpdated(true)
	{
		determineTarget();
	}
	
	Texture(unsigned width, unsigned height, Graphics::Format format, Graphics::DataType type)
	:	mTarget(TEXTURE_2D),
		mFormat(format),
		mType(type),
		mWrapS(CLAMP_TO_EDGE),
		mWrapT(CLAMP_TO_EDGE),
		mWrapR(CLAMP_TO_EDGE),
		mFilter(LINEAR),
		mWidth(width),
		mHeight(height),
		mDepth(0),
		mPixels(0), mBuffer(0),
		mParamsUpdated(true),
		mPixelsUpdated(true)
	{}	
	
	virtual ~Texture(){}


	Filter filter() const { return mFilter; }
	Format format() const { return mFormat; }
	Target target() const { return mTarget; }
	DataType type() const { return mType; }	

	unsigned width() const { return mWidth; }
	unsigned height() const { return mHeight; }
	unsigned depth() const { return mDepth; }

	/// Return number of components per pixel
	unsigned numComps() const { return Graphics::numComponents(format()); }

	/// Return total number of elements (components x width x height x depth)
	unsigned numElems() const {
		return numPixels() * numComps();
	}
	
	unsigned numPixels() const {
		return width() * (height()?height():1) * (depth()?depth():1);
	}

	Texture& filter(Filter v){ return update(v, mFilter, mParamsUpdated); }
	Texture& format(Format v){ return update(v, mFormat, mPixelsUpdated); }
	Texture& type(DataType v){ return update(v, mType  , mPixelsUpdated); }

	Texture& width (unsigned v){ return update(v, mWidth, mPixelsUpdated); }
	Texture& height(unsigned v){ return update(v, mHeight,mPixelsUpdated); }
	Texture& depth (unsigned v){ return update(v, mDepth ,mPixelsUpdated); }

	Texture& wrap(Wrap v){ return wrap(v,v,v); }
	Texture& wrap(Wrap S, Wrap T){ return wrap(S,T,mWrapR); }
	Texture& wrap(Wrap S, Wrap T, Wrap R){
		if(S!=mWrapS || T!=mWrapT || R!=mWrapR){
			mWrapS = S; mWrapT = T; mWrapR = R;
			mParamsUpdated = true;
		}
		return *this;
	}


	void bind(int unit = 0) {
		// ensure it is created:
		//validate(); // FIXME: needed?
		sendParams(false);
		sendPixels(false);
		
		// multitexturing:
		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		
		// bind:
		glEnable(target());
		glBindTexture(target(), id());
		
		Graphics::error("binding texture");
	}
	
	void unbind(int unit = 0) {		
		// multitexturing:
		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		
		glBindTexture(target(), 0);
		glDisable(target());
	}
	
	void quad(Graphics& gl, double w=1, double h=1, double x0=0, double y0=0);
	
	// submit manually
	// only safe while OpenGL context exists
	void submit(const Array& src, bool reconfigure=false) {	
		if (src.type() != AlloUInt8Ty) {
			printf("submit failed: only uint8_t arrays are supported\n");
			return;
		} 
		
		if (reconfigure) {
			// reconfigure texture from array
			switch (src.dimcount()) {
				case 1: mTarget = TEXTURE_1D; break;
				case 2: mTarget = TEXTURE_2D; break;
				case 3: mTarget = TEXTURE_3D; break;
				default:
					printf("invalid array dimensions for texture\n");
					return;
			}
			
			switch (src.dimcount()) {
				case 3:	mDepth = src.depth();
				case 2:	mHeight = src.height();
				case 1:	mWidth = src.width(); break;
			}

			switch (src.components()) {
				case 1:	mFormat = Graphics::LUMINANCE; break; // alpha or luminance?
				case 2:	mFormat = Graphics::LUMINANCE_ALPHA; break;
				case 3:	mFormat = Graphics::RGB; break;
				case 4:	mFormat = Graphics::RGBA; break;
				default:
					printf("invalid array component count for texture\n");
					return;
			}
			
			printf("configured to %dD=%X, format %X, align %d\n", src.dimcount(), mTarget, mFormat, src.alignment());
		} 
		else {
			if (src.width() != width()) {
				printf("submit failed: source array width does not match\n");
				return;
			}
			if (height() && src.height() != height()) {
				printf("submit failed: source array height does not match\n");
				return;
			}
			if (depth() && src.depth() != depth()) {
				printf("submit failed: source array depth does not match\n");
				return;
			}
		
			switch (format()) {
				case Graphics::ALPHA:
				case Graphics::LUMINANCE:
					if (src.dimcount() != 1) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				case Graphics::LUMINANCE_ALPHA:
					if (src.dimcount() != 2) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				case Graphics::RGB:
					if (src.dimcount() != 3) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				case Graphics::RGBA:
					if (src.dimcount() != 4) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				default:
					break;
			}
		}
		
		submit(src.data.ptr, src.alignment());
	}
	
	
	/// Resize texture data on GPU and copy over pixels
	
	/// If pixels is NULL, then the only effect is to resize the texture
	/// remotely.
	virtual void submit(const void * pixels=NULL, uint32_t align=4) {
		
		validate();
		
		determineTarget();
		glBindTexture(target(), id());
		
		// set glPixelStore according to the array layout:
		//glPixelStorei(GL_UNPACK_ALIGNMENT, align);
		
		int comps = numComps();
		
		// void glTexImage3D(
		//		GLenum target, GLint level, GLenum internalformat,
		//		GLsizei width, GLsizei height, GLsizei depth, 
		//		GLint border, GLenum format, GLenum type, const GLvoid *pixels
		// );
		switch(mTarget){
		case GL_TEXTURE_1D:	glTexImage1D(mTarget, 0, comps, width(), 0, format(), type(), pixels); break;
		case GL_TEXTURE_2D: glTexImage2D(mTarget, 0, comps, width(), height(), 0, format(), type(), pixels); break;
		case GL_TEXTURE_3D: glTexImage3D(mTarget, 0, comps, width(), height(), depth(), 0, format(), type(), pixels); break;
		default:;
		}
		
		// set alignment back to default
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		
		Graphics::error("submitting texture");
		
//		// OpenGL may have changed the internal format to one it supports:
//		GLint format;
//		glGetTexLevelParameteriv(mTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
//		if (format != mInternalFormat) {
//			printf("converted from %X to %X format\n", mInternalFormat, format);
//			mInternalFormat = format;
//		}

		//printf("submitted texture data %p\n", pixels);
		
		glBindTexture(target(), 0);
	}


	void allocate(){
		deallocate();
		mBuffer = malloc(numElems() * Graphics::numBytes(type()));
		mPixels = mBuffer;
	}

	void deallocate(){
		if(mBuffer){
			free(mBuffer);
			mBuffer=0;
			mPixels=0;
		}
	}

protected:
//	GLint mLevel;	// TODO: on a rainy day...
//	GLint mBorder;
//	GLint mAlignment;
	Target mTarget;				// TEXTURE_1D, TEXTURE_2D, etc.
	Format mFormat;				// RGBA, ALPHA, etc.
	DataType mType;				// UBYTE, FLOAT, etc.
	Wrap mWrapS, mWrapT, mWrapR;	
	Filter mFilter;
	unsigned mWidth, mHeight, mDepth;
	void * mPixels;				// pointer to client-side pixel data (0 if none)
	void * mBuffer;				// internally allocated pixel buffer
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
			//determineTarget(); // moved to submit()
			submit();
			mPixelsUpdated = false;
		}
	}

	void determineTarget(){
		if(0 == mHeight)		mTarget = TEXTURE_1D;
		else if(0 == mDepth)	mTarget = TEXTURE_2D;
		else					mTarget = TEXTURE_3D;
		//invalidate(); // FIXME: mPixelsUpdated flag now triggers update
	}

	// Pattern for setting a variable that when changed sets a notification flag
	template<class T>
	Texture& update(const T& v, T& var, bool& flag){
		if(v!=var){ var=v; flag=true; } return *this; }

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

} // al::

#endif
