#include "allocore/io/al_Window.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Lens.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Stereoscopic.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/math/al_Random.hpp"

using namespace al;

static Graphics gl;
static Mesh mesh;
static Stereoscopic stereo;
static Lens lens;
Nav nav;

struct MyWindow : Window, public Drawable{

	bool onKeyDown(const Keyboard& k){

		switch(k.key()){
			case '-': lens.eyeSep(lens.eyeSep() - 0.01); break;
			case '+': lens.eyeSep(lens.eyeSep() + 0.01); break;
			case Keyboard::TAB: stereo.stereo(!stereo.stereo()); return false;
			case '1': stereo.mode(Stereoscopic::ANAGLYPH); return false;
			case '2': stereo.mode(Stereoscopic::ACTIVE); return false;
			case '3': stereo.mode(Stereoscopic::DUAL); return false;
			case '4': stereo.mode(Stereoscopic::LEFT_EYE); return false;
			case '5': stereo.mode(Stereoscopic::RIGHT_EYE); return false;
			default:;
		}
		return true;
	}

	bool onFrame(){
		nav.step();
		stereo.draw(gl, lens, nav, Viewport(width(), height()), *this);
		printf("fps %f\n", fpsAvg());
		return true;
	}

	void onDraw(Graphics& gl){
		gl.fog(lens.far(), lens.far()/2, stereo.clearColor());
		gl.depthTesting(1);
		gl.draw(mesh);
	}
};

MyWindow win;

int main(){

	nav.smooth(0.8);
	nav.pos(0, 0, -20);

	lens.near(1).far(100).focalLength(1).fovy(45);
	lens.eyeSep(lens.eyeSepAuto());
	stereo.stereo(true);
	stereo.mode(Stereoscopic::ACTIVE);


	// set up mesh:
	mesh.primitive(Graphics::TRIANGLES);
	double tri_size = 2;
	double world_radius = 50;
	int count = 4000;
	for (int i=0; i<count; i++) {
		double x = rnd::uniformS(world_radius);
		double y = rnd::uniformS(world_radius);
		double z = rnd::uniformS(world_radius);
		for (int v=0; v<3; v++) {
			mesh.color(HSV(float(i)/count, v!=2, 1));
			mesh.vertex(x+rnd::uniformS(tri_size), y+rnd::uniformS(tri_size), z+rnd::uniformS(tri_size));
		}
	}

	win.create(Window::Dim(100, 0, 640, 480), "Stereoscopic Example", 60, Window::DEFAULT_BUF | Window::STEREO_BUF);

	//win.create(Window::Dim(100, 0, 640, 480), "Stereoscopic Example", 60);
	//win.displayMode(win.displayMode() | Window::STEREO_BUF);

	win.append(*new StandardWindowKeyControls);
	win.append(*new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
