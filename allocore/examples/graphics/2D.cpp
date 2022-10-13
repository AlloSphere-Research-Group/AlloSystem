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

		verts.lineStrip();
		verts.color(1,1,1);

		// Create a sine wave
		const int N = 128;
		for(int i=0; i<N; ++i){
			float f = float(i)/(N-1);
			verts.vertex(2*f-1, 0.5*sin(f*M_PI*2));
		}

		initWindow();
	}

	void onDraw(Graphics& g) override {

		// Push projection matrix
		g.matrixScope(Graphics::PROJECTION, [&](){

			// Set up 2D orthographic projection coordinates
			// The args to Matrix4::ortho2D are left, right, bottom, top
			const auto& vp = viewport();
			float aspect = vp.aspect(); // width divided by height
			g.loadMatrix(Matrix4f::ortho2D(-aspect,aspect, -1,1));

			// If you want units of pixels, use this instead:
			//g.loadMatrix(Matrix4f::ortho2D(0,vp.w, vp.h, 0));

			// Push modelview matrix
			g.matrixScope(Graphics::MODELVIEW, [&](){
				g.loadIdentity();
				g.draw(verts);
			});

		});
	}
};

int main(){
	MyApp().start();
}
