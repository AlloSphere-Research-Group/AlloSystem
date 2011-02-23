#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

using namespace al;

static GraphicsGL gl;
static Mesh mesh;
static Stereographic stereo;
static Camera cam;
Nav nav;

struct MyWindow : Window, public Drawable{
	
	bool onKeyDown(const Keyboard& k){	
		
		switch(k.key()){
			case Key::Tab: stereo.stereo(!stereo.stereo()); return false;
			case '1': stereo.mode(Stereographic::Anaglyph); return false;
			case '2': stereo.mode(Stereographic::Active); return false;
			case '3': stereo.mode(Stereographic::Dual); return false;
			case '4': stereo.mode(Stereographic::LeftEye); return false;
			case '5': stereo.mode(Stereographic::RightEye); return false;
			default: return true;
		}
	}

	bool onFrame(){
		nav.step();
		stereo.draw(gl, cam, nav, Viewport(width(), height()), *this);
		return true;
	}

	void onDraw(Graphics& gl){
		gl.depthTesting(1);
		gl.draw(mesh);	
	}
};


int main(){

	nav.smooth(0.9);
	nav.pos(0, 0, -20);

	// exaggerate stereo:
	cam.eyeSep(1/20.);
	stereo.stereo(true);

	
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

	MyWindow win;
	win.create(Window::Dim(100, 0, 640, 480), "Stereographic Example");
	
	win.displayMode(win.displayMode() | DisplayMode::StereoBuf);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
