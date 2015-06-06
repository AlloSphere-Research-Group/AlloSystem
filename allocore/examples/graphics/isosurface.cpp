/*
Allocore Example: Isosurface

Description:
This demonstrates how to render a scalar field as an isosurface.

Author:
Lance Putnam, March 2015
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
using namespace al;

class MyApp : public App {
public:

	static const int N = 32;
	float volData[N*N*N];
	Isosurface iso;
	double phase;
	bool evolve;
	bool wireframe;

	Light light;
	Material mtrl;

	MyApp(){
		phase = 0;
		evolve = true;
		wireframe = false;

		nav().pos(0,0,5);
		initWindow(Window::Dim(800,600), "Isosurface Example");
	}

	void onAnimate(double dt){

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

			iso.level(0);
			iso.generate(volData, N, 1./N);
		}
	}

	void onDraw(Graphics& g){

		light.dir(1,1,1);
		light.ambient(RGB(1));
		mtrl.useColorMaterial(false);
		mtrl.ambient(HSV(0.7, 1, 0.1));
		mtrl.diffuse(HSV(0.3, 1, 0.7));
		mtrl.specular(HSV(0.1, 1, 0.7));
		mtrl();
		light();

		g.polygonMode(wireframe ? Graphics::LINE : Graphics::FILL);

		g.pushMatrix();
			glEnable(GL_RESCALE_NORMAL);
			g.translate(-1,-1,-1);
			g.scale(2);
			g.draw(iso);
		g.popMatrix();
	}

	void onKeyDown(const Keyboard& k){
		switch(k.key()){
		case 'f': wireframe^=1; break;
		case ' ': evolve^=1; break;
		}
	}
};

int main(){
	MyApp().start();
}
