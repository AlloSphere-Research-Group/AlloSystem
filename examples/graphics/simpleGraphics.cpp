/*
Allocore Example: Simple 3D graphics test

Description:
This renders a colored square

	(-1, 1, 0)		( 1, 1, 0)
	Red				Blue


	(-1,-1, 0)		( 1,-1, 0)
	Green			White

that can be helpful for understanding viewing transformations.

Author:
Lance Putnam, 7/6/2011
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

Graphics gl;

struct MyWindow : Window{

	bool onFrame(){

		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.viewport(0,0, width(), height());

		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,4), Vec3d(0,0,0), Vec3d(0,1,0)));

//		gl.loadIdentity();
//		gluLookAt(0,0,4, 0,0,0, 0,1,0);

//		gl.rotate(sin(al_time()) * 90., 0, 1, 0);
		gl.begin(gl.TRIANGLE_STRIP);
			gl.color(1,0,0);	gl.vertex(-1, 1);
			gl.color(0,1,0);	gl.vertex(-1,-1);
			gl.color(0,0,1);	gl.vertex( 1, 1);			
			gl.color(1,1,1);	gl.vertex( 1,-1);
		gl.end();

/*		gl.begin(gl.TRIANGLE_STRIP);
			gl.color(1,0,0);	gl.vertex(1,0,0);
			gl.color(0,1,0);	gl.vertex(0,1,0);
			gl.color(0,0,1);	gl.vertex(0,0,1);
		gl.end();*/
		return true;
	}
};

MyWindow win1;

int main(){
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
