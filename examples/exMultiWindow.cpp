
#include <stdio.h>
#include <stdlib.h>

#include "types/al_MsgQueue.hpp"
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "math/al_Random.hpp"

using namespace al;

Camera cam;

// invent some kind of scene:
struct vertex {
	float x, y, z;
	float r, g, b;
};
#define NUM_VERTICES (1024)
static vertex vertices[NUM_VERTICES];

struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); 
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void onFrame(){
		al_sec t = al_time();
		avg += 0.1*((t-last)-avg);
		last = t;
		
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);
		
		gl.begin(gl.LINE_STRIP);
			static float limit = NUM_VERTICES;
			for (int i = 0; i<NUM_VERTICES; i++) {
				float p = i / limit;
				vertex& v = vertices[i];
				gl.color(v.r, v.g, v.b);
				gl.vertex(v.x, v.y, v.z);
			}
		gl.end();
//printf("%p: %d x %d\n", this, dimensions().w, dimensions().h);
	}
	
	void freqs(float v1, float v2){ freq1=v1; freq2=v2; }

	gfx::Graphics gl;
	float freq1, freq2;
	al_sec last;
	al_sec avg;
};

MyWindow win;
MyWindow win2;

void set_freqs(al_sec t, double offset) {
	win.freqs(t, t+offset);
	win2.freqs(t, t+offset);
	printf("set_freqs @%f, fps %f\n", t, 1.0/win.avg);
	MainLoop::queue().send(t+1., set_freqs, offset);
}

int main (int argc, char * argv[]) {

	rnd::Random<> rng;

	/// define the scene - a semi random walk
	vertex v0 = { 
		0, 0, 0,
		rng.uniform(), rng.uniform(), rng.uniform()
	};
	for (int i=1; i<NUM_VERTICES; i++) {
		float ratio = 0.5 * sin(2.0*M_PI*sin(i));
		float antiratio = 0.5 * cos(2.0*M_PI*sin(i));
		float scale = 3;
		vertices[i].x = v0.x + antiratio * (scale * vertices[i-1].r) + ratio * vertices[i-1].x;
		vertices[i].y = v0.y + antiratio * (scale * vertices[i-1].g) + ratio * vertices[i-1].y;
		vertices[i].z = v0.z + antiratio * (scale * vertices[i-1].b) + ratio * vertices[i-1].z;
		
		float twist = 0.25;
		vertices[i].r = vertices[i-1].r + twist * (rng.uniform() - vertices[i-1].r);
		vertices[i].g = vertices[i-1].g + twist * (rng.uniform() - vertices[i-1].g);
		vertices[i].b = vertices[i-1].b + twist * (rng.uniform() - vertices[i-1].b);

	}

	/// Mainloop is created implicitly by the first reference to it; 
	/// e.g., the creation of a WindowGL, or directly via MainLoop:: calls.
	
	/// decreasing the interval may lead to more accurate timing, but more expensive
	/// reducing it to near the refresh rate may cause jerky animation
	MainLoop::interval(0.0025); 
	
	win.create(WindowGL::Dim(200,200,0), "left", 40);
	win2.create(WindowGL::Dim(200,200,200), "right", 40);
	win.freqs(1,2);
	win.freqs(1,2);
	win.avg = 0;
	
	/// set these windows to use the same camera:
	win.viewport().camera(&cam);
	win2.viewport().camera(&cam);

	MainLoop::queue().send(1.0, set_freqs, 1.);
	MainLoop::start();

	return 0;
}
