
#include "protocol/al_Graphics.hpp"
#include "graphics/al_Config.h"

namespace al {
namespace gfx{

#ifdef AL_GRAPHICS_USE_OPENGLES2

enum{
	ATTRIB_VERTEX,
	ATTRIB_COLOR,
	ATTRIB_NORMAL,
	ATTRIB_TEXCOORD
};

// TODO: Common to OpenGL, OpenGL ES1, and OpenGL ES2
static void gl_clear(int attribMask){ glClear(attribMask); }
static void gl_clearColor(float r, float g, float b, float a){ glClearColor(r,g,b,a); }
static void gl_loadIdentity(){ glLoadIdentity(); }
static void gl_viewport(int x, int y, int w, int h){ glViewport(x,y,w,h); }
static void gl_begin(Graphics * g, int mode) {
	g->data().resetBuffers();
	g->data().primitive(mode);
}
static void gl_end(Graphics * g) {
	g->draw();
	g->data().resetBuffers();
}

static void gl_color(Graphics * G, double r, double g, double b, double a) {
	G->data().addColor(r,g,b,a);
}
static void gl_normal(Graphics * g, double x, double y, double z) {
	g->data().addNormal(x,y,z);
}
static void gl_vertex(Graphics * g, double x, double y, double z) {
	g->data().addVertex(x,y,z);
}


static void gl_draw(){
//	// Something like this will be used here...
//
//	// Enable arrays and set pointers...
//	glEnableVertexAttribArray(ATTRIB_VERTEX);
//	glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, &v.vertices()[0]);
//
//	if(Nn){
//		glEnableVertexAttribArray(ATTRIB_NORMAL);
//		glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, 0, 0, &v.normals()[0]);
//	}
//	
//	if(Nc){
//		glEnableVertexAttribArray(ATTRIB_COLOR);
//		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 0, 0, &v.colors()[0]);
//	}
//	
//	if(Nt2 || Nt3){
//		glEnableVertexAttribArray(ATTRIB_TEXCOORD);
//		if(Nt2)	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, 0, 0, &v.texCoord2s()[0]);
//		else	glVertexAttribPointer(ATTRIB_TEXCOORD, 3, GL_FLOAT, 0, 0, &v.texCoord3s()[0]);
//	}
//	
//	// Send the package over...
//	if(Ni){
//		glDrawElements(v.primitive(), ie-is, GL_UNSIGNED_INT, &v.indices()[is]);
//	}
//	else{
//		glDrawArrays(v.primitive(), 0, v.vertices().size());
//	}

}


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

	g->s_clear = gl_clear;
	g->s_clearColor = gl_clearColor;
	g->s_loadIdentity = gl_loadIdentity;
	g->s_viewport = gl_viewport;

	g->s_begin = gl_begin;
	g->s_end = gl_end;
	g->s_vertex = gl_vertex;
	g->s_normal = gl_normal;
	g->s_color = gl_color;
	
	g->s_draw = gl_draw;
	
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

	SET_GL_ENUM(COLOR_BUFFER_BIT);
	SET_GL_ENUM(DEPTH_BUFFER_BIT);
	
//	g->mMode = GL_POINTS;
	
	printf("using GraphicsBackend::OpenGLES2\n");
	g->mBackend = GraphicsBackend::OpenGLES2;
	return true;
}

#else

bool setBackendOpenGLES2(Graphics * g) {
	return false;
}

#endif

} // ::al::gfx
} // ::al

