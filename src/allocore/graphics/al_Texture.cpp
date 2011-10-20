#include <stdlib.h>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Texture.hpp"

namespace al{

Texture :: Texture(unsigned width, unsigned height, Graphics::Format format, Graphics::DataType type)
:	GPUObject(),
	mTarget(TEXTURE_2D),
	mFormat(format),
	mType(type),
	mWrapS(CLAMP_TO_EDGE),
	mWrapT(CLAMP_TO_EDGE),
	mWrapR(CLAMP_TO_EDGE),
	mFilter(LINEAR),
	mWidth(width),
	mHeight(height),
	mDepth(0),
	mUnpack(1),
	mPixels(0), 
	//mBuffer(0),
	mParamsUpdated(true),
	mPixelsUpdated(true)
{
	resetArray(mUnpack);
	
	//printf("created Texture %p\n", this);
}

Texture :: Texture(unsigned width, unsigned height, unsigned depth, Graphics::Format format, Graphics::DataType type)
:	GPUObject(),
	mTarget(TEXTURE_3D),
	mFormat(format),
	mType(type),
	mWrapS(CLAMP_TO_EDGE),
	mWrapT(CLAMP_TO_EDGE),
	mWrapR(CLAMP_TO_EDGE),
	mFilter(LINEAR),
	mWidth(width),
	mHeight(height),
	mDepth(depth),
	mUnpack(1),
	mPixels(0), 
	//mBuffer(0),
	mParamsUpdated(true),
	mPixelsUpdated(true)
{
	resetArray(mUnpack);
	
	//printf("created Texture %p\n", this);
}

void Texture :: determineTarget(){
	if(0 == mHeight)		mTarget = TEXTURE_1D;
	else if(0 == mDepth)	mTarget = TEXTURE_2D;
	else					mTarget = TEXTURE_3D;
	//invalidate(); // FIXME: mPixelsUpdated flag now triggers update
}

void Texture :: quad(Graphics& gl, double w, double h, double x0, double y0){
	bind();
	//gl.color(1, 1, 1, 1);
	gl.pushMatrix();
	gl.translate(x0, y0, 0);
	gl.scale(w, h, 1);
	gl.begin(gl.QUADS);
		gl.texCoord	( 0, 0);
		gl.vertex	( 0, 0, 0);
		gl.texCoord	( 0, 1);
		gl.vertex	( 0, 1, 0);
		gl.texCoord	( 1, 1);
		gl.vertex	( 1, 1, 0);
		gl.texCoord	( 1, 0);
		gl.vertex	( 1, 0, 0);
	gl.end();
	gl.popMatrix();
	unbind();
}

void Texture :: resetArray(unsigned align) {
	//printf("resetArray %p\n", this);
	deallocate();
	
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


void Texture :: allocate(unsigned align) {
	deallocate();
	resetArray(align);
	mArray.dataCalloc();
	mPixels = mArray.data.ptr;
	mPixelsUpdated = true;
	
	//mBuffer = malloc(numElems() * Graphics::numBytes(type()));
	//mPixels = mBuffer;
}

void Texture :: allocate(const Array& src, bool reconfigure) {
	
	if (reconfigure) {
		
		//printf("allocating & reconfiguring %p from\n", this); src.print();
		
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
			default:
				printf("texture array must have 1, 2 or 3 dimenions\n");
				return;
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
		
		mArray.format(src);
		
		
		//printf("allocating & reconfigured %p\n", this); mArray.print();
		
		// re-allocate array:
		allocate(src.alignment());
		
		//printf("allocated & reconfigured %p\n", this);
		//mArray.print();
		
	} else {
		
		// TODO: read the source into the dst without changing dst layout
		//printf("allocating without reconfiguring %p\n", this);
		
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
	
	//src.print();
	//mArray.print();
	
	// copy data:
	memcpy(mArray.data.ptr, src.data.ptr, src.size());
	mPixels = mArray.data.ptr;
	
	//printf("copied to mArray %p\n", this);
}

void Texture :: deallocate() {
	mArray.dataFree();
	mPixels=0;
}

void Texture :: submit(const Array& src, bool reconfigure) {	
//	if (src.type() != AlloUInt8Ty) {
//		printf("submit failed: only uint8_t arrays are supported\n");
//		return;
//	} 
	
	if (reconfigure) {
		// reconfigure texture from array
		switch (src.dimcount()) {
			case 1: target(TEXTURE_1D); break;
			case 2: target(TEXTURE_2D); break;
			case 3: target(TEXTURE_3D); break;
			default:
				printf("invalid array dimensions for texture\n");
				return;
		}
		
		switch (src.dimcount()) {
			case 3:	depth(src.depth());
			case 2:	height(src.height());
			case 1:	width(src.width()); break;
		}

		switch (src.components()) {
			case 1:	format(Graphics::LUMINANCE); break; // alpha or luminance?
			case 2:	format(Graphics::LUMINANCE_ALPHA); break;
			case 3:	format(Graphics::RGB); break;
			case 4:	format(Graphics::RGBA); break;
			default:
				printf("invalid array component count for texture\n");
				return;
		}
		
		switch (src.type()) {
			case AlloUInt8Ty:	type(Graphics::UBYTE); break; 
			case AlloSInt8Ty:	type(Graphics::BYTE); break; 
			case AlloUInt16Ty:	type(Graphics::SHORT); break; 
			case AlloSInt16Ty:	type(Graphics::USHORT); break; 
			case AlloUInt32Ty:	type(Graphics::INT); break; 
			case AlloSInt32Ty:	type(Graphics::UINT); break; 
			case AlloFloat32Ty:	type(Graphics::FLOAT); break; 
			case AlloFloat64Ty:	type(Graphics::DOUBLE); break; 
			default:
				printf("invalid array type for texture\n");
				return;
		}
		
		// reformat internal array to match:
		if (!mArray.isFormat(src)) mArray.format(src);
		
		//printf("configured to target=%X(%dD), type=%X(%X), format=%X, align=(%d)\n", mTarget, src.dimcount(), type(), src.type(), mFormat, src.alignment());
	} 
	else {
		
		if (src.width() != width()) {
			printf("submit failed: source array width does not match\n");
			return;
		}
		if (target() != TEXTURE_1D) {
			if (height() && src.height() != height()) {
				printf("submit failed: source array height does not match\n");
				return;
			}
			if (target() == TEXTURE_3D) {
				if (depth() && src.depth() != depth()) {
					printf("submit failed: source array depth does not match\n");
					return;
				}
			}
		}
		
		if (Graphics::toDataType(src.type()) != type()) {
			printf("submit failed: source array type does not match texture\n");
			return;
		}
	
		switch (format()) {
			case Graphics::ALPHA:
			case Graphics::LUMINANCE:
				if (src.components() != 1) {
					printf("submit failed: source array component count does not match (got %d, should be 1)\n", src.components());
					return;
				}
				break;
			case Graphics::LUMINANCE_ALPHA:
				if (src.components() != 2) {
					printf("submit failed: source array component count does not match (got %d, should be 2)\n", src.components());
					return;
				}
				break;
			case Graphics::RGB:
				if (src.components() != 3) {
					printf("submit failed: source array component count does not match (got %d, should be 3)\n", src.components());
					return;
				}
				break;
			case Graphics::RGBA:
				if (src.components() != 4) {
					printf("submit failed: source array component count does not match (got %d, should be 4)\n", src.components());
					return;
				}
				break;
			default:
				break;
		}
	}
	
	submit(src.data.ptr, src.alignment());
}

void Texture :: submit(const void * pixels, uint32_t align) {
	//printf("submitting %p\n", this);

	validate();
	
	determineTarget();	// is this necessary? surely the target is already set!
	glBindTexture(target(), id());
	
	// set glPixelStore according to layout:
	glPixelStorei(GL_UNPACK_ALIGNMENT, mUnpack);
	
	// void glTexImage3D(
	//		GLenum target, GLint level, GLenum internalformat,
	//		GLsizei width, GLsizei height, GLsizei depth, 
	//		GLint border, GLenum format, GLenum type, const GLvoid *pixels
	// );
	
	// internal format is important
	// TODO: complete the derivation, probably do it elsewhere...
	int internalformat;
	if (type() == Graphics::FLOAT || type() == Graphics::DOUBLE) {
		switch (numComponents()) {
			case 1:
				internalformat = GL_LUMINANCE32F_ARB;
			case 2:
				internalformat = GL_LUMINANCE_ALPHA32F_ARB;
			case 3:
				internalformat = GL_RGB32F_ARB;
			case 4:
				internalformat = GL_RGBA32F_ARB;
				break;
			default:
				break;
		}
	} else {
		// the old way - let the GPU decide:
		internalformat = numComponents();
	}
	switch(mTarget){
		case GL_TEXTURE_1D:	glTexImage1D(mTarget, 0, internalformat, width(), 0, format(), type(), pixels); break;
		case GL_TEXTURE_2D: glTexImage2D(mTarget, 0, internalformat, width(), height(), 0, format(), type(), pixels); break;
		case GL_TEXTURE_3D: glTexImage3D(mTarget, 0, internalformat, width(), height(), depth(), 0, format(), type(), pixels); break;
		default:;
	}
	
	// set alignment back to default
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
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

} // al::
