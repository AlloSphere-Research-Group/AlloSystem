/*
Allocore Example: omnigraphic

Description:
This demonstrates stereo panorama by omnigraphic method

Author:
Graham Wakefield 2011
*/

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"

using namespace al;

static Graphics gl;
static Mesh mesh, grid;
static Stereographic stereo;
static Camera cam;
Nav nav;

/*
	Omnigraphic mode splits up the window into vertical slices (viewports)
	Each viewport has a slightly rotated view of the scene, such that the 
	borders match up and create a seamless panoramic view.
	In effect, it presents a cylindrical map.
	With stereographics, it creates omnistereo (where stereoscopy is continuous
	around the cylinder)
	
	However it can be expensive, as each slice is a full scene render.
	
	Perhaps a similar effect can be achieved with a shader?:
	
		According to the polar angle of a vertex, displace it with a modified 
		version of the ModelViewProjection matrix (instead of e.g. fttransform())
	
		The modelview/projection matrices can be calculated in the same way as 
		the Stereographic class (perspectiveLeft/Right, lookatLeft/Right)
	
	The advantage would be full spherical omnistereo, and hopefully cheaper.
*/

struct MyWindow : Window, public Drawable{
	
	bool onKeyDown(const Keyboard& k){	
		
		switch(k.key()){
			case 'o': stereo.omni(!stereo.omni()); return false;
			case Keyboard::TAB: stereo.stereo(!stereo.stereo()); return false;
			case '1': stereo.mode(Stereographic::ANAGLYPH); return false;
			case '2': stereo.mode(Stereographic::ACTIVE); return false;
			case '3': stereo.mode(Stereographic::DUAL); return false;
			case '4': stereo.mode(Stereographic::LEFT_EYE); return false;
			case '5': stereo.mode(Stereographic::RIGHT_EYE); return false;
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

MyWindow win;

int main(){

	nav.smooth(0.8);
	nav.pos(0, 0, -20);

	cam.near(1).far(100).focalLength(1).fovy(45);
	cam.eyeSep(-cam.eyeSepAuto());
	stereo.omni(true, 24, 120);
	stereo.stereo(false);
	stereo.mode(Stereographic::ANAGLYPH);
	
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
	
	win.create(Window::Dim(100, 0, 640, 480), "Omnigraphic Example", 60);
	
	win.displayMode(win.displayMode() | Window::STEREO_BUF);

	win.append(*new StandardWindowKeyControls);
	win.append(*new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
