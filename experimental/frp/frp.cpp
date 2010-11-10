#include "allocore/al_Allocore.hpp"

#include "al_frp.hpp"

using namespace al;

Graphics gl(new GraphicsBackendOpenGL);

struct MyWindow : Window{

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.data().resetBuffers();
		gl.begin(gl.TRIANGLES);
		
		//printf("%d\n", gl.DEPTH_BUFFER_BIT);
		
		for(int i=0; i<2000; ++i){
			//printf("%d ", i);
			float x = rnd::uniformS();
			float y = rnd::uniformS();
			float d = 0.01;

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

int main(){

	MyWindow win1;

	win1.add(new StandardWindowKeyControls);

	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
