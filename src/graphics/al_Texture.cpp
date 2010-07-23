#include <stdlib.h>
#include "protocol/al_GraphicsBackend.hpp"
#include "graphics/al_Config.h"
#include "graphics/al_Texture.hpp"

namespace al {
namespace gfx{


Texture::Texture(GraphicsBackend *backend)
:	GPUObject(),
	mBackend(backend),
	mMode(DATA),
	mRebuild(true),
	mUpdate(false),
	mRect(true),
	mWidth(512),
	mHeight(512),
	mDepth(0),
	mTarget(TEXTURE_RECT),
	mFormat(RGBA),
	mType(UCHAR),
	mWrap(REPEAT),
	mMinFilter(LINEAR),
	mMagFilter(LINEAR),
	mBorderColor(0, 0, 0, 1)
{}

Texture::~Texture() {
}
	
void Texture::bind(int unit) {
	if(mRebuild) {
		destroy();
	}

	if(! created()) {
		create();
		mRebuild = false;
	}

	mBackend->textureBind(this, unit);
	

	if(mUpdate) {
		mBackend->textureSubmit(this);
		mUpdate = false;
	}
	
	mBackend->textureEnter(this, unit);
}

void Texture::unbind(int unit) {
	mBackend->textureLeave(this, unit);
	mBackend->textureUnbind(this, unit);
}

void Texture::setLatticeFormat(AlloLatticeHeader &header) {
	mFormat = format_for_lattice_components(header.components);
	mType = type_for_lattice_type(header.type);

	mWidth = header.dim[0];
	mHeight = header.dim[1];
	mDepth = header.dim[2];
	
	mTarget = target_for_lattice_dimcount(header.dimcount);
}

void Texture::fromLattice(al::Lattice *lattice) {
	if(! mLattice.equal(lattice->header) || mMode != DATA) {
		mMode = DATA;
		setLatticeFormat(lattice->header);
		mLattice.adapt(lattice);
		memcpy(mLattice.data.ptr, lattice->data.ptr, mLattice.size());
		mRebuild = true;
	}
	
	mUpdate = true;
}
	
bool Texture::rect() {
	return mRect;
}

void Texture::rect(bool v) {
	mRect = v;
}
	
char * Texture::getData() {
	return mLattice.data.ptr;
}

int Texture::getRowStride() {
	return mLattice.header.stride[1];
}

int Texture::width() {
	return mWidth;
}

void Texture::width(int w) {
	if(mWidth != w) {
		mWidth = w;
		mRebuild = true;
	}
}

int Texture::height() {
	return mHeight;
}

void Texture::height(int h) {
	if(mHeight != h) {
		mHeight = h;
		mRebuild = true;
	}
}

int Texture::depth() {
	return mDepth;
}

void Texture::depth(int d) {
	if(mDepth != d) {
		mDepth = d;
		mRebuild = true;
	}
}

void Texture::getDimensions(int &w, int &h) {
	w = mWidth; h = mHeight;;
}

void Texture::getDimensions(int &w, int &h, int &d) {
	w = mWidth; h = mHeight; d = mDepth;
}

void Texture::dimensions(int w, int h) {
	if(mWidth != w || mHeight != h) {
		mWidth = w;
		mHeight = h;
		mRebuild = true;
	}
}

void Texture::dimensions(int w, int h, int d) {
	if(mWidth != w || mHeight != h || mDepth != d) {
		mWidth = w;
		mHeight = h;
		mDepth = d;
		mRebuild = true;
	}
}

Texture::Mode Texture::mode() {
	return mMode;
}

void Texture::mode(Mode v) {
	if(mMode != v) {
		mMode = v;
		mRebuild = true;
		
		if(mMode == SURFACE) {
			mUpdate = false;
		}
	}
}

Texture::Target Texture::target() {
	return mTarget;
}

void Texture::target(Target v) {
	if(mTarget != v) {
		mTarget = v;
		mRebuild = true;
	}
}

Texture::Format Texture::format() {
	return mFormat;
}

void Texture::format(Format v) {
	if(mFormat != v) {
		mFormat = v;
		mRebuild = true;
	}
}

Texture::Type Texture::type() {
	return mType;
}

void Texture::type(Type v) {
	if(mType != v) {
		mType = v;
		mRebuild = true;
	}
}

Texture::Wrap Texture::wrap() {
	return mWrap;
}

void Texture::wrap(Wrap v) {
	if(mWrap != v) {
		mWrap = v;
		mRebuild = true;
	}
}

Texture::Filter Texture::minFilter() {
	return mMinFilter;
}

void Texture::minFilter(Filter v) {
	if(mMinFilter != v) {
		mMinFilter = v;
		mRebuild = true;
	}
}

Texture::Filter Texture::magFilter() {
	return mMagFilter;
}

void Texture::magFilter(Filter v) {
	if(mMagFilter != v) {
		mMagFilter = v;
		mRebuild = true;
	}
}


void Texture::onCreate() {
	mBackend->textureCreate(this);
}

void Texture::onDestroy() {
	mBackend->textureDestroy(this);
}

Texture::Format Texture::format_for_lattice_components(int components) {
	switch(components) {
		case 1:	return LUMINANCE;
		case 2:	return LUMALPHA;
		case 3:	return RGB;
		case 4:	
		default:
			return RGBA;
	}
}

Texture::Type Texture::type_for_lattice_type(AlloTy type) {
	switch(type) {
		case AlloUInt8Ty:		return UCHAR;
		case AlloSInt32Ty:		return INT;
		case AlloUInt32Ty:		return UINT;
		case AlloPointer32Ty:	return FLOAT32;
		default:
			return UCHAR;
	}
}

Texture::Target Texture::target_for_lattice_dimcount(int dimcount) {
	switch(dimcount) {
		case 1:	return TEXTURE_1D;
		case 3:	return TEXTURE_3D;
		
		case 2:
		default:
			return mRect ? TEXTURE_RECT : TEXTURE_2D;
	}
}

} // ::al::gfx
} // ::al


//#include <stdlib.h>
//#include "graphics/al_Config.h"
//#include "graphics/al_Texture.hpp"
//
//namespace al {
//namespace gfx{
//
//
//TextureBase::TextureBase(ColorFormat::t format, DataType::t dataType, WrapMode::t wm)
//:	mPixels(0), mBuffer(0),
//	mFormat(format), mIpol(IpolMode::Linear), mType(dataType), mWrap(wm)
//{}
//
//TextureBase::~TextureBase(){
//	destroy();
//	freeMem();
//}
//
//// This creates an internal buffer and points mPixels to it
//void TextureBase::allocMem(){
//	freeMem();
//	mBuffer = malloc(numComponents(format()) * numBytes(dataType()) * size());
//	mPixels = mBuffer;
//}
//
//const TextureBase& TextureBase::begin() const { glBindTexture(target(), id()); return *this; }
//const TextureBase& TextureBase::bind() const { return begin(); }
//void TextureBase::end() const { glBindTexture(target(), 0); }
//
//void TextureBase::freeMem(){
//	if(mBuffer) free(mBuffer); mBuffer=0;
//}
//
//const TextureBase& TextureBase::send() const{
//	begin();
//	texSubImage();
//	end();
//	return *this;
//}
//
//TextureBase& TextureBase::dataType(DataType::t v){ mType=v; return *this; }
//TextureBase& TextureBase::format(ColorFormat::t v){ mFormat=v; return *this; }
//TextureBase& TextureBase::ipolMode(IpolMode::t v){ mIpol=v; return *this; }
//TextureBase& TextureBase::wrapMode(WrapMode::t v){ mWrap=v; return *this; }
//
//void TextureBase::onCreate(){
//	GLuint glID = (GLuint)mID;
//	glGenTextures(1, &glID); //printf("%i\n", mID);
//	bind();
//	texImage();
//	glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, ipolMode());
//	glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, ipolMode());
//	texWrap();
//	send();
//}
//
//void TextureBase::onDestroy(){
//	GLuint glID = (GLuint)mID;
//	glDeleteTextures(1, &glID);
//}
//
//
//Texture2::Texture2(int w, int h, ColorFormat::t format, DataType::t type, WrapMode::t wrap)
//:	TextureBase(format, type, wrap), w(w), h(h)
//{	allocMem(); }
//
//Texture2& Texture2::draw(
//	float ql, float qt, float qr, float qb,
//	float tl, float tt, float tr, float tb)
//{	
//	glBegin(GL_QUADS);       
//		glTexCoord2f(tl,tt); glVertex2f(ql,qt);
//		glTexCoord2f(tl,tb); glVertex2f(ql,qb);
//		glTexCoord2f(tr,tb); glVertex2f(qr,qb);
//		glTexCoord2f(tr,tt); glVertex2f(qr,qt);
//	glEnd();
//	return *this;
//}
//
//Texture2& Texture2::load(int width, int height, void * pixs){
//	w = width;
//	h = height;
//	mPixels = pixs;
//	create();
//	return *this;
//}
//
//int Texture2::target() const { return GL_TEXTURE_2D; }
//void Texture2::texImage() const {
//	glTexImage2D(GL_TEXTURE_2D, 0, numComponents(format()), w, h, 0, format(), dataType(), mPixels);
//}
//void Texture2::texSubImage() const {
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format(), dataType(), mPixels);
//}
//void Texture2::texWrap() const {
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode());
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode());
//}
//
//
//
//Texture3::Texture3(int _w, int _h, int _d, ColorFormat::t format, DataType::t type, WrapMode::t wrap)
//:	TextureBase(format, type, wrap), w(_w), h(_h), d(_d)
//{	allocMem(); }
//
//Texture3& Texture3::load(int width, int height, int depth, void * pixs){
//	w = width;
//	h = height;
//	d = depth;
//	mPixels = pixs;
//	create();
//	return *this;
//}
//
//int Texture3::target() const { return GL_TEXTURE_3D; }
//void Texture3::texImage() const {
//	glTexImage3D(GL_TEXTURE_3D, 0, numComponents(format()), w, h, d, 0, format(), dataType(), mPixels);
//}
//void Texture3::texSubImage() const {
//	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, d, format(), dataType(), mPixels);
//}
//void Texture3::texWrap() const {
//	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapMode());
//	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapMode());
//	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapMode());	
//}
//
//} // ::al::gfx
//} // ::al
