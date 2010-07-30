
#include "system/al_MainLoop.hpp"
#include "io/al_WindowGL.hpp"
#include "graphics/al_Config.h"
#include "protocol/al_GraphicsBackendOpenGL.hpp"
#include "graphics/al_Stereographic.hpp"
#include "math/al_Random.hpp"

using namespace al;

static gfx::GraphicsBackendOpenGL backend;
static gfx::Graphics gl(&backend);

static gfx::GraphicsData stuff;

static gfx::Stereographic stereo;
static Camera cam;

static rnd::Random<> rng;

struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	 	
		const double a = 0.5;
		const double v = 0.1;
			
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
			case Key::Right:
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 0, 1, 0));
				cam.vel().quat().set(Quatd::fromEuler(a, 0, 0));
				break;
			case Key::Left:
				//cam.vel().quat().set(Quatd::fromAxisAngle(a, 0, -1, 0));
				cam.vel().quat().set(Quatd::fromEuler(-a, 0, 0));
				break;
				
			case Key::Up:
				cam.moveZ(v);
				break;
			case Key::Down:
				cam.moveZ(-v);
				break;
			case '\'':
				cam.moveY(v);
				break;
			case '/':
				cam.moveY(-v);
				break;
			case ',':
				cam.moveX(-v);
				break;
			case '.':
				cam.moveX(v);
				break;
				
			case '`':
				cam.halt(); cam.home();
				break;
				
			case Key::Tab:
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
		glEnable(GL_DEPTH_TEST); //<< why is this glitchy?
		
		gfx::State state;
		state.depth_enable = true;
		gl.pushState(state);
		
		gl.pointSize(2);
		gl.draw(stuff);		
		gl.popState();
		
	}
	
	al_sec n;
};



int main (int argc, char * const argv[]) {

	// exaggerate stereo:
	cam.eyeSep(1/20.);
	
	// set up stuff:
	stuff.primitive(gfx::TRIANGLES);
	double size = 0.5;
	for (int i=0; i<256; i++) {
		double x = rng.uniformS(4.);
		double y = rng.uniformS(4.);
		double z = rng.uniformS(4.);
		double c = rng.uniformS(M_PI);
		for (int v=0; v<3; v++) {
			stuff.addColor(0.5+cos(c), 0.5, 0.5+sin(c));
			stuff.addVertex(x+rng.uniformS(size), y+rng.uniformS(size), z+rng.uniformS(size));
		}
	}
		
    MyWindow win;
	win.create(WindowGL::Dim(720,480), "Window 1", 40);
	MainLoop::start();
    return 0;
}
