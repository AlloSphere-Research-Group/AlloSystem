#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"
#include "alloutil/al_OpenGL.hpp"
//#include "alloutil/al_FrameBufferGL.hpp"

#define TEST

using namespace al;

static GraphicsGL gl;
static Mesh mesh, grid;
static Camera cam;
static Nav nav;
static CubeMapFBO cubeFBO;

struct MyWindow : Window, public Drawable{

	bool onKeyDown(const Keyboard& k){	
		return true;
	}

	bool onFrame(){
		nav.step();
		
		// capture the scene:
		cubeFBO.capture(gl, cam, nav, *this);

		// now draw the captured texture:		
		gl.viewport(0, 0, width(), height());
		gl.clearColor(cubeFBO.clearColor());
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		
		// ortho for 2D, ranging from 0..1 in each axis:
		gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		gl.modelView(Matrix4d::identity());
		
		gl.lighting(false);
		gl.blending(false);
		cubeFBO.drawMap(gl);
		
		return true;
	}

	void onDraw(Graphics& gl){
		gl.fog(cam.far(), cam.far()/2, cubeFBO.clearColor());
		gl.depthTesting(1);
		gl.draw(grid);
		gl.draw(mesh);	
	}
};


int main(){
	double world_radius = 50;
	
	nav.smooth(0.8);
	nav.pos(0, 0, -20);

	cam.near(1).far(world_radius);
	
	// set up mesh:
	mesh.primitive(Graphics::TRIANGLES);
	double tri_size = 2;
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
	win.create(Window::Dim(100, 0, 640, 480), "Cube Map FBO Example", 60);
	
	win.displayMode(win.displayMode() | DisplayMode::StereoBuf);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
