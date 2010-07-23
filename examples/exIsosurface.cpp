#include <math.h>
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "graphics/al_Isosurface.hpp"

using namespace al;

const int N = 32;
float volData[N*N*N];
Isosurface<float> iso;
double phase=0;

struct MyWindow : WindowGL{
	
	void onKeyDown(const Keyboard& k){
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
		}
	}

	void onFrame(){
		gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);

		if((phase += 0.001) > 2*M_PI) phase -= 2*M_PI;

		for(int k=0; k<N; ++k){
		for(int j=0; j<N; ++j){
		for(int i=0; i<N; ++i){
			double x = double(i)/N * 4*M_PI;
			double y = double(j)/N * 4*M_PI;
			double z = double(k)/N * 4*M_PI;
			volData[k*N*N + j*N + i] 
				= cos(x * cos(phase*7)) 
				+ cos(y * cos(phase*8)) 
				+ cos(z * cos(phase*9));
		}
		}
		}

		iso.primitive(gfx::TRIANGLES);
		iso.level(0);
		iso.resetBuffers();
		iso.generate(volData, N, 1./N);
		gl.draw(iso);
	}

	gfx::Graphics gl;

};


int main (int argc, char * argv[]){

	MyWindow win;
	win.create(WindowGL::Dim(800,600), "Isosurface Example", 40);

	WindowGL::startLoop();
	return 0;
}
