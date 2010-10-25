#include "al_Allocore.hpp"
#include "al_NavControl.hpp"

using namespace al;

static gfx::GraphicsBackendOpenGL backend;
static gfx::Graphics gl(&backend);

static gfx::GraphicsData stuff;

static gfx::Stereographic stereo;
static Camera cam;

static rnd::Random<> rng;


//class NavControls {
//public:
//	NavControls(double turnRate = 2, double slide = 0.85) 
//	: mSlide(slide) {}
//	
//	void moveX(double v) { mMove[0] = v; }
//	void moveY(double v) { mMove[1] = v; }
//	void moveZ(double v) { mMove[2] = v; }
//	void turnX(double v) { mTurn[0] = v; }
//	void turnY(double v) { mTurn[1] = v; }
//	void turnZ(double v) { mTurn[2] = v; }
//	void halt() { mMove.set(0); mTurn.set(0); }
//	
//	void update(al_sec dt, Camera &cam) {
//		double amt = 1.-mSlide;	// TODO: adjust for dt
//		mMove1.lerp(mMove, amt);
//		mTurn1.lerp(mTurn, amt);
//		cam.vel().quat().set(Quatd::fromEuler(mTurn1[1], mTurn1[0], mTurn1[2]));
//		cam.vel().vec().set(mMove1);
//	}
//protected:
//	Vec3d mMove, mMove1;
//	Vec3d mTurn, mTurn1;
//	double mSlide;
//};
//
//static NavControls navcontrols;


struct MyWindow : Window{
	
	bool onKeyDown(const Keyboard& k){	
		
		switch(k.key()){
			case Key::Tab: stereo.stereo(!stereo.stereo()); return false;
			case '1': stereo.mode(gfx::Stereographic::Anaglyph); return false;
			case '2': stereo.mode(gfx::Stereographic::Active); return false;
			case '3': stereo.mode(gfx::Stereographic::Dual); return false;
			case '4': stereo.mode(gfx::Stereographic::LeftEye); return false;
			case '5': stereo.mode(gfx::Stereographic::RightEye); return false;
			default: return true;
		}
	}

	bool onFrame(){
	
		al_sec f = al_time();
		al_sec dt = f-n;
		al_sec frame_dt = dt*fps();
		//printf("dt %f\n", dt);
		n = f;

		cam.step(frame_dt);
		
		stereo.draw(gl, cam, render, dimensions().w, dimensions().h, NULL);
		return true;
	}

	static void render(void * ud) 
	{	
		gfx::State state;
		state.depth_enable = false;
		state.blend_enable = true;
		
		state.blend_src = gfx::SRC_ALPHA_SATURATE;
		state.blend_dst = gfx::ONE;
		state.antialias_mode = gfx::NICEST;
		gl.pushState(state);
		
		//glEnable(GL_POLYGON_SMOOTH);
		
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
	
	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(&cam));
	
	win.create(Window::Dim(720,480), "Window 1", 40);
	MainLoop::start();
    return 0;
}
