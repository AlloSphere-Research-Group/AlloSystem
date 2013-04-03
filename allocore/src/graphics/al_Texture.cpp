#include <stdlib.h>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Texture.hpp"

namespace al{

Texture :: Texture(
	unsigned width, unsigned height,
	Graphics::Format format, Graphics::DataType type,
	bool alloc
)
:	GPUObject(),
	mTarget(TEXTURE_2D),
	mFormat(format),
	mTexelFormat(0),
	mType(type),
	mWrapS(CLAMP_TO_EDGE),
	mWrapT(CLAMP_TO_EDGE),
	mWrapR(CLAMP_TO_EDGE),
	mFilterMin(LINEAR),
	mFilterMag(LINEAR),
	mWidth(width),
	mHeight(height),
	mDepth(0),
	mUnpack(1),
	mParamsUpdated(true),
	mPixelsUpdated(true),
	mOwnsData(0) 
{
	if(alloc) allocate();
}

Texture :: Texture(
	unsigned width, unsigned height, unsigned depth,
	Graphics::Format format, Graphics::DataType type,
	bool alloc
)
:	GPUObject(),
	mTarget(TEXTURE_3D),
	mFormat(format),
	mTexelFormat(0),
	mType(type),
	mWrapS(CLAMP_TO_EDGE),
	mWrapT(CLAMP_TO_EDGE),
	mWrapR(CLAMP_TO_EDGE),
	mFilterMin(LINEAR),
	mFilterMag(LINEAR),
	mWidth(width),
	mHeight(height),
	mDepth(depth),
	mUnpack(1),
	mParamsUpdated(true),
	mPixelsUpdated(true),
	mOwnsData(0) 
{
	if(alloc) allocate();
}

Texture :: Texture(AlloArrayHeader& header) 
:	GPUObject(),
	mTexelFormat(0),
	mWrapS(CLAMP_TO_EDGE),
	mWrapT(CLAMP_TO_EDGE),
	mWrapR(CLAMP_TO_EDGE),
	mFilterMin(LINEAR),
	mFilterMag(LINEAR),
	mUnpack(1),
	mParamsUpdated(true),
	mPixelsUpdated(true),
	mOwnsData(0) 
{
	configure(header);
	mArray.dataCalloc();
}

Texture :: ~Texture() {
	if (!mOwnsData) mArray.data.ptr = 0;
}

void Texture::onCreate(){
	//printf("Texture onCreate\n");
	glGenTextures(1, (GLuint *)&mID);
	sendParams();
	sendPixels();
	AL_GRAPHICS_ERROR("creating texture", id());
}
	
void Texture::onDestroy(){
	glDeleteTextures(1, (GLuint *)&mID);
}

void Texture :: configure(AlloArrayHeader& header) {
	switch (header.dimcount) {
		case 1: target(TEXTURE_1D); break;
		case 2: target(TEXTURE_2D); break;
		case 3: target(TEXTURE_3D); break;
		default:
			AL_WARN("invalid array dimensions for texture");
			return;
	}
	
	switch (header.dimcount) {
		case 3:	depth(header.dim[2]);
		case 2:	height(header.dim[1]);
		case 1:	width(header.dim[0]); break;
	}

	switch (header.components) {
		case 1:	format(Graphics::LUMINANCE); break; // alpha or luminance?
		case 2:	format(Graphics::LUMINANCE_ALPHA); break;
		case 3:	format(Graphics::RGB); break;
		case 4:	format(Graphics::RGBA); break;
		default:
			AL_WARN("invalid array component count for texture");
			return;
	}
	
	switch (header.type) {
		case AlloUInt8Ty:	type(Graphics::UBYTE); break; 
		case AlloSInt8Ty:	type(Graphics::BYTE); break; 
		case AlloUInt16Ty:	type(Graphics::SHORT); break; 
		case AlloSInt16Ty:	type(Graphics::USHORT); break; 
		case AlloUInt32Ty:	type(Graphics::INT); break; 
		case AlloSInt32Ty:	type(Graphics::UINT); break; 
		case AlloFloat32Ty:	type(Graphics::FLOAT); break; 
		case AlloFloat64Ty:	type(Graphics::DOUBLE); break; 
		default:
			AL_WARN("invalid array type for texture");
			return;
	}
	
	// reconfigure internal array to match:
	mArray.configure(header);
	
	mParamsUpdated = true; 
}

void Texture :: bind(int unit) {
	// ensure it is created:
	AL_GRAPHICS_ERROR("(before Texture::bind)", id());
	validate();
	//AL_GRAPHICS_ERROR("validate binding texture", id());
	sendParams(false);
	//AL_GRAPHICS_ERROR("sendparams binding texture", id());
	sendPixels(false);
	//AL_GRAPHICS_ERROR("sendpixels binding texture", id());
	
	// multitexturing:
	glActiveTexture(GL_TEXTURE0 + unit);
	//AL_GRAPHICS_ERROR("active texture binding texture", id());

	// bind:
	glEnable(target());
	//AL_GRAPHICS_ERROR("enable target binding texture", id());
	glBindTexture(target(), id());
	AL_GRAPHICS_ERROR("binding texture", id());
}

void Texture :: unbind(int unit) {		
	// multitexturing:
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target(), 0);
	glDisable(target());
}

void Texture :: determineTarget(){
	if(0 == mHeight)		mTarget = TEXTURE_1D;
	else if(0 == mDepth)	mTarget = TEXTURE_2D;
	else					mTarget = TEXTURE_3D;
	//invalidate(); // FIXME: mPixelsUpdated flag now triggers update
}

void Texture :: quad(Graphics& gl, double w, double h, double x0, double y0){	
	//Graphics::error(id(), "prebind quad texture");
	bind();	
	Mesh& m = gl.mesh();
	m.reset();
	//Graphics::error(id(), "reset mesh quad texture");
	m.primitive(gl.TRIANGLE_STRIP);
		m.texCoord	( 0, 0);
		m.vertex	(x0, y0, 0);
		m.texCoord	( 1, 0);
		m.vertex	(x0+w, y0, 0);
		m.texCoord	( 0, 1);
		m.vertex	(x0, y0+h, 0);
		m.texCoord	( 1, 1);
		m.vertex	(x0+w, y0+h, 0);
	//Graphics::error(id(), "set mesh quad texture");
	gl.draw(m);
	//Graphics::error(id(), "draw mesh quad texture");
	unbind();
}

void Texture::quadViewport(
	Graphics& g, const Color& color,
	double w, double h, double x, double y
){
	g.pushMatrix(g.PROJECTION);
	g.loadIdentity();
	g.pushMatrix(g.MODELVIEW);
	g.loadIdentity();
	g.depthMask(0); // write only to color buffer
		g.color(color);
		quad(g, w,h, x,y);
	g.depthMask(1);	
	g.popMatrix(g.PROJECTION);
	g.popMatrix(g.MODELVIEW);
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
	mPixelsUpdated = true;
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
				AL_WARN("invalid array dimensions for texture");
				return;
		}
		
		// reconfigure size:
		switch (src.dimcount()) {
			case 3:	depth(src.depth());
			case 2:	height(src.height());
			case 1:	width(src.width()); break;
			default:
				AL_WARN("texture array must have 1, 2 or 3 dimenions");
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
					AL_WARN("invalid array component count for texture");
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
			AL_WARN("couldn't allocate array, mismatch format");
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
	
	//printf("copied to mArray %p\n", this);
}

void Texture :: deallocate() {
	mArray.dataFree();
}

void Texture::sendParams(bool force){
	if(mParamsUpdated || force){
		glBindTexture(target(), id());
		glTexParameterf(target(), GL_TEXTURE_MAG_FILTER, filterMag());
		glTexParameterf(target(), GL_TEXTURE_MIN_FILTER, filterMin());
		glTexParameterf(target(), GL_TEXTURE_WRAP_S, mWrapS);
		glTexParameterf(target(), GL_TEXTURE_WRAP_T, mWrapT);
		glTexParameterf(target(), GL_TEXTURE_WRAP_R, mWrapR);
		if (filterMin() != LINEAR && filterMin() != NEAREST) {
			glTexParameteri(target(), GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		}
		glBindTexture(target(), 0);
		mParamsUpdated = false;
	}
}

void Texture::sendPixels(bool force){
	if(mPixelsUpdated || force){
		//printf("%p submitting %p\n", this, mArray.data.ptr);
		submit(mArray.data.ptr);
		mPixelsUpdated = false;
	}
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
				AL_WARN("invalid array dimensions for texture");
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
				AL_WARN("invalid array component count for texture");
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
				AL_WARN("invalid array type for texture");
				return;
		}
		
		// reformat internal array to match:
		if (!mArray.isFormat(src)) mArray.format(src);
		
		//printf("configured to target=%X(%dD), type=%X(%X), format=%X, align=(%d)\n", mTarget, src.dimcount(), type(), src.type(), mFormat, src.alignment());
	} 
	else {
		
		if (src.width() != width()) {
			AL_WARN("submit failed: source array width does not match");
			return;
		}
		if (target() != TEXTURE_1D) {
			if (height() && src.height() != height()) {
				AL_WARN("submit failed: source array height does not match");
				return;
			}
			if (target() == TEXTURE_3D) {
				if (depth() && src.depth() != depth()) {
					AL_WARN("submit failed: source array depth does not match");
					return;
				}
			}
		}
		
		if (Graphics::toDataType(src.type()) != type()) {
			AL_WARN("submit failed: source array type does not match texture");
			return;
		}
	
		switch (format()) {
			case Graphics::ALPHA:
			case Graphics::LUMINANCE:
				if (src.components() != 1) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 1)", src.components());
					return;
				}
				break;
			case Graphics::LUMINANCE_ALPHA:
				if (src.components() != 2) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 2)", src.components());
					return;
				}
				break;
			case Graphics::RGB:
				if (src.components() != 3) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 3)", src.components());
					return;
				}
				break;
			case Graphics::RGBA:
				if (src.components() != 4) {
					AL_WARN("submit failed: source array component count does not match (got %d, should be 4)", src.components());
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
	AL_GRAPHICS_ERROR("(before Texture::submit)", id());
	validate();
	
	determineTarget();	// is this necessary? surely the target is already set!
	
	sendParams(false);
	
	glActiveTexture(GL_TEXTURE0);
	AL_GRAPHICS_ERROR("Texture::submit (glActiveTexture)", id());
	glEnable(target());
	AL_GRAPHICS_ERROR("Texture::submit (glEnable(texture target))", id());
	glBindTexture(target(), id());
	AL_GRAPHICS_ERROR("Texture::submit (glBindTexture)", id());
	
	// set glPixelStore according to layout:
	glPixelStorei(GL_UNPACK_ALIGNMENT, mUnpack);
	AL_GRAPHICS_ERROR("Texture::submit (glPixelStorei set)", id());
	
	// void glTexImage3D(
	//		GLenum target, GLint level, GLenum internalformat,
	//		GLsizei width, GLsizei height, GLsizei depth, 
	//		GLint border, GLenum format, GLenum type, const GLvoid *pixels
	// );
	
//	// internal format is important
//	// TODO: complete the derivation, probably do it elsewhere...
//	if(type() == Graphics::FLOAT || type() == Graphics::DOUBLE){
//		switch(numComponents()){
//			case 1: intFmt = GL_LUMINANCE32F_ARB; break;
//			case 2: intFmt = GL_LUMINANCE_ALPHA32F_ARB; break;
//			case 3: intFmt = GL_RGB32F_ARB; break;
//			case 4: intFmt = GL_RGBA32F_ARB; break;
//			default:;
//		}
//	} else {
//		// the old way - let the GPU decide:
//		intFmt = numComponents();
//	}

	int intFmt;

	// Use specified texel format, if defined
	if(mTexelFormat){
		intFmt = mTexelFormat;
	}
	
	// Derive internal texel format from texture data format.
	// By default, we can just use the texture data format. In cases where
	// there is no corresponding texel format, just hand in the number of
	// components.
	else{
		if(	format() == Graphics::RED ||
			format() == Graphics::GREEN ||
			format() == Graphics::BLUE
		){
			intFmt = 1;
		}
		else if(format() == Graphics::BGRA){
			intFmt = 4;
		}
		else{
			intFmt = format();
		}	
	}

	switch(mTarget){
		case GL_TEXTURE_1D:
			glTexImage1D(mTarget, 0, intFmt, width(), 0, format(), type(), pixels);
			break;
		case GL_TEXTURE_2D:
			glTexImage2D(mTarget, 0, intFmt, width(), height(), 0, format(), type(), pixels);
			break;
		case GL_TEXTURE_3D:
			glTexImage3D(mTarget, 0, intFmt, width(), height(), depth(), 0, format(), type(), pixels);
			break;
		default:
			AL_WARN("invalid texture target %d", mTarget);
	}
	AL_GRAPHICS_ERROR("Texture::submit (glTexImage)", id());
	
	// set alignment back to default
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	AL_GRAPHICS_ERROR("Texture::submit (glPixelStorei unset)", id());
	
//		// OpenGL may have changed the internal format to one it supports:
//		GLint format;
//		glGetTexLevelParameteriv(mTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
//		if (format != mInternalFormat) {
//			printf("converted from %X to %X format\n", mInternalFormat, format);
//			mInternalFormat = format;
//		}

	//printf("submitted texture data %p\n", pixels);
	
	glDisable(target());
	glBindTexture(target(), 0);
	AL_GRAPHICS_ERROR("Texture::submit (glBindTexture 0)", id());
}


void Texture :: print() {

	const char * target = "?";
	const char * format = "?";
	const char * type = "?";
	
	switch (mTarget) {
		case TEXTURE_1D: 
			target = "GL_TEXTURE_1D"; 
			printf("Texture target=%s, %d(%d), ", target, width(), mArray.width());
			break;
		case TEXTURE_2D: 
			target = "GL_TEXTURE_2D";
			printf("Texture target=%s, %dx%d(%dx%d), ", target, width(), height(), mArray.width(), mArray.height()); 
			break;
		case TEXTURE_3D: 
			target = "GL_TEXTURE_3D"; 
			printf("Texture target=%s, %dx%dx%d(%dx%dx%d), ", target, width(), height(), depth(), mArray.width(), mArray.height(), mArray.depth());
			break;
		default:;
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
		default:;
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
		default:;
	}
	
	printf("type=%s(%s), format=%s(%d), unpack=%d(align=%d))\n", type, allo_type_name(mArray.type()), format, mArray.components(), mUnpack, mArray.alignment());
	//mArray.print();
}


Texture& Texture::updatePixels(){
	AL_WARN_ONCE("Texture::updatePixels() deprecated, use Texture::dirty()");
	return dirty();
}

void Texture::submit(){
	AL_WARN_ONCE("Texture::submit() deprecated, use Texture::dirty()");
	dirty();
}


} // al::
