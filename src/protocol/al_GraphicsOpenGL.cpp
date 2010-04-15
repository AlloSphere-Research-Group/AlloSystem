
#include "protocol/al_Graphics.hpp"

// OpenGL platform-dependent includes
#if defined (__IPHONE_2_0)

#elif defined (__APPLE__) || defined (OSX)
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
static void gl_color4d(Graphics * g, double x, double y, double z, double a) { glColor4d(x, y, z, a); }

bool setBackendOpenGL(Graphics * g) {
	
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex3d = gl_vertex3d;
	g->s_color4d = gl_color4d;

	#define SET_GL_ENUM(x) g->x = GL_##x
	SET_GL_ENUM(POINTS);
	SET_GL_ENUM(LINES);
	SET_GL_ENUM(LINE_LOOP);
	SET_GL_ENUM(LINE_STRIP);
	SET_GL_ENUM(TRIANGLES);
	SET_GL_ENUM(TRIANGLE_STRIP);
	SET_GL_ENUM(TRIANGLE_FAN);
	SET_GL_ENUM(QUADS);
	SET_GL_ENUM(QUAD_STRIP);
	SET_GL_ENUM(POLYGON);
	
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
