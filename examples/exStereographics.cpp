
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "graphics/al_Config.h"
#include "protocol/al_GraphicsBackendOpenGL.hpp"
#include "graphics/al_Stereographic.hpp"

using namespace al;

static gfx::GraphicsBackendOpenGL backend;
static gfx::Graphics gl(&backend);
static gfx::Stereographic stereo;

static Nav cam;

struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	 	
		const double a = 1;
		const double v = 0.04;
		/* 
			key codes for my mac: 
		*/
		#define KEY_TAB 9
		
		#define KEY_APOSTROPHE 39
		#define KEY_COMMA 44
		#define KEY_PERIOD 46
		#define KEY_SLASH 47
		
		#define KEY_ARROW_RIGHT 269
		#define KEY_ARROW_UP 270
		#define KEY_ARROW_LEFT 271
		#define KEY_ARROW_DOWN 272
			
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			//case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
			
			case 'w':
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 1, 0, 0));
				cam.vel().quat().set(Quatd::fromEuler(0, -a, 0));
				break;
			case 'x':
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, -1, 0, 0));
				cam.vel().quat().set(Quatd::fromEuler(0, a, 0));
				break;
			case 'a':
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 0, 0, -1));
				cam.vel().quat().set(Quatd::fromEuler(0, 0, a));
				break;
			case 'd':
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 0, 0, 1));
				cam.vel().quat().set(Quatd::fromEuler(0, 0, -a));
				break;
			case KEY_ARROW_RIGHT:
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 0, -1, 0));
				cam.vel().quat().set(Quatd::fromEuler(-a, 0, 0));
				break;
			case KEY_ARROW_LEFT:
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 0, 1, 0));
				cam.vel().quat().set(Quatd::fromEuler(a, 0, 0));
				break;
				
			case KEY_ARROW_UP:
				cam.moveZ(v);
				break;
			case KEY_ARROW_DOWN:
				cam.moveZ(-v);
				break;
			case KEY_APOSTROPHE:
				cam.moveY(v);
				break;
			case KEY_SLASH:
				cam.moveY(-v);
				break;
			case KEY_COMMA:
				cam.moveX(-v);
				break;
			case KEY_PERIOD:
				cam.moveX(v);
				break;
				
			case '`':
				cam.halt(); cam.home();
				break;
				
			case KEY_TAB:
				stereo.stereo(!stereo.stereo());
				break;
				
			case '1':
				stereo.mode(gfx::Stereographic::Anaglyph);
				printf("stereo mode %d\n", stereo.mode());
				break;
			case '2':
				stereo.mode(gfx::Stereographic::Active);
				printf("stereo mode %d\n", stereo.mode());
				break;
			case '3':
				stereo.mode(gfx::Stereographic::Dual);
				printf("stereo mode %d\n", stereo.mode());
				break;
			case '4':
				stereo.mode(gfx::Stereographic::LeftEye);
				printf("stereo mode %d\n", stereo.mode());
				break;
			case '5':
				stereo.mode(gfx::Stereographic::RightEye);
				printf("stereo mode %d\n", stereo.mode());
				break;
				
			default:
				printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
				break;
		}
	}
	void onKeyUp(const Keyboard& k) {
		cam.vel().quat().identity();
		cam.move(0, 0, 0);
	}
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void onFrame(){
	
		al_sec f = al_time();
		al_sec dt = f-n;
		n = f;
		//printf("%f\n", 1./(f-n));
		
		//cam.decay(1.0 - dt*2);
		cam.step(dt*30.);
		
		stereo.draw(gl, cam, render, dimensions().w, dimensions().h, NULL);
	}

	static void render(void * ud) 
	{	
		const Vec3d& pos = cam.pos();
		const Vec3d& ux  = cam.ux();
		const Vec3d& uy  = cam.uy();
		const Vec3d& uz  = cam.uz();
		const Vec3d& eye = pos + ux * stereo.IOD();
		const Vec3d& at  = eye + uz * stereo.focal();
	
		gl.pointSize(4);
		
		
		// draw a reference grid:
		gl.begin(gfx::POINTS);
		double step = 0.125;
		for (double x=-1; x<=1; x+= step) {
		for (double y=-1; y<=1; y+= step) {
		for (double z=-1; z<=1; z+= step) {
			gl.color((x+1)*0.5+0.3, (y+1)*0.5+0.3, (z+1)*0.5+0.3);
			gl.vertex(x, y, z);
		}}}
		gl.end();
		
		// draw axis
		gl.lineWidth(2);
		gl.pushMatrix();
		gl.translate(at[0], at[1], at[2]);
		gl.begin(gfx::LINES);
			gl.color(1, 0, 0);
			gl.vertex(0, 0, 0); gl.vertex(1, 0, 0);
			gl.color(0, 1, 0);
			gl.vertex(0, 0, 0); gl.vertex(0, 1, 0);
			gl.color(0, 0, 1);
			gl.vertex(0, 0, 0); gl.vertex(0, 0, 1);
		gl.end();
		
		gl.translate(1, 0, 0);
		gl.begin(gfx::LINES);
			gl.color(0, 1, 1);
			gl.vertex(0, 0, 0); gl.vertex(ux);
			gl.color(1, 0, 1);
			gl.vertex(0, 0, 0); gl.vertex(uy);
			gl.color(1, 1, 0);
			gl.vertex(0, 0, 0); gl.vertex(uz);
		gl.end();
		gl.popMatrix();
		
	}
	
	al_sec n;
};

int main (int argc, char * const argv[]) {

    MyWindow win;
	win.create(WindowGL::Dim(720,480), "Window 1", 40);
	MainLoop::start();
    return 0;
}
