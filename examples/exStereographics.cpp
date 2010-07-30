
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


class NavControls {
public:
	NavControls(double moveSpeed = 0.5, double turnSpeed = 2, double slide = 0.9) 
	: mMoveRate(moveSpeed), mTurnRate(turnSpeed), mSlide(slide) {}
	
	bool keyDown(Camera &cam, int k) {
		switch (k) {
			case 'w':
				turn[0] = -1;
				return true;
			case 'x':
				turn[0] = 1;
				return true;
			case 'a':
				turn[2] = 1;
				return true;
			case 'd':
				turn[2] = -1;
				return true;
			case Key::Right:
				turn[1] = 1;
				return true;
			case Key::Left:
				turn[1] = -1;
				return true;
			case Key::Up:
				move[2] = mMoveRate;
				return true;
			case Key::Down:
				move[2] = -mMoveRate;
				return true;
			case '\'':
				move[1] = mMoveRate;
				return true;
			case '/':
				move[1] = -mMoveRate;
				return true;
			case ',':
				move[0] = -mMoveRate;
				return true;
			case '.':
				move[0] = mMoveRate;
				return true;
				
			case '`':
				cam.halt(); cam.home();
				return true;
		}
		return false;
	}
	
	bool keyUp(Camera &cam, int k) {
		switch (k) {
			case 'w':
				turn[0] = 0;
				return true;
			case 'x':
				turn[0] = 0;
				return true;
			case 'a':
				turn[2] = 0;
				return true;
			case 'd':
				turn[2] = 0;
				return true;
			case Key::Right:
				turn[1] = 0;
				return true;
			case Key::Left:
				turn[1] = 0;
				return true;
			case Key::Up:
				move[2] = 0;
				return true;
			case Key::Down:
				move[2] = 0;
				return true;
			case '\'':
				move[1] = 0;
				return true;
			case '/':
				move[1] = 0;
				return true;
			case ',':
				move[0] = 0;
				return true;
			case '.':
				move[0] = 0;
				return true;	
			case '`':
				turn.set(0); move.set(0);
				return true;
		}
		return false;
	}
	
	void update(double dt, Camera &cam) {
		double amt = 1.-pow(mSlide,dt);
		move1.lerp(move, amt);
		turn1.lerp(turn, amt);
		cam.vel().quat().set(Quatd::fromAxisAngle(mTurnRate, turn1[0], turn1[1], turn1[2]));
		cam.vel().vec().set(move1);
	}
protected:
	Vec3d move, move1;
	Vec3d turn, turn1;
	
	double mMoveRate;
	double mTurnRate;
	
	double mSlide;
};

static NavControls navcontrols;


struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	 	
		
		if (navcontrols.keyDown(cam, k.key())) return;
			
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			//case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
				
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
		navcontrols.keyUp(cam, k.key());
	}
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		//printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void onFrame(){
	
		al_sec f = al_time();
		al_sec dt = f-n;
		n = f;
		
		al_sec frame_dt = dt*fps();
		
		navcontrols.update(frame_dt, cam);
		cam.step(frame_dt);
		
		stereo.draw(gl, cam, render, dimensions().w, dimensions().h, NULL);
	}

	static void render(void * ud) 
	{	
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
	double tri_size = 1;
	double world_radius = 25;
	for (int i=0; i<1024; i++) {
		double x = rng.uniformS(world_radius);
		double y = rng.uniformS(world_radius);
		double z = rng.uniformS(world_radius);
		double c = rng.uniform(0.5);
		for (int v=0; v<3; v++) {
			stuff.addColor(0.5+c, 0.5, 1-c);
			stuff.addVertex(x+rng.uniformS(tri_size), y+rng.uniformS(tri_size), z+rng.uniformS(tri_size));
		}
	}
		
    MyWindow win;
	win.create(WindowGL::Dim(720,480), "Window 1", 40);
	MainLoop::start();
    return 0;
}
