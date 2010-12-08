#include <stdlib.h>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Texture.hpp"

namespace al{

Texture::Texture(Graphics *backend)
:	GPUObject(),
	mBackend(backend),
	mSurface(0),
	mMode(DATA),
	mRebuild(true),
	mUpdate(false),
	mRect(true),
	mWidth(512),
	mHeight(512),
	mDepth(0),
	mTarget(TEXTURE_2D),
	mFormat(RGBA),
	mType(UCHAR),
	mWrap(REPEAT),
	mMinFilter(LINEAR),
	mMagFilter(LINEAR),
	mBorderColor(0, 0, 0, 1)
{}

Texture::~Texture() {
}

void Texture::attach(Surface *s) {
	mode(Texture::SURFACE);
	mSurface = s;
}

void Texture::clear(int unit, bool do_bind, bool clear_data) {
	if(do_bind) {
		bind(unit);
	}
	
	if(mMode == Texture::SURFACE) {
		if(mSurface) {
			mSurface->enter();
			mSurface->leave();
		}
	}
	else {
		// clear lattice, upload data
		AlloArrayHeader header;
		header.components = components_for_format(mFormat);
		header.type = array_type_for_type(mType);
		header.dimcount = dimcount_for_target(mTarget);
		header.dim[0] = mWidth;
		header.dim[1] = mHeight;
		header.dim[2] = mDepth;
		
		// alignment
		allo_array_setstride(&header, 1);
		if(! allo_array_equal_headers(
				&(mArray.header),
				&(header)
			)
		) {
			allo_array_adapt(&mArray, &header);
			memset(mArray.data.ptr, '\0', allo_array_size(&mArray));
		}
		else if(clear_data) {
			memset(mArray.data.ptr, '\0', allo_array_size(&mArray));
		}
		
		if(do_bind) {
			mBackend->textureSubmit(this);
		}
	}
	
	if(do_bind) {
		unbind(unit);
	}
}
	
void Texture::bind(int unit) {
	if(mRebuild) {
		destroy();
	}

	bool did_create = false;
	if(!created()) {
		create();
		mRebuild = false;
		
		if(mMode == SURFACE && mSurface && !mSurface->creating()) {
			mSurface->destroy();
			mSurface->create();
			if(!mSurface->creating()) {
				clear(unit);
			}
		}
		
		did_create = true;
		mUpdate = true;
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

void Texture::setArrayFormat(const AlloArrayHeader &header) {
	mFormat = format_for_array_components(header.components);
	mType = type_for_array_type(header.type);

	mWidth = header.dim[0];
	mHeight = header.dim[1];
	mDepth = header.dim[2];
	
	mTarget = target_for_array_dimcount(header.dimcount);
	
	// allocate array data space
	mArray.format(header);
}

void Texture::fromArray(const al::Array *array) {
	int sz = allo_array_size(array);
	if(sz <= 0) {
		return;
	}

	if(! 
		allo_array_equal_headers(
			&(mArray.header), //&(mData->array.header), 
			&(array->header)
		) || 
		mMode != DATA)
	{
		mMode = DATA;
		setArrayFormat(array->header);
		mRebuild = true;
	}
	
	memcpy(mArray.data.ptr, array->data.ptr, sz);
	//memcpy(mData->array.data.ptr, array->data.ptr, allo_array_size(array));
	mUpdate = true;
	
	
//	if(! mArray.hasFormat(array->header) || mMode != DATA) {
//		mMode = DATA;
//		setArrayFormat(array->header);
//		mArray.format(*array);
//		memcpy(mArray.data.ptr, array->data.ptr, mArray.size());
//		mRebuild = true;
//	}
//	
//	mUpdate = true;
}

void Texture::toArray() {
	mBackend->textureToArray(this);
}

void Texture::allocate(AlloArrayHeader &header) {
	if(! 
		allo_array_equal_headers(
			//&(mData->array.header), 
			&(mArray.header), 
			&(header)
		) || 
		mMode != DATA)
	{
		mMode = DATA;
		setArrayFormat(header);
		//memset(mData->array.data.ptr, '\0', allo_array_size(&(mData->array)));
		memset(mArray.data.ptr, '\0', allo_array_size(&mArray));
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
	return mArray.data.ptr;
}

int Texture::getRowStride() {
	return mArray.header.stride[1];
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

Texture::Format Texture::singleChannel() {
	return mSingleChannel;
}

void Texture::singleChannel(Format v) {
	if(v == ALPHA || v == LUMINANCE) {
		mSingleChannel = v;
	}
	else {
		mSingleChannel = ALPHA;
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

void Texture::borderColor(const Color& c) {
	mBorderColor = c;
}

void Texture::onCreate() {
	mBackend->textureCreate(this);
}

void Texture::onDestroy() {
	mBackend->textureDestroy(this);
}

Texture::Format Texture::format_for_array_components(int components) {
	switch(components) {
		case 1:	return mSingleChannel;
		case 2:	return LUMALPHA;
		case 3:	return RGB;
		case 4:	
		default:
			return RGBA;
	}
}

int Texture::components_for_format(Format format) {
	switch(format) {
		case ALPHA: return 1;
		case LUMINANCE: return 1;
		case LUMALPHA: return 2;
		case RGB: return 3;
		case RGBA:
		default:
			return 4;
	}
}

Texture::Type Texture::type_for_array_type(AlloTy type) {
	switch(type) {
		case AlloUInt8Ty:		return UCHAR;
		case AlloSInt32Ty:		return INT;
		case AlloUInt32Ty:		return UINT;
		case AlloFloat32Ty:		return FLOAT32;
		default:
			return UCHAR;
	}
}

AlloTy Texture::array_type_for_type(Type type) {
	switch(type) {
		case UCHAR:		return AlloUInt8Ty;
		case INT:		return AlloSInt32Ty;
		case UINT:		return AlloUInt32Ty;
		case FLOAT32:	return AlloFloat32Ty;
		default:
			return AlloUInt8Ty;
	}
}

Texture::Target Texture::target_for_array_dimcount(int dimcount) {
	switch(dimcount) {
		case 1:	return TEXTURE_1D;
		case 3:	return TEXTURE_3D;
		
		case 2:
		default:
			return mRect ? TEXTURE_RECT : TEXTURE_2D;
	}
}

int Texture::dimcount_for_target(Target target) {
	switch(target) {
		case TEXTURE_1D:	return 1;
		case TEXTURE_3D:	return 3;
		case TEXTURE_2D:
		case TEXTURE_RECT:
		default:
			return 2;
	}
}

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
