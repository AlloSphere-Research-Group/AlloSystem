
#include "protocol/al_Graphics.hpp"

#if defined(__IPHONE_3_0)
	#define AL_GRAPHICS_USE_OPENGLES2
	
	#import <OpenGLES/ES2/gl.h>
	#import <OpenGLES/ES2/glext.h>
#endif


#ifdef AL_GRAPHICS_USE_OPENGLES2

namespace al {

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
	
	g->mBackend = GraphicsBackend::OpenGLES2;
	return true;
}

} // al::

#else

namespace al {

bool setBackendOpenGLES2(Graphics * g) {
	return false;
}

} // al::

#endif

