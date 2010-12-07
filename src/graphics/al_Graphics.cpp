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
	mMesh.resetBuffers();

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
		mMesh.addVertex(x, y, z);
		mMesh.equalizeBuffers();
	} else {
		raw_vertex(x, y, z);
	}	
}

void Graphics::texcoord(double u, double v) {
	if(mInImmediateMode) {
		mMesh.addTexCoord(u, v);
	} else {
		raw_texcoord(u, v);
	}
}

void Graphics::normal(double x, double y, double z) {
	if(mInImmediateMode) {
		mMesh.addNormal(x, y, z);
	} else {
		raw_normal(x, y, z);
	}
}

void Graphics::color(double r, double g, double b, double a) {
	if(mInImmediateMode) {
		mMesh.addColor(r, g, b, a);
	} else {
		raw_color(r, g, b, a);
	}
}


} // ::al
