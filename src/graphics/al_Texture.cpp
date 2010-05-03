#include <stdlib.h>
#include "graphics/al_Config.h"
#include "graphics/al_Texture.hpp"

namespace al {
namespace gfx{


TextureBase::TextureBase(Format::t format, DataType::t dataType, WrapMode::t wm)
:	mPixels(0), mBuffer(0),
	mFormat(format), mIpol(IpolMode::Linear), mType(dataType), mWrap(wm)
{}

TextureBase::~TextureBase(){
	destroy();
	freeMem();
}

// This creates an internal buffer and points mPixels to it
void TextureBase::allocMem(){
	freeMem();
	mBuffer = malloc(numComponents(format()) * numBytes(dataType()) * size());
	mPixels = mBuffer;
}

const TextureBase& TextureBase::begin() const { glBindTexture(target(), id()); return *this; }
const TextureBase& TextureBase::bind() const { return begin(); }
void TextureBase::end() const { glBindTexture(target(), 0); }

void TextureBase::freeMem(){
	if(mBuffer) free(mBuffer); mBuffer=0;
}

const TextureBase& TextureBase::send() const{
	begin();
	texSubImage();
	end();
	return *this;
}

TextureBase& TextureBase::dataType(DataType::t v){ mType=v; return *this; }
TextureBase& TextureBase::format(Format::t v){ mFormat=v; return *this; }
TextureBase& TextureBase::ipolMode(IpolMode::t v){ mIpol=v; return *this; }
TextureBase& TextureBase::wrapMode(WrapMode::t v){ mWrap=v; return *this; }

void TextureBase::onCreate(){
	GLuint glID = (GLuint)mID;
	glGenTextures(1, &glID); //printf("%i\n", mID);
	bind();
	texImage();
	glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, ipolMode());
	glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, ipolMode());
	texWrap();
	send();
}

void TextureBase::onDestroy(){
	GLuint glID = (GLuint)mID;
	glDeleteTextures(1, &glID);
}


Texture2::Texture2(int w, int h, Format::t format, DataType::t type, WrapMode::t wrap)
:	TextureBase(format, type, wrap), w(w), h(h)
{	allocMem(); }

Texture2& Texture2::draw(
	float ql, float qt, float qr, float qb,
	float tl, float tt, float tr, float tb)
{	
	glBegin(GL_QUADS);       
		glTexCoord2f(tl,tt); glVertex2f(ql,qt);
		glTexCoord2f(tl,tb); glVertex2f(ql,qb);
		glTexCoord2f(tr,tb); glVertex2f(qr,qb);
		glTexCoord2f(tr,tt); glVertex2f(qr,qt);
	glEnd();
	return *this;
}

Texture2& Texture2::load(int width, int height, void * pixs){
	w = width;
	h = height;
	mPixels = pixs;
	create();
	return *this;
}

int Texture2::target() const { return GL_TEXTURE_2D; }
void Texture2::texImage() const {
	glTexImage2D(GL_TEXTURE_2D, 0, numComponents(format()), w, h, 0, format(), dataType(), mPixels);
}
void Texture2::texSubImage() const {
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format(), dataType(), mPixels);
}
void Texture2::texWrap() const {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode());
}



Texture3::Texture3(int _w, int _h, int _d, Format::t format, DataType::t type, WrapMode::t wrap)
:	TextureBase(format, type, wrap), w(_w), h(_h), d(_d)
{	allocMem(); }

Texture3& Texture3::load(int width, int height, int depth, void * pixs){
	w = width;
	h = height;
	d = depth;
	mPixels = pixs;
	create();
	return *this;
}

int Texture3::target() const { return GL_TEXTURE_3D; }
void Texture3::texImage() const {
	glTexImage3D(GL_TEXTURE_3D, 0, numComponents(format()), w, h, d, 0, format(), dataType(), mPixels);
}
void Texture3::texSubImage() const {
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, d, format(), dataType(), mPixels);
}
void Texture3::texWrap() const {
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapMode());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapMode());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapMode());	
}

} // ::al::gfx
} // ::al
