#ifndef INC_AL_UTIL_TEXTUREGL_HPP
#define INC_AL_UTIL_TEXTUREGL_HPP

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
	Wrapper to OpenGL Textures

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include <stdio.h>

/*!
	A collection of utilities that are (currently) specific to OpenGL
*/

namespace al {



/*!
	A simple wrapper around OpenGL Textures,
	using al::Array as a CPU-side interface for configuring & submitting

	TODO: lift out common features of TextureGL and CubeMapTexture into a
	generic superclass ?
*/
class TextureGL : public GPUObject {
public:

	TextureGL(unsigned width=512, unsigned height=512, unsigned depth=0)
	:	GPUObject(),
		mTarget(GL_TEXTURE_2D),
		mInternalFormat(GL_RGBA),
		mWrapS(GL_CLAMP_TO_EDGE),
		mWrapT(GL_CLAMP_TO_EDGE),
		mWrapR(GL_CLAMP_TO_EDGE),
//		mType(GL_UNSIGNED_BYTE),
//		mLevel(0),
//		mBorder(0),
//		mAlignment(4),
		mWidth(width),
		mHeight(height),
		mDepth(depth),
		mSubmit(true)
	{
		determineTarget();
	}

	virtual ~TextureGL() {}

	uint32_t width() const { return mWidth; }
	uint32_t height() const { return mHeight; }
	uint32_t depth() const { return mDepth; }

	// format should be one of
	// GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, or GL_LUMINANCE_ALPHA
	TextureGL& format(GLenum format=GL_RGBA) {
		mInternalFormat = format;
		invalidate();
		return *this;
	}

	TextureGL& width(uint32_t v) { mWidth = v; determineTarget(); return *this; }
	TextureGL& height(uint32_t v) { mHeight = v; determineTarget(); return *this; }
	TextureGL& depth(uint32_t v) { mDepth = v; determineTarget(); return *this; }

	// e.g. GL_CLAMP, GL_CLAMP_TO_EDGE, GL_REPEAT
	TextureGL& wrap(GLint mode) {
		mWrapS = mWrapT = mWrapR = mode;
		invalidate();
		return *this;
	}
	TextureGL& wrap(GLint S, GLint T) {
		mWrapS = S;
		mWrapT = T;
		invalidate();
		return *this;
	}
	TextureGL& wrap(GLint S, GLint T, GLint R) {
		mWrapS = S;
		mWrapT = T;
		mWrapR = R;
		invalidate();
		return *this;
	}

	virtual void onCreate() {
		if (mID == 0) {
			//printf("TextureGL onCreate\n");

			glGenTextures(1, (GLuint *)&mID);
			glBindTexture(mTarget, id());

			// TODO: which options?
			glTexParameterf(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(mTarget, GL_TEXTURE_WRAP_S, mWrapS);
			glTexParameterf(mTarget, GL_TEXTURE_WRAP_T, mWrapT);
			glTexParameterf(mTarget, GL_TEXTURE_WRAP_R, mWrapR);
			glTexParameteri(mTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			glBindTexture(mTarget, 0);
			submit();
			Graphics::error("creating texture");
		}
	}

	virtual void onDestroy() {
		if (mID) {
			glDeleteTextures(1, (GLuint *)&mID);
			mID = 0;
		}
	}

	void bind(int unit = 0) {
		// ensure it is created:
		validate();

		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );

		// bind:
		glEnable(mTarget);
		glBindTexture(mTarget, id());

		Graphics::error("binding texturegl");
	}

	void unbind(int unit = 0) {
		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );

		glBindTexture(mTarget, 0);
		glDisable(mTarget);
	}

	void quad(Graphics& gl, double w=1, double h=1, double x0=0, double y0=0) {
		bind();
		gl.pushMatrix();
		gl.translate(x0, y0, 0);
		gl.scale(w, h, 1);
		gl.begin(gl.TRIANGLE_STRIP);
			gl.texCoord	( 0, 0);
			gl.vertex	( 0, 0, 0);
			gl.texCoord	( 1, 0);
			gl.vertex	( 1, 0, 0);
			gl.texCoord	( 0, 1);
			gl.vertex	( 0, 1, 0);
			gl.texCoord	( 1, 1);
			gl.vertex	( 1, 1, 0);
		gl.end();
		gl.popMatrix();
		unbind();
	}

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
				case 1:
					mTarget = GL_TEXTURE_1D;
					break;
				case 2:
					mTarget = GL_TEXTURE_2D;
					break;
				case 3:
					mTarget = GL_TEXTURE_3D;
					break;
				default:
					printf("invalid array dimensions for texture\n");
					return;
			}

			switch (src.dimcount()) {
				case 3:
					mDepth = src.depth();
				case 2:
					mHeight = src.height();
				case 1:
					mWidth = src.width();
					break;
			}

			switch (src.components()) {
				case 1:
					mInternalFormat = GL_LUMINANCE;
					// alpha or luminance?
					break;
				case 2:
					mInternalFormat = GL_LUMINANCE_ALPHA;
					break;
				case 3:
					mInternalFormat = GL_RGB;
					break;
				case 4:
					mInternalFormat = GL_RGBA;
					break;
				default:
					printf("invalid array component count for texture\n");
					return;
			}

			//printf("configured to %dD=%X, format %X, align %d\n", src.dimcount(), mTarget, mInternalFormat, src.alignment());
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

			switch (mInternalFormat) {
				case GL_ALPHA:
				case GL_LUMINANCE:
					if (src.dimcount() != 1) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				case GL_LUMINANCE_ALPHA:
					if (src.dimcount() != 2) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				case GL_RGB:
					if (src.dimcount() != 3) {
						printf("submit failed: source array dimcount does not match\n");
						return;
					}
					break;
				case GL_RGBA:
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

	// submit manually
	// supports the modes valid in GLES 1.0
	virtual void submit(void * pixels=NULL, uint32_t align=4) {

		validate();

		glBindTexture(mTarget, id());

		// set glPixelStore according to the array layout:
		//glPixelStorei(GL_UNPACK_ALIGNMENT, align);

		switch (mTarget) {
			case GL_TEXTURE_1D:
				glTexImage1D(mTarget,
					0,	// GLint level
					mInternalFormat,
					width(),
					0, //GLint border,
					mInternalFormat,
					GL_UNSIGNED_BYTE,
					pixels);
				break;
			case GL_TEXTURE_2D:
				glTexImage2D(mTarget,
					0,	// GLint level
					mInternalFormat,
					width(),
					height(),
					0, //GLint border,
					mInternalFormat,
					GL_UNSIGNED_BYTE,
					pixels);
				break;
			case GL_TEXTURE_3D:
				glTexImage3D(mTarget,
					0,	// GLint level
					mInternalFormat,
					width(),
					height(),
					depth(),
					0, //GLint border,
					mInternalFormat,
					GL_UNSIGNED_BYTE,
					pixels);
				break;
			default:
				printf("texture target not supported yet\n");
				break;
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

		glBindTexture(mTarget, 0);
	}



protected:

	void determineTarget() {
		if (mHeight == 0) {
			mTarget = GL_TEXTURE_1D;
		} else if (mDepth == 0) {
			mTarget = GL_TEXTURE_2D;
		} else {
			mTarget = GL_TEXTURE_3D;
		}
		invalidate();
	}

	GLenum formatFromArray(const Array& src) {
		switch(src.header.components) {
			case 1:	return GL_LUMINANCE;
			case 2:	return GL_LUMINANCE_ALPHA;
			case 3:	return GL_RGB;
			case 4:	return GL_RGBA;
			default:
				printf("warning: unknown type\n");
				return GL_RGBA;
		}
	}

	GLenum typeFromArray(const Array& src) {
		switch(src.header.type) {
			case AlloSInt8Ty:		return GL_BYTE;
			case AlloUInt8Ty:		return GL_UNSIGNED_BYTE;
			case AlloSInt16Ty:		return GL_SHORT;
			case AlloUInt16Ty:		return GL_UNSIGNED_SHORT;
			case AlloSInt32Ty:		return GL_INT;
			case AlloUInt32Ty:		return GL_UNSIGNED_INT;
			case AlloFloat32Ty:		return GL_FLOAT;
			default:
				printf("warning: unknown type\n");
				return GL_UNSIGNED_BYTE;
		}
	}

	GLenum targetFromArray(const Array& src) {
		switch(src.header.dimcount) {
			case 1:		return GL_TEXTURE_1D;
			case 2:		return GL_TEXTURE_2D;
			case 3:		return GL_TEXTURE_3D;
			default:
				printf("warning: unknown dimcount\n");
				return GL_TEXTURE_2D;
		}
	}

//	void configureFromArray(const Array& src) {
//		// derive parameters from array type:
//		mInternalFormat = formatFromArray(src);
////		mType = typeFromArray(src);
//		mTarget = targetFromArray(src);
////		mAlignment = src.alignment();
////		printf("reconfigure from array %x %x %x \n", mInternalFormat, mType, mTarget);
//	}
//
//	void submit() {
//		GLvoid * ptr = (GLvoid *)data();
//		glPixelStorei(GL_UNPACK_ALIGNMENT, mAlignment);
//
//		printf("texture submit id %i target %X level %i components %i dim %ix%i format %X type %X ptr %p alignment %i\n", id(), mTarget, mLevel, mArray.header.components, width(), height(), mFormat, mType, ptr, mAlignment);
//		printf("glTexImage2D target %X level %i components %i dim %ix%i format %X type %X ptr %p alignment %i\n", GL_TEXTURE_2D, 0, 3, 600, 600, GL_RGB, GL_UNSIGNED_BYTE, ptr, 4);
//
//		switch(mTarget) {
//			case GL_TEXTURE_1D:
//				glTexImage1D(
//					mTarget, mLevel, mArray.header.components,
//					width(), mBorder,
//					mFormat, mType, ptr
//				);
//				break;
//			case GL_TEXTURE_2D:
//			//case GL_TEXTURE_RECTANGLE_ARB:
////				glTexSubImage2D (
////					mTarget, mLevel,
////					0, 0, width(), height(),
////					mFormat, mType, ptr);
//				glTexImage2D(
//					mTarget, mLevel, mArray.header.components,
//					width(), height(), mBorder,
//					mFormat, mType, ptr
//				);
//				break;
//			case GL_TEXTURE_3D:
//				glTexImage3D(
//					mTarget, mLevel, mArray.header.components,
//					width(), height(), depth(), mBorder,
//					mFormat, mType, ptr
//				);
//				break;
//
//			default:
//				printf("target not yet handled\n");
//				break;
//		}
//		mSubmit = false;
//
//		GraphicsGL::gl_error("submitting texture");
//
//		// OpenGL may have changed the internal format to one it supports:
//		GLint format;
//		glGetTexLevelParameteriv(mTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
//		if (format != mFormat) {
//			printf("converted from %X to %X format\n", mFormat, format);
//			mFormat = format;
//		}
//
//
//	}

	GLint mTarget;			// GL_TEXTURE_1D, GL_TEXTURE_2D, etc.
	GLint mInternalFormat;	// GL_RGBA, GL_ALPHA etc.
	GLint mWrapS, mWrapT, mWrapR;

//	GLint mType;
//	GLint mLevel;
//	GLint mBorder;
//	GLint mAlignment;

	GLsizei mWidth, mHeight, mDepth;

	bool mSubmit;
};

class CubeMapTexture : public GPUObject {
public:
	// same order as OpenGL
	enum Faces {
		POSITIVE_X, NEGATIVE_X,
		POSITIVE_Y, NEGATIVE_Y,
		POSITIVE_Z, NEGATIVE_Z
	};

	CubeMapTexture(int resolution=1024)
	:	GPUObject(),
		mResolution(resolution),
		mTarget(GL_TEXTURE_CUBE_MAP)
	{
		// create mesh for drawing map:
		mMapMesh.primitive(Graphics::TRIANGLES);
		mMapMesh.color(1,1,1,1);
		int mapSteps =100;
		double step = 1./mapSteps;
		for (double x=0; x<=1.; x+=step) {
			for (double y=0; y<=1.; y+=step) {
				drawMapVertex(x,		y);
				drawMapVertex(x+step,	y);
				drawMapVertex(x,		y+step);

				drawMapVertex(x,		y+step);
				drawMapVertex(x+step,	y);
				drawMapVertex(x+step,	y+step);

			}
		}
	}

	virtual ~CubeMapTexture() {}

	virtual void onCreate() {

		// create cubemap texture:
		glGenTextures(1, (GLuint *)&mID);
		glBindTexture(mTarget, mID);
		// each cube face should clamp at texture edges:
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// normal filtering
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// no mipmapping:
		//glTexParameteri(mTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		//glTexParameterf(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Domagoj also has:
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		float X[4] = { 1,0,0,0 };
		float Y[4] = { 0,1,0,0 };
		float Z[4] = { 0,0,1,0 };
		glTexGenfv( GL_S, GL_OBJECT_PLANE, X );
		glTexGenfv( GL_T, GL_OBJECT_PLANE, Y );
		glTexGenfv( GL_R, GL_OBJECT_PLANE, Z );

		submit();

		// clean up:
		glBindTexture(mTarget, 0);
		Graphics::error("creating cubemap texture");

		//printf("created CubeMapTexture %dx%d\n", mResolution, mResolution);
	}

	virtual void onDestroy() {
		glDeleteTextures(1, (GLuint *)&mID);
	}

	void bind(int unit = 0) {
		// ensure it is created:
		validate();

		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );

		glEnable(mTarget);
		glBindTexture(mTarget, id());
	}

	void unbind(int unit = 0) {
		// multitexturing:
		glActiveTextureARB( GL_TEXTURE0_ARB+unit );

		glBindTexture(mTarget, 0);
		glDisable(mTarget);
	}

	unsigned resolution() const { return mResolution; }

	// useful for debugging
	// draws full cubemap in a cylindrical projection
	void drawMap(Graphics& gl, double x0=0., double y0=0., double x1=1., double y1=1.) {
		bind();
		gl.color(1, 1, 1, 1);
		gl.pushMatrix();
		gl.translate(x0, y0, 0);
		gl.scale((x1-x0), (y1-y0), 1.);
		gl.draw(mMapMesh);
		gl.popMatrix();
		unbind();
	}

protected:
	inline void drawMapVertex(double x, double y) {
		// x runs 0..1, convert to angle -PI..PI:
		double az = M_PI * (x*2.-1.);
		// y runs 0..1, convert to angle -PI_2..PI_2:
		double el = M_PI * 0.5 * (y*2.-1.);
		// convert polar to normal:
		double x1 = sin(az);
		double y1 = sin(el);
		double z1 = cos(az);
		Vec3d v(x1, y1, z1);
		v.normalize();
		mMapMesh.texCoord	( v );
		mMapMesh.vertex	( x, y, 0);
	}

	void submit() {
		// RGBA8 Cubemap texture, 24 bit depth texture, mResolution x mResolution
		// NULL means reserve texture memory, but texels are undefined
		for (int i=0; i<6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA8, mResolution, mResolution, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
		}
	}

	unsigned mResolution;
	Mesh mMapMesh;

	GLenum mTarget;
};


} // al::

#endif
