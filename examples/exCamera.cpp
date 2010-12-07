#include "allocore/al_Allocore.hpp"
#include "al_NavControl.hpp"
using namespace al;

gfx::GraphicsGL gl;
gfx::Mesh stuff;
gfx::Stereographic stereo;
Camera cam, cam2;

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
		//printf("dt %f %f\n", dt, 1./avgFps());
		n = f;

		cam.step(frame_dt);

		stereo.draw(gl, cam, render, dimensions().w, dimensions().h, NULL);
		return true;
	}

	static void render(void * ud){	
		gfx::State state;
		state.depth_enable = true;
		state.blend_enable = false;
		
		state.blend_src = gfx::SRC_ALPHA_SATURATE;
		state.blend_dst = gfx::ONE;
		state.antialias_mode = gfx::NICEST;
		gl.pushState(state);
		
		//glEnable(GL_POLYGON_SMOOTH);

		gl.draw(stuff);
		gl.popState();
	}
	
	al_sec n;
};

int main (int argc, char * const argv[]) {

	// exaggerate stereo:
	cam.eyeSep(1/20.);
	//cam.smooth(0);
	
	// set up stuff:
	stuff.primitive(gfx::TRIANGLES);
	int N=3000;
	double r = 2;
	double f1 = 101;
	double f2 = 166;
	for(int i=0; i<N; i++){
		double p = float(i)/N*M_2PI;
		double x = r*cos(f1*p) * sin(f2*p);
		double y = r*sin(f1*p) * sin(f2*p);
		double z = r*cos(f2*p);
		double c = cos(p)*0.25 + 0.25;

		stuff.addColor(HSV(0.2+c, 0.5, i%3 ? 0.5 : 0));
		stuff.addVertex(x, y, z);
	}

	//printf("%lu %lu\n", sizeof(Nav) + sizeof(NavControls), sizeof(NavSmooth));

    MyWindow win;
	win.create(Window::Dim(720,480), "Window 1", 40);

	win.add(new StandardWindowKeyControls).add(new NavInputControl(&cam));
	MainLoop::start();
    return 0;
}
