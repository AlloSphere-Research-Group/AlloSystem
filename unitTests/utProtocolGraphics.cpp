#include "utAllocore.h"

 
struct MyWindow2 : WindowGL{

	void onFrame(){
		using namespace al::gfx;
		
		gl.setBackend(Backend::OpenGL);

		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, dimensions().w, dimensions().h);
		gl.loadIdentity();
		
		gl.resetBuffers();
		
		int N=19;
		for(int i=0; i<=N; ++i){
			float p = float(i)/N;
			gl.addVertex(cos(p*M_2PI), sin(p*M_2PI));
			gl.addColor(p, p, 1);
		}

		for(int i=0; i<=N; ++i){
			gl.addIndex((i*(N/2-1)) % N);
		}

		//gl.primitive(gl.TRIANGLE_STRIP);
		gl.primitive(gl.LINE_STRIP);

		gl.draw();
	}
	
	gfx::Graphics gl;
};



int utProtocolGraphics(){

	MyWindow2 win;

	win.create(WindowGL::Dim(400,400,000), "Window 1", 40);

	WindowGL::startLoop();
	return 0;
}
