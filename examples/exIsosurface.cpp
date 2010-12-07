#include "allocore/al_Allocore.hpp"
#include "al_NavControl.hpp"
using namespace al;

Graphics gl;
Light light;
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
		gl.lighting(1);
		
		light();
		
		iso.level(0);
		iso.generate(volData, N, 1./N);
		gl.draw(iso);
		
		drawbox();
	}

	bool onFrame(){
		nav.smooth(0.8);
		nav.step(1.);
		stereo.draw(gl, cam, nav, Viewport(width(), height()), *this);
		
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
		}}}

		return true;
	}
};


int main (int argc, char * argv[]){

	iso.primitive(Graphics::TRIANGLES);
	
	MyWindow win;
	win.create(Window::Dim(800,600), "Isosurface Example", 140);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(&nav));

	Pose p;
	Vec3d v1, v2;
	p.pos(v1 - v2);

	Window::startLoop();
	return 0;
}
