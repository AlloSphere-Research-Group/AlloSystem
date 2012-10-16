/*
Allocore Example: Lévy flight

Description:
A Lévy flight is a random walk where the step size is determined by a function
that is heavy-tailed. This example uses a Cauchy distribution.

Author:
Lance Putnam, 9/2011
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:

	RingBuffer<Vec3f> A;
	Mesh vert;

	MyApp(): A(8000)
	{
		nav().pos(0,0,4);
		initWindow();
		window().displayMode(window().displayMode() | Window::MULTISAMPLE);
	}
	
	virtual void onAnimate(double dt){
	
		for(int i=0; i<4; ++i){
			Vec3f p;
			rnd::ball<3>(p.elems());

			float r = p.magSqr();
			float l = 0.04;				// spread of steps; lower is more flighty
			float v = l/(r+l*l) * 0.1;	// map uniform to Cauchy distribution

			p = p.normalized() * v;
			p += A.newest();
			A.write(p);
		}
		
		vert.primitive(Graphics::LINE_STRIP);
		vert.reset();
		for(int i=0; i<A.fill(); ++i){
			float f = float(i)/A.size();
			vert.vertex(A.read(i));
			
			Vec3f dr = A.read(i+1) - A.read(i-1);
			
			vert.color(HSV((1-f)*0.2, al::clip(dr.mag()*4 + 0.2), 1-f));
		}
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		g.draw(vert);
	}
};


int main(){
	MyApp().start();
}
