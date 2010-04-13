#include "protocol/al_Graphics.hpp"

#if defined(GL_VERSION_ES_CM_1_0) || defined(GL_VERSION_ES_CL_1_0) || defined(GL_VERSION_ES_CM_1_1) || defined(GL_VERSION_ES_CL_1_1)
	#define AL_GRAPHICS_DETECTED_BACKEND_OPENGLES1
#endif 
#ifdef GL_ES_VERSION_2_0
	#define AL_GRAPHICS_DETECTED_BACKEND_OPENGLES2
#endif
#if defined (__APPLE__) || defined (OSX)	
	#define AL_GRAPHICS_DETECTED_BACKEND_OPENGL
#endif 
#if defined(__linux__)		
	#define AL_GRAPHICS_DETECTED_BACKEND_OPENGL
#endif 
#if defined(WIN32)	
	#define AL_GRAPHICS_DETECTED_BACKEND_OPENGL
#endif

#if defined(AL_GRAPHICS_DETECTED_BACKEND_OPENGLES2)
	#define AL_GRAPHICS_DEFAULT_BACKEND		(GraphicsBackend::OpenGLES2)
#elif defined(AL_GRAPHICS_DETECTED_BACKEND_OPENGLES1)
	#define AL_GRAPHICS_DEFAULT_BACKEND		(GraphicsBackend::OpenGLES1)
#elif defined(AL_GRAPHICS_DETECTED_BACKEND_OPENGL)
	#define AL_GRAPHICS_DEFAULT_BACKEND		(GraphicsBackend::OpenGL)
#endif

namespace al {

static void gl_begin(Graphics * g, int mode) {}
static void gl_end(Graphics * g) {}
static void gl_vertex3d(Graphics * g, double x, double y, double z) {}
static void gl_color3d(Graphics * g, double x, double y, double z) {}



bool setBackendNone(Graphics * g) {
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex3d = gl_vertex3d;
	g->s_color3d = gl_color3d;
	
	
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
