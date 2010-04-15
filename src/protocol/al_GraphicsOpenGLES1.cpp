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

static void gl_color4d(Graphics * G, double r, double g, double b, double a) { 
	G->mVertexBuffer.top().color.set(r, g, b, a);
}
static void gl_normal3d(Graphics * g, double x, double y, double z) { 
	g->mVertexBuffer.top().normal.set(x, y, z);
}
static void gl_vertex3d(Graphics * g, double x, double y, double z) { 
	g->mVertexBuffer.top().position.set(x, y, z);
	// done with top element; create a new top:
	g->mVertexBuffer.extend();
}

bool setBackendOpenGLES1(Graphics * g) {
	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex3d = gl_vertex3d;
	g->s_normal3d = gl_normal3d;
	g->s_color4d = gl_color4d;
	
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
	
	g->mMode = GL_POINTS;
	
	printf("using GraphicsBackend::OpenGLES1\n");
	g->mBackend = GraphicsBackend::OpenGLES1;
	return true;
}

#else

bool setBackendOpenGLES1(Graphics * g) {
	return false;
}

#endif

} // al::

