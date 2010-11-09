#include "utAllocore.h"

static GraphicsBackendOpenGL backend;
static Graphics gl(&backend);
 
struct MyWindow2 : Window{

	bool onFrame(){
		
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, dimensions().w, dimensions().h);
		gl.loadIdentity();
		
		data.resetBuffers();
	
		// test vertices and colors
		int N=19;
		for(int i=0; i<=N; ++i){
			float p = float(i)/N;
			data.addVertex(cos(p*M_2PI), sin(p*M_2PI));
			data.addColor(p, p, 1);
		}

		// test rendering from index array
		for(int i=0; i<=N; ++i){
			data.addIndex((i*(N/2-1)) % N);
		}

		//gl.primitive(gl.TRIANGLE_STRIP);
		data.primitive(gl.LINE_STRIP);

		gl.draw(data);

		return true;
	}
	
	GraphicsData data;
};



int utProtocolGraphics(){

	MyWindow2 win;

	win.create(Window::Dim(400,400), "Window 1", 40);

	Window::startLoop();
	return 0;
}
