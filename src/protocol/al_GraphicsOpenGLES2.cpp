
#include "protocol/al_Graphics.hpp"

#if defined(__IPHONE_3_0)
	#define AL_GRAPHICS_USE_OPENGLES2
	
	#import <OpenGLES/ES2/gl.h>
	#import <OpenGLES/ES2/glext.h>
#endif

namespace al {

#ifdef AL_GRAPHICS_USE_OPENGLES2

bool setBackendOpenGLES2(Graphics * g) {
	
	// check hardware support:
	// e.g. version OpenGL ES-CM 1.1
	char name[20];
	int major, minor;
	const char * version = (char *)glGetString(GL_VERSION);
	sscanf (version,"OpenGL %s %d.%d",name,&major,&minor);
	if (major < 2) {
		return false;
	} 
	
	#define SET_GL_ENUM(x) g->x = GL_##x
	SET_GL_ENUM(POINTS);
	SET_GL_ENUM(LINES);
	SET_GL_ENUM(LINE_LOOP);
	SET_GL_ENUM(LINE_STRIP);
	SET_GL_ENUM(TRIANGLES);
	SET_GL_ENUM(TRIANGLE_STRIP);
	SET_GL_ENUM(TRIANGLE_FAN);
	g->QUADS = 0;
	g->QUAD_STRIP = 0;
	g->POLYGON = 0;
	
	printf("using GraphicsBackend::OpenGLES2\n");
	g->mBackend = GraphicsBackend::OpenGLES2;
	return true;
}

#else

bool setBackendOpenGLES2(Graphics * g) {
	return false;
}

#endif

} // al::

