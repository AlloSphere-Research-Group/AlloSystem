#include <stdio.h>
#include <stdlib.h>

#include "allocore/al_Allocore.hpp"
using namespace al;

/*
Graphics gl;

Camera camera;
Texture *texture = backend.textureNew();
Texture *surface_tex = backend.textureNew();
Surface *surface = backend.surfaceNew();
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
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	gl.matrixMode(gl.PROJECTION);
	gl.pushMatrix();
	//	gl.loadMatrix(camera.projectionMatrix());

	gl.matrixMode(gl.MODELVIEW);
	gl.pushMatrix();
	//	gl.loadMatrix(camera.modelViewMatrix());


	//surface->enter();
	gl.begin(gl.LINES);
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
		gl.begin(gl.QUADS);
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

		gl.begin(gl.QUADS);
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

	gl.matrixMode(gl.PROJECTION);
	gl.popMatrix();

	gl.matrixMode(gl.MODELVIEW);
	gl.popMatrix();*/
}



struct MyWindow : Window{

	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	bool onFrame(){
		drawScene(this);
		return true;
	}
};



double fps =100;

int main (int argc, char * argv[]) {
	printf("main\n");
	printf("%f\n", MainLoop::get().T0());

	MyWindow win1;

	win1.add(new StandardWindowKeyControls);

/*
	al::Array tex_data;
	tex_data.create2d(3, AlloUInt8Ty, 256, 256);

	for(int j=0; j < 256; j++) {
		unsigned char *data = (unsigned char *)(tex_data.data.ptr + j*tex_data.header.stride[1]);

		for(int i=0; i < 256; i++) {
			*data++ = i;
			*data++ = j;
			*data++ = 255-j;
		}
	}

	texture->fromArray(&tex_data);

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

	win1.create(Window::Dim(320, 240, 0, 40), "win1", fps);

//	camera.turn(Quatd::fromEuler(0., 0., 0.1));
//	MainLoop::interval(0.01);



	MainLoop::start();
	return 0;
}
