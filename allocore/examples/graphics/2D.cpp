/*
Allocore Example: 2D drawing

Description:
This demonstrates how to do 2D drawing by setting up an orthographic projection
matrix.

Author:
Lance Putnam, Feb. 2012
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
			float f = float(i)/(N-1);
			verts.vertex(2*f-1, 0.5*sin(f*M_PI*2));
		}

		initWindow();
	}

	void onDraw(Graphics& g, const Viewpoint& vp){

		// Switch to the projection matrix
		g.pushMatrix(Graphics::PROJECTION);

		// Set up 2D orthographic projection coordinates
		// The args to Matrix4::ortho2D are left, right, bottom, top
		float aspect = vp.viewport().aspect(); // width divided by height
		g.loadMatrix(Matrix4f::ortho2D(-aspect,aspect, -1,1));

		// If you want units of pixels, use this instead:
		//g.loadMatrix(Matrix4f::ortho2D(0,vp.viewport().w, vp.viewport().h, 0));

		// Switch to the modelview matrix
		g.pushMatrix(Graphics::MODELVIEW);
		g.loadIdentity();

			g.draw(verts);

		g.popMatrix();

		// Don't forget to restore original projection matrix
		g.popMatrix(Graphics::PROJECTION);
	}
};

int main(){
	MyApp().start();
}
