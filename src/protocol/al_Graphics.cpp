#include "protocol/al_Graphics.hpp"

namespace al {

static void gl_begin(Graphics * g, int mode) {}
static void gl_end(Graphics * g) {}
static void gl_vertex(Graphics * g, double x, double y, double z) {}
static void gl_normal(Graphics * g, double x, double y, double z) {}
static void gl_color(Graphics * g, double x, double y, double z, double a) {}



bool setBackendNone(Graphics * g) {
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex = gl_vertex;
	g->s_normal = gl_normal;
	g->s_color = gl_color;
	
	
	g->mBackend = GraphicsBackend::None;
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

Graphics :: Graphics(GraphicsBackend::type backend) {
	setBackend(backend);	
}

Graphics :: ~Graphics() {
	
}

bool Graphics :: setBackend(GraphicsBackend::type backend) {
	if (backend == GraphicsBackend::AutoDetect) {
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
		case GraphicsBackend::OpenGLES2:
			setBackendOpenGLES2(this);
			break;
		case GraphicsBackend::OpenGLES1:
			setBackendOpenGLES1(this);
			break;
		case GraphicsBackend::OpenGL:
			setBackendOpenGL(this);
			break;
		default:
			setBackendNone(this);
			return false;
	}
	
	return true;
}

} // al::
