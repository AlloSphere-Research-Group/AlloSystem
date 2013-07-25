/*
Allocore Example: Lighting

Description:
This example demonstrates how to use lighting within a scene.

Author:
Lance Putnam, 12/2010 (putnam.lance at gmail dot com)
*/

#include "allocore/al_Allocore.hpp"

using namespace al;

struct MyWindow : Window{

	MyWindow(){
		phase = 0.5;
		
		surface.primitive(Graphics::TRIANGLE_STRIP);
		surface.color(1,1,1);
		
		const int Nx=30, Ny=30;
		for(int j=0; j<Ny; ++j){ float y=float(j)/(Ny-1);
		for(int i=0; i<Nx; ++i){ float x=float(i)/(Nx-1);
			surface.vertex(x*4-2, y*4-2);
			surface.normal(0,0,1);
		}}
		
		for(int j=0; j<Ny-1; ++j){
			for(int i=0; i<Nx  ; ++i){
				int idx = j*Nx + i;
				surface.index(idx);
				surface.index(idx+Nx);
			}
			int idx = surface.indices().last();
			surface.index(idx);
			surface.index(idx - Nx + 1);
		}
	}


	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,5), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(1);
		gl.blending(0);

		// Set light position
		phase += 1./1800; if(phase > 1) phase -= 1;
		float x = cos(7*phase*2*M_PI);
		float y = sin(11*phase*2*M_PI);
		float z = cos(phase*2*M_PI)*0.5 + 0.5;
		
		light.pos(x,y,z);
		light.diffuse(Color(1,0,0));

		gl.mesh().reset();
		gl.mesh().primitive(gl.TRIANGLES);
		gl.mesh().color(1,1,1);
		addSphere(gl.mesh(),0.1);
		gl.mesh().translate(x,y,z);
		gl.draw();

		material();
		light();

		gl.draw(surface);

		return true;
	}

	Graphics gl;
	Light light;
	Material material;
	Mesh surface;
	double phase;
};

MyWindow win;

int main(){
	win.append(*new StandardWindowKeyControls);
	win.create();
	MainLoop::start();
	return 0;
}
