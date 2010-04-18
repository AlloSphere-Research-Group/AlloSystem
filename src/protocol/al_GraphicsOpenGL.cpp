
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

#if 0
static void gl_begin(Graphics * g, int mode) { glBegin(mode); }
static void gl_end(Graphics * g) { glEnd(); }
static void gl_vertex(Graphics * g, double x, double y, double z) { glVertex3d(x, y, z); }
static void gl_normal(Graphics * g, double x, double y, double z) { glNormal3d(x, y, z); }
static void gl_color(Graphics * g, double x, double y, double z, double a) { glColor4d(x, y, z, a); }
#else
static void gl_begin(Graphics * g, int mode) { 
	g->mMode = mode; 
}
static void gl_end(Graphics * g) { 
	//  TODO: drawarrays
	int nvertices = g->mVertexBuffer.size() - 1; // always have 1 extra	
	Graphics::VertexData * data = g->mVertexBuffer.data();
	
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Graphics::VertexData), &data[0].position);
	glNormalPointer(3, sizeof(Graphics::VertexData), &data[0].normal);
	glColorPointer(4, GL_FLOAT, sizeof(Graphics::VertexData), &data[0].color);
	//glTexturePointer(2, GL_FLOAT, sizeof(Graphics::Vertex), &data[0].texcoord);
	
	glDrawArrays(g->mMode, 0, nvertices);
	
	g->mVertexBuffer.clear();
	g->mVertexBuffer.extend(); // always have 1
}

static void gl_color(Graphics * G, double r, double g, double b, double a) { 
	G->mVertexBuffer.top().color.set(r, g, b, a);
}
static void gl_normal(Graphics * g, double x, double y, double z) { 
	g->mVertexBuffer.top().normal.set(x, y, z);
}
static void gl_vertex(Graphics * g, double x, double y, double z) { 
	g->mVertexBuffer.top().position.set(x, y, z);
	// done with top element; create a new top:
	g->mVertexBuffer.extend();
}
#endif


bool setBackendOpenGL(Graphics * g) {
	
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex = gl_vertex;
	g->s_normal = gl_normal;
	g->s_color = gl_color;

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
	
	g->mMode = GL_POINTS;
	g->mVertexBuffer.extend(); // always have 1
	
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
