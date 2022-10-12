/*
Allocore Example: Matrix Transformations

Description:
This demonstrates how basic affine transformations, scaling, rotation, and
translation, can be applied when rendering meshes.

Author:
Lance Putnam, March 2015
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:

	Mesh mesh;

	MyApp(){

		// Add wire-frame cube to mesh
		addWireBox(mesh);

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {

		// The modelview matrix transforms each vertex before it is rendered.
		// First, we push a copy of the current modelview matrix onto a stack.
		// This allows us to save the matrix's current state and restore it at
		// a later point.
		g.pushMatrix();

		// Typically we scale, rotate, then translate. Note that we must apply
		// the transformations in the reverse order!
		g.translate(0, -0.5, 0);
		g.rotate(-45, 1,0,0);
		g.scale(0.5, 0.3, 0.1);

		// Finally, draw mesh
		g.color(HSV(0.6, 0.5, 1));
		g.draw(mesh);

		// Pop the modified matrix off the stack to restore the original matrix.
		g.popMatrix();


		// We can draw the mesh as many times as we like with different
		// transformations. Here, we build a pyramid.
		for(int i=1; i<16; ++i){

			float ds = 1./16;

			g.pushMatrix();

			g.translate(0, ds*(16-i), 0);
			g.scale(ds*i, ds*0.5, ds*i);
			g.color(HSV(0.1, 0.7, 1));
			g.draw(mesh);

			g.popMatrix();
		}
	}
};

int main(){
	MyApp().start();
}
