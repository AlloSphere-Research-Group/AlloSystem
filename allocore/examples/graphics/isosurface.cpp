/*
Allocore Example: Isosurface

Description:
This demonstrates how to render a scalar field as an isosurface. An isosurface
is the two-dimensional surface embedded in a three-dimensional scalar field
where all values are equal to some constant.

Author:
Lance Putnam, March 2015
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
using namespace al;

class MyApp : public App {
public:

	static const int N = 32;
	float volData[N*N*N];
	Isosurface iso;
	double phase = 0;
	bool evolve = true, wireframe = false;

	MyApp(){
		nav().pullBack(6);
		nav().faceToward(Vec3f(-0.5,-1,-1));
		initWindow(Window::Dim(800,600), "Isosurface Example");
	}

	void onAnimate(double dt) override {

		if(evolve){
			if((phase += 0.0002) > 2*M_PI) phase -= 2*M_PI;
			for(int k=0; k<N; ++k){ double z = double(k)/(N-1) * 6*M_PI;
			for(int j=0; j<N; ++j){ double y = double(j)/(N-1) * 6*M_PI;
			for(int i=0; i<N; ++i){ double x = double(i)/(N-1) * 6*M_PI;

				volData[k*N*N + j*N + i]
					= cos(x * cos(phase*7))
					+ cos(y * cos(phase*8))
					+ cos(z * cos(phase*9));
			}}}

			// Set the level at which the surface is drawn
			iso.level(0);

			// Generate surface from volume data
			iso.generate(volData, N, 1./N);
		}
	}

	void onDraw(Graphics& g) override {

		auto& light = g.light();
		auto& mtrlF = g.materialFront();
		auto& mtrlB = g.materialBack();

		// Set colors of front- and back-facing surfaces
		mtrlF.ambientAndDiffuse(RGB(1));
		mtrlF.specular(RGB(0.3));
		mtrlF.shininess(20);

		mtrlB.ambientAndDiffuse(HSV(0.6,0.5,0.5));

		// Apply lighting
		Light::twoSided(true); // light both sides of surface
		light.pos(1,1,1);
		light.attenuation(1,0.2);

		g.wireframe(wireframe);

		g.matrixScope([&](){
			g.translate(-1,-1,-1);
			g.scale(2);
			g.draw(iso);
		});
	}

	void onKeyDown(const Keyboard& k) override {
		switch(k.key()){
		case 'f': wireframe^=1; break;
		case ' ': evolve^=1; break;
		}
	}
};

int main(){
	MyApp().start();
}
