/*
Allocore Example: Freenect

Description:
Demonstrating a binding to libfreenect

first 'make allonect'

Author:
Graham Wakefield, 2011
*/

#include "allonect/al_Freenect.hpp"



using namespace al;

Graphics gl;
FreenectDepthViewer depth0(0);
FreenectDepthViewer depth1(1);

struct MyWindow : Window{

	bool onCreate(){
		return true;
	}

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,3), Vec3d(0,0,0), Vec3d(0,1,0)));

		
		depth0.tex.bind();

			gl.begin(gl.QUADS);
			gl.vertex( 0,  1);
			gl.vertex(-1,  1);
			gl.vertex(-1,  0);
			gl.vertex( 0,  0);
			gl.texCoord(1,0);
			gl.texCoord(0,0);
			gl.texCoord(0,1);
			gl.texCoord(1,1);
			gl.end();

		depth0.tex.unbind();
		
		depth1.tex.bind();

			gl.begin(gl.QUADS);
			gl.vertex( 1,  1);
			gl.vertex( 0,  1);
			gl.vertex( 0,  0);
			gl.vertex( 1,  0);
			gl.texCoord(1,0);
			gl.texCoord(0,0);
			gl.texCoord(0,1);
			gl.texCoord(1,1);
			gl.end();

		depth1.tex.unbind();

		return true;
	}
	
	
};

MyWindow win;

int main(){

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(800, 600));
	
	
	Freenect::get().start();

	MainLoop::start();
	return 0;
}
