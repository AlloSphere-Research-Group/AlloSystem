#include "al_Graphics.hpp"

#if defined(__IPHONE_3_0)
	#define AL_GRAPHICS_USE_OPENGLES1
	
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>
#endif


#ifdef AL_GRAPHICS_USE_OPENGLES1

namespace al {

bool setBackendOpenGLES1(Graphics * g) {
	
		
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

