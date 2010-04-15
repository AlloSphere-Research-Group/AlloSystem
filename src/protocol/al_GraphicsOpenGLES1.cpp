#include "protocol/al_Graphics.hpp"

#if defined(__IPHONE_3_0)
	#define AL_GRAPHICS_USE_OPENGLES1
	
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>
#endif


#ifdef AL_GRAPHICS_USE_OPENGLES1

static void gl_begin(Graphics * g, int mode) { mMode = mode; }
static void gl_end(Graphics * g) { 
	//  TODO: drawarrays
}
static void gl_vertex3d(Graphics * g, double x, double y, double z) { g->mVertexBuffer.append(Vec3d(x, y, z)); }
static void gl_color3d(Graphics * g, double r, double g, double b) { g->mColorBuffer.append(Vec3d(r, g, b, 1)); }

namespace al {

bool setBackendOpenGLES1(Graphics * g) {
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex3d = gl_vertex3d;
	g->s_color3d = gl_color3d;

	g->mBackend = GraphicsBackend::OpenGLES1;
	return true;
}

} // al::

#else

namespace al {

bool setBackendOpenGLES1(Graphics * g) {
	return false;
}

} // al::

#endif

