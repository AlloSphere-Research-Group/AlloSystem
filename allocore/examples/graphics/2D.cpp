/*
Allocore Example: 2D drawing

Description:
This demonstrates how to do 2D drawing.

Author:
Lance Putnam, 2/2012 (putnam.lance at gmail dot com)
*/

#include "allocore/io/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	Mesh verts;

	MyApp(){
	
		verts.primitive(Graphics::LINE_STRIP);
		verts.color(1,1,1);
		
		// Create a sine wave
		const int N = 128;
		for(int i=0; i<N; ++i){
			float f = float(i)/N;
			verts.vertex(2*f-1, sin(f*M_PI*2));
		}
	
		initWindow();
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		
		// Start with a new projection matrix
		g.pushMatrix(g.PROJECTION);
		g.loadIdentity();

		// Set up 2D orthographic projection coordinates
		// The args are left, right, bottom, top
		gluOrtho2D(-1,1,-1,1);

		g.pushMatrix(g.MODELVIEW);
		g.loadIdentity();

			g.draw(verts);

		g.popMatrix();

		// Don't forget to restore original projection matrix
		g.popMatrix(g.PROJECTION);
	}
};

int main(){ MyApp().start(); }
