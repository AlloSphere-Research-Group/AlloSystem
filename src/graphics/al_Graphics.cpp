#include <vector>
#include <map>
#include <string>

#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Graphics::Graphics() : mInImmediateMode(false) {}
Graphics::~Graphics() {}


// Immediate Mode
void Graphics::begin(Primitive mode) {
	// clear buffers
	mMesh.reset();

	// start primitive drawing
	mMesh.primitive(mode);
	mInImmediateMode = true;
}

void Graphics::end() {
	draw(mMesh);
	mInImmediateMode = false;
}

void Graphics::vertex(double x, double y, double z) {
	if(mInImmediateMode) {
		// make sure all buffers are the same size if > 0
		mMesh.vertex(x, y, z);
		mMesh.equalizeBuffers();
	}	
}

void Graphics::texcoord(double u, double v) {
	if(mInImmediateMode) {
		mMesh.texCoord(u, v);
	}
}

void Graphics::normal(double x, double y, double z) {
	if(mInImmediateMode) {
		mMesh.normal(x, y, z);
	}
}

void Graphics::color(double r, double g, double b, double a) {
	if(mInImmediateMode) {
		mMesh.color(r, g, b, a);
	} else {
		p_currentColor(r, g, b, a);
	}
}


} // ::al
