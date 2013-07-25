/*
Allocore Example: cubemap

Description:
This demonstrates a cubemap render-to-texture.

Author:
Graham Wakefield 2011
*/

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_CubeMapFBO.hpp"

#define TEST

using namespace al;

static Graphics gl;
static Mesh mesh, grid, cube;
static Lens lens;
static Nav nav;
static CubeMapFBO cubeFBO;
unsigned drawMode = 1;

struct MyWindow : Window, public Drawable{

	bool onKeyDown(const Keyboard& k){	
		return true;
	}

	bool onFrame(){
		nav.step();
		
		// capture the scene:
		cubeFBO.capture(gl, lens, nav, *this);

		// now use the captured texture:		
		gl.viewport(0, 0, width(), height());
		gl.clearColor(0.2, 0.2, 0.2, 0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		
		// first, draw it as a cylindrical map in the background:
		// ortho for 2D, ranging from 0..1 in each axis:
		gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		gl.modelView(Matrix4d::identity());
		gl.depthTesting(false);
		gl.lighting(false);
		gl.blending(false);
		cubeFBO.drawMap(gl);
					
					
		// second, use it to texture a rotating object:
		gl.projection(Matrix4d::perspective(70, width()/(double)height(), lens.near(), lens.far()));
		gl.modelView(Matrix4d::lookAt(Vec3d(0, 0, 4), Vec3d(0, 0, 2), Vec3d(0, 1, 0)));
		
		// rotate over time:
		gl.rotate(MainLoop::now()*30., 0.707, 0.707, 0.);
		
		gl.lighting(false);
		gl.blending(false);
		gl.depthTesting(true);
		
		// use cubemap texture
		cubeFBO.bind();
		
		// generate texture coordinates from the object:
		GLenum map = GL_OBJECT_LINEAR;
		//GLenum map = GL_REFLECTION_MAP;
		//GLenum map = GL_NORMAL_MAP;
		//GLenum map = GL_EYE_LINEAR;
		//GLenum map = GL_SPHERE_MAP;		
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, map); 
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, map); 
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, map); 
		glEnable(GL_TEXTURE_GEN_S); 
		glEnable(GL_TEXTURE_GEN_T); 
		glEnable(GL_TEXTURE_GEN_R);
		
		gl.draw(cube);
		
		glDisable(GL_TEXTURE_GEN_S); 
		glDisable(GL_TEXTURE_GEN_T); 
		glDisable(GL_TEXTURE_GEN_R);
		
		cubeFBO.unbind();
		
		return true;
	}

	void onDraw(Graphics& gl){
		gl.fog(lens.far(), lens.far()/2, cubeFBO.clearColor());
		gl.depthTesting(1);
		gl.draw(grid);
		gl.draw(mesh);	
	}
};

MyWindow win;

int main(){

	double world_radius = 50;
	
	nav.smooth(0.8);
	lens.near(1).far(world_radius);
	
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
	
	// set up grid:
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
	
	// set up cube:
	cube.color(1,1,1,1);
	cube.primitive(Graphics::TRIANGLES);
	addCube(cube);
	cube.generateNormals();
	
	win.create(Window::Dim(100, 0, 640, 480), "Cube Map FBO Example", 60);
	win.displayMode(win.displayMode() | Window::STEREO_BUF);
	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	MainLoop::start();
	
    return 0;
}
