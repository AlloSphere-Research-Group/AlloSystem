
#include <stdio.h>
#include <stdlib.h>

#include "types/al_MsgQueue.hpp"
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "math/al_Random.hpp"
#include "system/al_Time.hpp"
#include "graphics/al_Context.hpp"
#include "spatial/al_Camera.hpp"
#include "protocol/al_GraphicsBackendOpenGL.hpp"

using namespace al;

/*
gfx::GraphicsBackendOpenGL backend;
gfx::Graphics gl(&backend);

Camera camera;
gfx::Texture *texture = backend.textureNew();
gfx::Texture *surface_tex = backend.textureNew();
gfx::Surface *surface = backend.surfaceNew();
double eyesep = 1;


// invent some kind of scene:
struct vertex {
	float x, y, z;
	float r, g, b;
};
#define NUM_VERTICES (256)
static vertex vertices[NUM_VERTICES];

void mprint(const Matrix4d &m) {
	printf("%.5f %.5f %.5f %.5f\n", m[0], m[4], m[8], m[12]);
	printf("%.5f %.5f %.5f %.5f\n", m[1], m[5], m[9], m[13]);
	printf("%.5f %.5f %.5f %.5f\n", m[2], m[6], m[10], m[14]);
	printf("%.5f %.5f %.5f %.5f\n", m[3], m[7], m[11], m[15]);
}
*/
static void drawScene(void * self) {

	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(1, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

/*
	gl.clearColor(1, 0, 0, 0);
    gl.clear(gfx::AttributeBit::ColorBuffer | gfx::AttributeBit::DepthBuffer);

	gl.matrixMode(gfx::PROJECTION);
	gl.pushMatrix();
	//	gl.loadMatrix(camera.projectionMatrix());

	gl.matrixMode(gfx::MODELVIEW);
	gl.pushMatrix();
	//	gl.loadMatrix(camera.modelViewMatrix());


	//surface->enter();
	gl.begin(gfx::LINES);
		for (int i=0; i< NUM_VERTICES; i++) {
			vertex& v = vertices[i];
			gl.color(v.r, v.g, v.b, 0.5);
			gl.vertex(v.x, v.y, v.z);
		}

		gl.color(1, 0, 0.5, 1);
		gl.vertex(-1, 0, 0);

		gl.color(1, 0, 0.5, 1);
		gl.vertex(1, 0, 0);
	gl.end();
	//surface->leave();
*/
/*
	gl.pushMatrix();
	gl.scale(2, 2, 2);
		surface_tex->bind();
		gl.begin(gfx::QUADS);
			gl.color(1, 1, 1, 1);
			gl.texcoord(0, 0);
			gl.vertex(-1, -1);

			gl.texcoord(1, 0);
			gl.vertex(1, -1);

			gl.texcoord(1, 1);
			gl.vertex(1, 1);

			gl.texcoord(0, 1);
			gl.vertex(-1, 1);
		gl.end();
		surface_tex->unbind();
	gl.popMatrix();


	gl.pushMatrix();
	gl.translate(1, 1, 0);
		texture->bind();
		gl.draw();

		gl.begin(gfx::QUADS);
			gl.color(1, 1, 1, 1);
			gl.texcoord(0, 0);
			gl.vertex(-1, -1);

			gl.texcoord(1, 0);
			gl.vertex(1, -1);

			gl.texcoord(1, 1);
			gl.vertex(1, 1);

			gl.texcoord(0, 1);
			gl.vertex(-1, 1);
		gl.end();

		texture->unbind();
	gl.popMatrix();

	gl.matrixMode(gfx::PROJECTION);
	gl.popMatrix();

	gl.matrixMode(gfx::MODELVIEW);
	gl.popMatrix();*/
}



struct MyWindow : WindowGL{
/*
	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }

	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
*/

	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey();
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl() || k.alt()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}

	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void onFrame(){
		drawScene(this);
	}
};



double fps =100;

int main (int argc, char * argv[]) {
        printf("main\n");
        printf("%f\n", MainLoop::get().T0());

MyWindow win1;

/*
	al::Lattice tex_data;
	tex_data.create2d(3, AlloUInt8Ty, 256, 256);

	for(int j=0; j < 256; j++) {
		unsigned char *data = (unsigned char *)(tex_data.data.ptr + j*tex_data.header.stride[1]);

		for(int i=0; i < 256; i++) {
			*data++ = i;
			*data++ = j;
			*data++ = 255-j;
		}
	}

	texture->fromLattice(&tex_data);

	surface_tex->dimensions(512, 512);
	surface->attach(surface_tex);


	rnd::Random<> rng;

	/// define the scene - a semi random walk
	for(int i=0; i<NUM_VERTICES; i++) {

		double r, g, b, y, z;

		double p = i/(double)NUM_VERTICES;
		r = p;
		g = 1-p;
		b = sin(p * 6 * M_PI);

		y = b;
		z = 5 + 2*sin(p * 2 * M_PI);

		vertices[i].x = -1.5;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;

		i++;

		vertices[i].x = 1.5;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;

	}
	*/

	win1.create(WindowGL::Dim(320, 240, 0, 40), "win1", fps);

//	camera.turn(Quatd::fromEuler(0., 0., 0.1));
//	MainLoop::interval(0.01);



	MainLoop::start();
	return 0;
}
