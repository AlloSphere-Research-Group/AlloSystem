#include <math.h>
#include "io/al_WindowGL.hpp"
#include "protocol/al_Graphics.hpp"
#include "protocol/al_GraphicsBackendOpenGL.hpp"
#include "graphics/al_Isosurface.hpp"
#include "graphics/al_Light.hpp"
#include "spatial/al_Camera.hpp"


using namespace al;

gfx::GraphicsBackendOpenGL backend;
gfx::Light light;
gfx::Graphics gl(&backend);
static Camera cam;

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


class NavControls {
public:
	NavControls(double turnRate = 1, double slide = 0.7) 
	: mSlide(slide) {}
	
	void moveX(double v) { mMove[0] = v; }
	void moveY(double v) { mMove[1] = v; }
	void moveZ(double v) { mMove[2] = v; }
	void turnX(double v) { mTurn[0] = v; }
	void turnY(double v) { mTurn[1] = v; }
	void turnZ(double v) { mTurn[2] = v; }
	void halt() { mMove.set(0); mTurn.set(0); }
	
	void update(al_sec dt, Camera &cam) {
		double amt = 1.-mSlide;	// TODO: adjust for dt
		mMove1.lerp(mMove, amt);
		mTurn1.lerp(mTurn, amt);
		cam.vel().quat().set(Quatd::fromEuler(mTurn1[1], mTurn1[0], mTurn1[2]));
		cam.vel().vec().set(mMove1);
	}
protected:
	Vec3d mMove, mMove1;
	Vec3d mTurn, mTurn1;
	double mSlide;
};

static NavControls navcontrols;


struct MyWindow : WindowGL{
	
	void onKeyDown(const Keyboard& k){	 	
	
		static double a = 0.3;		// rotational speed: degrees per update
		static double v = 0.05;		// speed: units per update
		
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			//case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
				
			case '`':
				navcontrols.halt();
				cam.halt(); 
				cam.home();
				break;					
			case 'w':
				navcontrols.turnX(-a);
				break;
			case 'x':
				navcontrols.turnX(a);
				break;
			case Key::Right:
				navcontrols.turnY(a);
				break;
			case Key::Left:
				navcontrols.turnY(-a);
				break;
			case 'a':
				navcontrols.turnZ(a);
				break;
			case 'd':
				navcontrols.turnZ(-a);
				break;
			case ',':
				navcontrols.moveX(-v);
				break;
			case '.':
				navcontrols.moveX(v);
				break;
			case '\'':
				navcontrols.moveY(v);
				break;
			case '/':
				navcontrols.moveY(-v);
				break;
			case Key::Up:
				navcontrols.moveZ(v);
				break;
			case Key::Down:
				navcontrols.moveZ(-v);
				break;
				
			default:
				printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
				break;
		}
	}
	void onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case 'w':
			case 'x':
				navcontrols.turnX(0);
				break;
			case Key::Right:
			case Key::Left:
				navcontrols.turnY(0);
				break;
			case 'a':
			case 'd':
				navcontrols.turnZ(0);
				break;
			case ',':
			case '.':
				navcontrols.moveX(0);
				break;
			case '\'':
			case '/':
				navcontrols.moveY(0);
				break;
			case Key::Up:
			case Key::Down:
				navcontrols.moveZ(0);
				break;
			default:
				break;
		}
	}

	void onFrame(){
		double aspect = dimensions().w / dimensions().h;
		
		gl.viewport(0,0, dimensions().w, dimensions().h);
		gl.clear(gfx::AttributeBit::ColorBuffer | gfx::AttributeBit::DepthBuffer);
		
		navcontrols.update(1, cam);
		cam.step(1.);
		
		const Vec3d& pos = cam.pos();
		const Vec3d& ux  = cam.ux();
		const Vec3d& uy  = cam.uy();
		const Vec3d& uz  = cam.uz();
		double fovy = cam.fovy();
		double near = cam.near();
		double far = cam.far();
		
		// apply camera transform:
		gl.matrixMode(gfx::PROJECTION);
		gl.pushMatrix();
		gl.loadMatrix(Matrix4d::perspective(fovy, aspect, near, far));

		gl.matrixMode(gfx::MODELVIEW);
		gl.pushMatrix();
		gl.loadMatrix(Matrix4d::lookAt(ux, uy, uz, pos));

		
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
		
		light();
		
		gfx::State state;
		state.lighting_enable = true;
		state.depth_enable = true;
		gl.pushState(state);
		
		iso.level(0);
		iso.generate(volData, N, 1./N);
		gl.draw(iso);
		
		drawbox();
		
		gl.popState();
		
	}

};


int main (int argc, char * argv[]){

	iso.primitive(gfx::TRIANGLES);
	
	MyWindow win;
	win.create(WindowGL::Dim(800,600), "Isosurface Example", 140);

	WindowGL::startLoop();
	return 0;
}
