/*
Allocore Example: Lévy flight

Description:
A Lévy flight is a random walk where the step size is determined by a function
that is heavy-tailed. This example uses a Cauchy distribution.

Author:
Lance Putnam, 9/2011
*/

#include "allocore/io/al_App.hpp"
#include "allocore/math/al_Functions.hpp"
using namespace al;

class MyApp : public App{
public:

	RingBuffer<Vec3f> A{8000};
	Mesh vert;

	MyApp(){
		nav().pullBack(4);
		initWindow();
		window().displayMode(window().displayMode() | Window::MULTISAMPLE);
	}

	void onAnimate(double dt) override {

		for(int i=0; i<4; ++i){
			auto p = rnd::ball<Vec3f>();

			float mm= p.magSqr();
			float l = 0.04;				// spread of steps; lower is more flighty
			float v = l/(mm+l*l) * 0.1;	// map uniform to Cauchy distribution

			p = p.normalized(v);
			p += A.newest();
			A.write(p);
		}

		vert.reset().lineStrip();
		for(int i=0; i<A.fill(); ++i){
			float f = float(i)/A.size();
			vert.vertex(A.read(i));

			auto dr = A.read(i+1) - A.read(i-1);

			vert.color(HSV((1-f)*0.2, al::clip(dr.mag()*4 + 0.2), 1-f));
		}
	}

	void onDraw(Graphics& g) override {
		g.draw(vert);
	}
};


int main(){
	MyApp().start();
}
