#include "protocol/al_Graphics.hpp"

namespace al {
namespace gfx{



void GraphicsData::resetBuffers(){
	vertices().clear();
	normals().clear();
	colors().clear();
	texCoord2s().clear();
	texCoord3s().clear();
	indices().clear();	
}


static void gl_draw(const GraphicsData& v){}

static void gl_clear(int attribMask){}
static void gl_clearColor(float r, float g, float b, float a){}
static void gl_loadIdentity(){}
static void gl_viewport(int x, int y, int w, int h){}

static void gl_begin(Graphics * g, int mode) {}
static void gl_end(Graphics * g) {}
static void gl_vertex(Graphics * g, double x, double y, double z) {}
static void gl_normal(Graphics * g, double x, double y, double z) {}
static void gl_color(Graphics * g, double x, double y, double z, double a) {}



bool setBackendNone(Graphics * g) {
	g->s_draw = gl_draw;
	g->s_clear = gl_clear;
	g->s_clearColor = gl_clearColor;
	g->s_loadIdentity = gl_loadIdentity;
	g->s_viewport = gl_viewport;

	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex = gl_vertex;
	g->s_normal = gl_normal;
	g->s_color = gl_color;

	g->mBackend = Backend::None;
	
	return true;
}


bool setBackendOpenGLES(Graphics * g) {
	if (setBackendOpenGLES2(g)) {
		return true;
	} else if (setBackendOpenGLES1(g)) {
		return true;
	} else {
		return false;
	}
}

Graphics :: Graphics(Backend::type backend) {
	setBackend(backend);	
}

Graphics :: ~Graphics() {
	
}

bool Graphics :: setBackend(Backend::type backend) {
	if (backend == Backend::AutoDetect) {
		if (setBackendOpenGLES(this)) {
			return true;
		} else if (setBackendOpenGL(this)) {
			return true;
		} else {
			setBackendNone(this);
			return false;
		}
	}
		
	// set function pointers according to backend
	switch (backend) {
		case Backend::OpenGLES2:
			setBackendOpenGLES2(this);
			break;
		case Backend::OpenGLES1:
			setBackendOpenGLES1(this);
			break;
		case Backend::OpenGL:
			setBackendOpenGL(this);
			break;
		default:
			setBackendNone(this);
			return false;
	}
	
	return true;
}

} // ::al::gfx
} // ::al
