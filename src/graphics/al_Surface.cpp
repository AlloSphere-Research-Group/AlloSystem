#include <stdlib.h>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Surface.hpp"

namespace al{

Surface::Surface(Graphics *backend, void *surface_data)
:	GPUObject(),
	mBackend(backend),
	mSurfaceData(surface_data),
	mTexture(0),
	mClearColor(0., 0., 0., 1.),
	mClearDepth(1.),
	mAutoclear(true)
{}

Surface::~Surface() {
}

void * Surface::surfaceData() {
	return mSurfaceData;
}

void Surface::attach(Texture *tex) {
	mTexture = tex;
	mTexture->mode(Texture::SURFACE);
}

Texture * Surface::texture() {
	return mTexture;
}

void Surface::enter() {
	if(mTexture) {
		if(! created()) {
			create();
		}
	
		mBackend->surfaceBind(this);
		mBackend->surfaceEnter(this);
	}
}

void Surface::leave() {
	mBackend->surfaceLeave(this);
	mBackend->surfaceUnbind(this);
}

Color& Surface::clearColor() {
	return mClearColor;
}

float Surface::clearDepth() {
	return mClearDepth;
}

void Surface::clearDepth(float v) {
	mClearDepth = v;
}

bool Surface::autoClear() {
	return mAutoclear;
}

void Surface::autoClear(bool v) {
	mAutoclear = v;
}

int Surface::width() {
	return mTexture ? mTexture->width() : 0;
}

int Surface::height() {
	return mTexture ? mTexture->height() : 0;
}


void Surface::onCreate() {
	mBackend->surfaceCreate(this);
}

void Surface::onDestroy() {
	mBackend->surfaceDestroy(this);
}

} // ::al
