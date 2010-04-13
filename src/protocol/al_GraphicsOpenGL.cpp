
#include "al_Graphics.hpp"

// OpenGL platform-dependent includes
#if defined (__APPLE__) || defined (OSX)
	#define AL_GRAPHICS_USE_OPENGL

	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
	
#elif defined(__linux__)
	#define AL_GRAPHICS_USE_OPENGL

	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <time.h>
	
#elif defined(WIN32)
	#define AL_GRAPHICS_USE_OPENGL

	#include <windows.h>
	#include <gl/gl.h>
	#include <gl/glu.h>
	#pragma comment( lib, "winmm.lib")
	#pragma comment( lib, "opengl32.lib" )
	#pragma comment( lib, "glu32.lib" )
	
#endif

#ifdef AL_GRAPHICS_USE_OPENGL

namespace al {

static void gl_begin(Graphics * g, int mode) { glBegin(mode); }
static void gl_end(Graphics * g) { glEnd(); }
static void gl_vertex3d(Graphics * g, double x, double y, double z) { glVertex3d(x, y, z); }
static void gl_color3d(Graphics * g, double x, double y, double z) { glColor3d(x, y, z); }

bool setBackendOpenGL(Graphics * g) {
	
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex3d = gl_vertex3d;
	g->s_color3d = gl_color3d;
	
	g->mBackend = GraphicsBackend::OpenGL;
	return true;
}

} // al::

#else

namespace al {

bool setBackendOpenGL(Graphics * g) {
	return false;
}

} // al::

#endif
