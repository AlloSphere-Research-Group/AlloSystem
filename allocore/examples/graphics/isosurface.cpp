#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
#include "allocore/io/al_ControlNav.hpp"

using namespace al;

Graphics gl;
Light light;
Material mtrl;
Lens lens;
Nav nav(Vec3d(0,0,5));
Stereographic stereo;

const int N = 32;
float volData[N*N*N];
Isosurface iso;
double phase=0;
bool evolve = true;
bool wireframe = false;


struct MyWindow : public Window, public Drawable{

	void onDraw(Graphics& gl){
		gl.depthTesting(1);

		light.dir(1,1,1);
		light.ambient(Color(1));
		mtrl.useColorMaterial(false);
		mtrl.ambient(Color(0.1,0,0));
		mtrl.diffuse(Color(0.7,0,0));
		mtrl.specular(HSV(0.1,1,0.7));
		mtrl();		
		light();

		iso.level(0);
		iso.generate(volData, N, 1./N);

		if(wireframe)	gl.polygonMode(gl.LINE);
		else			gl.polygonMode(gl.FILL);

		gl.pushMatrix(gl.MODELVIEW);
			glEnable(GL_RESCALE_NORMAL);
			gl.translate(-1,-1,-1);
			gl.scale(2);
			gl.draw(iso);
		gl.popMatrix();
	}

	bool onFrame(){
		nav.smooth(0.8);
		nav.step(1.);
		stereo.draw(gl, lens, nav, Viewport(width(), height()), *this);

		if(evolve){
			if((phase += 0.0002) > 2*M_PI) phase -= 2*M_PI;
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

	virtual bool onKeyDown(const Keyboard& k){
		switch(k.key()){
		case 'f': wireframe^=1; return false;
		case ' ': evolve^=1; return false;
		}
		return true;
	}
};

MyWindow win;
	
int main(){
	win.create(Window::Dim(800,600), "Isosurface Example", 140);
	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControl(nav));
	Window::startLoop();
}
