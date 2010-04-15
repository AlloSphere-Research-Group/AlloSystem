#if defined(__IPHONE_2_0)
	#define AL_GRAPHICS_USE_OPENGLES1
	
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>
#endif

#include "protocol/al_Graphics.hpp"

namespace al {

#ifdef AL_GRAPHICS_USE_OPENGLES1

static void gl_begin(Graphics * g, int mode) { 
	g->mMode = mode; 
}
static void gl_end(Graphics * g) { 
	//  TODO: drawarrays
	
	glVertexPointer(3, GL_FLOAT, 0, g->mVertexBuffer.data());
    glEnableClientState(GL_VERTEX_ARRAY);
    glColorPointer(4, GL_FLOAT, 0, g->mColorBuffer.data());
    glEnableClientState(GL_COLOR_ARRAY);
	
	glDrawArrays(g->mMode, 0, g->mVertexBuffer.size());
	
	printf("drawn\n");
}
static void gl_vertex3d(Graphics * g, double x, double y, double z) { 
	g->mVertexBuffer.append(Vec3f(x, y, z)); 
}
static void gl_color4d(Graphics * G, double r, double g, double b, double a) { 
	G->mColorBuffer.append(Vec4f(r, g, b, a)); 
}

bool setBackendOpenGLES1(Graphics * g) {
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex3d = gl_vertex3d;
	g->s_color4d = gl_color4d;
	
	#define SET_GL_ENUM(x) Graphics::x = GL_##x
	SET_GL_ENUM(POINTS);
	SET_GL_ENUM(LINES);
	SET_GL_ENUM(LINE_LOOP);
	SET_GL_ENUM(LINE_STRIP);
	SET_GL_ENUM(TRIANGLES);
	SET_GL_ENUM(TRIANGLE_STRIP);
	SET_GL_ENUM(TRIANGLE_FAN);
	
	// init defaults:
	g->mMode = GL_POINTS;
	printf("set GraphicsBackend::OpenGLES1");
	g->mBackend = GraphicsBackend::OpenGLES1;
	return true;
}

#else

bool setBackendOpenGLES1(Graphics * g) {
	return false;
}

#endif

} // al::

