#if defined(__IPHONE_2_0)
	#define AL_GRAPHICS_USE_OPENGLES1
	
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>
#endif

#include "protocol/al_Graphics.hpp"


namespace al {
namespace gfx{

#ifdef AL_GRAPHICS_USE_OPENGLES1

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

// TODO: Common to OpenGL and OpenGL ES1
static void gl_draw(const GraphicsData& v){
	int Nv = v.vertices().size();
	if(0 == Nv) return;
	
	int Nc = v.colors().size();
	int Nn = v.normals().size();
	int Nt2= v.texCoord2s().size();
	int Nt3= v.texCoord3s().size();
	int Ni = v.indices().size();

	// Enable arrays and set pointers...
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &v.vertices()[0]);

	if(Nn){
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &v.normals()[0]);
	}
	
	if(Nc){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &v.colors()[0]);			
	}
	
	if(Nt2 || Nt3){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(Nt2) glTexCoordPointer(2, GL_FLOAT, 0, &v.texCoord2s()[0]);
		if(Nt3) glTexCoordPointer(3, GL_FLOAT, 0, &v.texCoord3s()[0]);
	}
	
	// Send the package over...
	if(Ni){
		//unsigned vs=0, ve=Nv;	// range of vertex indices to prefetch
								// NOTE:	if this range exceeds the number of vertices,
								//			expect a segmentation fault...
		unsigned is=0, ie=Ni;	// range of indices to draw

//		glDrawRangeElements(v.primitive(), vs, ve, ie-is, GL_UNSIGNED_INT, &v.indices()[is]);
		glDrawElements(v.primitive(), ie-is, GL_UNSIGNED_INT, &v.indices()[is]);
	}
	else{
		glDrawArrays(v.primitive(), 0, v.vertices().size());
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	if(Nn) glDisableClientState(GL_NORMAL_ARRAY);
	if(Nc) glDisableClientState(GL_COLOR_ARRAY);
	if(Nt2 || Nt3) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


bool setBackendOpenGLES1(Graphics * g) {

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
	
	printf("using GraphicsBackend::OpenGLES1\n");
	g->mBackend = GraphicsBackend::OpenGLES1;
	return true;
}

#else

bool setBackendOpenGLES1(Graphics * g) {
	return false;
}

#endif

} // ::al::gfx
} // ::al
