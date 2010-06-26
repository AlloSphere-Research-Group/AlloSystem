
#include <stdio.h>
#include <stdlib.h>

#include "types/al_MsgQueue.hpp"
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "math/al_Random.hpp"

#define deg2rad(deg) (deg*0.0174532925)

using namespace al;


class Cam : public Nav {
public:
	double focal, near, far, fovy;
	
};

Cam cam;
Camera camera;

static void setStereoFrustum(Camera& cam, double aspect, double eyesep) {
	
	// typically choose IOD to be 1/30 of the focal length
	double IOD = eyesep * cam.focalLength()/30.0;
	double top = cam.near() * tan( deg2rad( cam.fovy() / 2 ));	
	double right = aspect * top;				
	double frustumshift = (IOD/2)*cam.near()/cam.focalLength();
	double l, r, t, b, n, f;
	
	t = top;
	b = -top;
	l = -right + frustumshift;
	r = right + frustumshift;
	n = cam.near();
	f = cam.far();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();                                       //reset projection matrix
	glFrustum(l, r, b, t, n, f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadIdentity();
	const Vec3d& vec = camera.vec();
	const Vec3d& ux  = camera.ux();
	const Vec3d& uy  = camera.uy();
	const Vec3d& uz  = camera.uz();
	const Vec3d eye = vec + (ux * IOD/2);
	const Vec3d at = eye + (uz * cam.focalLength());
	gluLookAt(	eye[0],	eye[1],	eye[2],
				at[0],	at[1],	at[2],
				uy[0],	uy[1],	uy[2]);
	
}

// invent some kind of scene:
struct vertex {
	float x, y, z;
	float r, g, b;
};
#define NUM_VERTICES (256)
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
		
		//viewport().camera()->focalLength(10 * sin(al_time()));
		viewport().camera().vec()[0] = 10 * sin(al_time());
		
		viewport().dimensions(dimensions().w, dimensions().h);
		viewport().applyFrustumStereo(eyesep);
		
		glDrawBuffer(GL_BACK);                                   //draw into both back buffers
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);     //clear color and depth buffers
		gl.loadIdentity();
		
		
		al_sec t = al_time();
		avg += 0.1*((t-last)-avg);
		last = t;
		
//		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
//		gl.loadIdentity();
//		gl.viewport(0,0, dimensions().w, dimensions().h);
		
		gl.begin(gl.LINES);
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
	
	double eyesep;
};

MyWindow win;
MyWindow win2;

int main (int argc, char * argv[]) {

	rnd::Random<> rng;

	/// define the scene - a semi random walk
	for (int i=0; i<NUM_VERTICES; i++) {
	
		double r, g, b, x, y, z;
		
		double p = i/(double)NUM_VERTICES;
		r = p;
		g = 1-p;
		b = sin(p * 6 * M_PI);
	
		y = b;
		z = -5 + sin(p * 2 * M_PI);
		
		vertices[i].x = -1;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;
		
		i++;
		
		vertices[i].x = 1;
		vertices[i].y = y;
		vertices[i].z = z;
		vertices[i].r = r;
		vertices[i].g = g;
		vertices[i].b = b;

	} 
	
	win.create(WindowGL::Dim(640,480,0), "left", 40);
	win2.create(WindowGL::Dim(640,480,640), "right", 40);
	win.avg = 0;
	
	win.eyesep = 2;
	win2.eyesep = -win.eyesep;
	
	/// set these windows to use the same camera:
	win.viewport().camera(&camera);
	win2.viewport().camera(&camera);

	MainLoop::start();
	return 0;
}
