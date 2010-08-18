
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
	NavControls(double turnRate = 2, double slide = 0.85) 
	: mSlide(slide) {}
	
	void moveX(double v) { mMove[0] = v; }
	void moveY(double v) { mMove[1] = v; }
	void moveZ(double v) { mMove[2] = v; }
	void turnX(double v) { mTurn[0] = v; }
	void turnY(double v) { mTurn[1] = v; }
	void turnZ(double v) { mTurn[2] = v; }
	void halt() { mMove.set(0); mTurn.set(0); }
	
	void update(al_sec dt, Camera &cam) {
		double amt = 1.-mSlide;	// TODO: adjust for dt
		mMove1.lerp(mMove, amt);
		mTurn1.lerp(mTurn, amt);
		cam.vel().quat().set(Quatd::fromEuler(mTurn1[1], mTurn1[0], mTurn1[2]));
		cam.vel().vec().set(mMove1);
	}
protected:
	Vec3d mMove, mMove1;
	Vec3d mTurn, mTurn1;
	double mSlide;
};

static NavControls navcontrols;


struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	 	
	
		static double a = 1;		// rotational speed: degrees per update
		static double v = 0.25;		// speed: units per update
		
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			//case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
				
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
				
			case '`':
				navcontrols.halt();
				cam.halt(); 
				cam.home();
				break;					
			case 'w':
				navcontrols.turnX(-a);
				break;
			case 'x':
				navcontrols.turnX(a);
				break;
			case Key::Right:
				navcontrols.turnY(a);
				break;
			case Key::Left:
				navcontrols.turnY(-a);
				break;
			case 'a':
				navcontrols.turnZ(a);
				break;
			case 'd':
				navcontrols.turnZ(-a);
				break;
			case ',':
				navcontrols.moveX(-v);
				break;
			case '.':
				navcontrols.moveX(v);
				break;
			case '\'':
				navcontrols.moveY(v);
				break;
			case '/':
				navcontrols.moveY(-v);
				break;
			case Key::Up:
				navcontrols.moveZ(v);
				break;
			case Key::Down:
				navcontrols.moveZ(-v);
				break;
				
			default:
				printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
				break;
		}
	}
	void onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case 'w':
			case 'x':
				navcontrols.turnX(0);
				break;
			case Key::Right:
			case Key::Left:
				navcontrols.turnY(0);
				break;
			case 'a':
			case 'd':
				navcontrols.turnZ(0);
				break;
			case ',':
			case '.':
				navcontrols.moveX(0);
				break;
			case '\'':
			case '/':
				navcontrols.moveY(0);
				break;
			case Key::Up:
			case Key::Down:
				navcontrols.moveZ(0);
				break;
			default:
				break;
		}
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
		al_sec frame_dt = dt*fps();
		//printf("dt %f\n", dt);
		n = f;
		
		navcontrols.update(dt, cam);
		cam.step(frame_dt);
		
		stereo.draw(gl, cam, render, dimensions().w, dimensions().h, NULL);
	}

	static void render(void * ud) 
	{	
		gfx::State state;
		state.depth_enable = false;
		state.blend_enable = true;
		
		state.blend_src = gfx::SRC_ALPHA_SATURATE;
		state.blend_dst = gfx::ONE;
		state.antialias_mode = gfx::AntiAliasMode::Nicest;
		gl.pushState(state);
		
		//glEnable(GL_POLYGON_SMOOTH);
		
		gl.pointSize(2);
		gl.draw(stuff);		
		gl.popState();
		
	}
	
	al_sec n;
};

int main (int argc, char * const argv[]) {

	printf("%d %d\n", sizeof(Foo), sizeof(Bar));

	// exaggerate stereo:
	cam.eyeSep(1/20.);
	
	// set up stuff:
	stuff.primitive(gfx::TRIANGLES);
	double tri_size = 2;
	double world_radius = 50;
	for (int i=0; i<4000; i++) {
		double x = rng.uniformS(world_radius);
		double y = rng.uniformS(world_radius);
		double z = rng.uniformS(world_radius);
		double c = rng.uniform(0.5);
		for (int v=0; v<3; v++) {
			stuff.addColor(0.5+c, 0.5, 1-c, 0.5);
			stuff.addVertex(x+rng.uniformS(tri_size), y+rng.uniformS(tri_size), z+rng.uniformS(tri_size));
		}
	}
		
    MyWindow win;
	win.create(WindowGL::Dim(720,480), "Window 1", 40);
	MainLoop::start();
    return 0;
}
