#include "allocore/al_Allocore.hpp"
#include "al_NavControl.hpp"
using namespace al;

gfx::GraphicsBackendOpenGL backend;
gfx::Light light;
gfx::Graphics gl(&backend);
Camera cam;

const int N = 32;
float volData[N*N*N];
Isosurface<float> iso;
double phase=0;

void drawbox() {
	gl.begin(gfx::LINES);
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


struct MyWindow : Window{

	bool onFrame(){
		double aspect = dimensions().w / dimensions().h;
		
		gl.viewport(0,0, dimensions().w, dimensions().h);
		gl.clear(gfx::COLOR_BUFFER_BIT | gfx::DEPTH_BUFFER_BIT);

		cam.step(1.);
		
		const Vec3d& pos = cam.pos();
		const Vec3d& ur  = cam.ur();
		const Vec3d& uu  = cam.uu();
		const Vec3d& uf  = cam.uf();
		double fovy = cam.fovy();
		double near = cam.near();
		double far  = cam.far();
		
		// apply camera transform:
		gl.matrixMode(gfx::PROJECTION);
		gl.pushMatrix();
		gl.loadMatrix(Matrix4d::perspective(fovy, aspect, near, far));

		gl.matrixMode(gfx::MODELVIEW);
		gl.pushMatrix();
		gl.loadMatrix(Matrix4d::lookAt(ur, uu, uf, pos));
		
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

		gl.depthTesting(1);
		gl.lighting(1);
		
		light();
		
		iso.level(0);
		iso.generate(volData, N, 1./N);
		gl.draw(iso);
		
		drawbox();
		return true;
	}
};


int main (int argc, char * argv[]){

	iso.primitive(gfx::TRIANGLES);
	
	MyWindow win;
	win.create(Window::Dim(800,600), "Isosurface Example", 140);

	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(&cam));

	Window::startLoop();
	return 0;
}
