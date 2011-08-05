#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

using namespace al;

static GraphicsGL gl;
static Mesh mesh, grid;
static Stereographic stereo;
static Camera cam;
Nav nav;

struct MyWindow : Window, public Drawable{
	
	bool onKeyDown(const Keyboard& k){	
		
		switch(k.key()){
			case 'o': stereo.omni(!stereo.omni()); return false;
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
		gl.fog(cam.far(), cam.far()/2, stereo.clearColor());
		gl.depthTesting(1);
		gl.draw(grid);
		gl.draw(mesh);	
	}
};


int main(){

	nav.smooth(0.8);
	nav.pos(0, 0, -20);

	cam.near(1).far(100).focalLength(1).fovy(45);
	cam.eyeSep(cam.eyeSepAuto());
	stereo.omni(true, 24, 360);
	stereo.stereo(false);
	stereo.mode(Stereographic::Anaglyph);
	
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
	
	grid.primitive(Graphics::LINES);
	double stepsize = 1./2;
	for (double x=-1; x<=1; x+= stepsize) {
	for (double y=-1; y<=1; y+= stepsize) {
		grid.vertex(x, y, 1);
		grid.vertex(x, y, -1);
	}}
	for (double x=-1; x<=1; x+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
		grid.vertex(x, 1, z);
		grid.vertex(x, -1, z);
	}}
	for (double y=-1; y<=1; y+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
		grid.vertex(1, y, z);
		grid.vertex(-1, y, z);
	}}
	grid.scale(world_radius);
	
	MyWindow win;
	win.create(Window::Dim(100, 0, 640, 480), "Omnigraphic Example", 60);
	
	win.displayMode(win.displayMode() | DisplayMode::StereoBuf);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
