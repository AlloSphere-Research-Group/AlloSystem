#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
#include "alloutil/al_ControlNav.hpp"

using namespace al;

GraphicsGL gl;
Light light;
Material material;
Camera cam;
Nav nav;
Stereographic stereo;

const int N = 32;
float volData[N*N*N];
Isosurface<float> iso;
double phase=0;

void drawbox() {
	gl.begin(gl.LINES);
	gl.color(1,1,1);
	for (int z=-1; z<=1; z+=2) {
		for (int y=-1; y<=1; y+=2) {
			for (int x=-1; x<=1; x+=2) {
				gl.vertex(x, y, z);
			}
		}
		for (int y=-1; y<=1; y+=2) {
			for (int x=-1; x<=1; x+=2) {
				gl.vertex(y, x, z);
			}
		}
		for (int y=-1; y<=1; y+=2) {
			for (int x=-1; x<=1; x+=2) {
				gl.vertex(y, z, x);
			}
		}
	}
	gl.end();
}


struct MyWindow : public Window, public Drawable{

	void onDraw(Graphics& gl){
		gl.depthTesting(1);

		material.shininess(10)();
		light();

		iso.level(0);
		iso.generate(volData, N, 1./N);
		
		gl.pushMatrix(gl.MODELVIEW);
			glEnable(GL_RESCALE_NORMAL);
			gl.translate(-1,-1,-1);
			gl.scale(2);
			iso.color(0.6,0.6,0.6);
			gl.draw(iso);
		gl.popMatrix();
		
		drawbox();
	}

	bool onFrame(){
		nav.smooth(0.8);
		nav.step(1.);
		stereo.draw(gl, cam, nav, Viewport(width(), height()), *this);
		
		if((phase += 0.0005) > 2*M_PI) phase -= 2*M_PI;

		static bool once = false;

		if(!once){ once = true;
		for(int k=0; k<N; ++k){ double z = double(k)/N * 4*M_PI;
		for(int j=0; j<N; ++j){ double y = double(j)/N * 4*M_PI;
		for(int i=0; i<N; ++i){ double x = double(i)/N * 4*M_PI;
			
			volData[k*N*N + j*N + i] 
				= cos(x * cos(phase*7)) 
				+ cos(y * cos(phase*8)) 
				+ cos(z * cos(phase*9));
		}}}
		}

		return true;
	}
};


int main (int argc, char * argv[]){

	iso.primitive(Graphics::TRIANGLES);
	
	nav.pos(0,0,-5);
	
	MyWindow win;
	win.create(Window::Dim(800,600), "Isosurface Example", 140);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));

	Window::startLoop();
	return 0;
}
