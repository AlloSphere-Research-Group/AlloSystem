#include "allocore/al_Allocore.hpp"

using namespace al;

GraphicsGL gl;

struct MyWindow : Window{

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-3), Vec3d(0,0,0), Vec3d(0,1,0)));

		
		gl.mesh().reset();
		gl.begin(gl.TRIANGLES);
		
		//printf("%d\n", gl.DEPTH_BUFFER_BIT);
		
		for(int i=0; i<2000; ++i){
			//printf("%d ", i);
			float x = rnd::uniformS();
			float y = rnd::uniformS();
			float d = 0.02;

			gl.color(1,0,0);
			gl.vertex(x-d, y-d);
			gl.color(0,1,0);
			gl.vertex(x+d, y-d);
			gl.vertex(x  , y+d);
		}
		
		gl.end();
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
