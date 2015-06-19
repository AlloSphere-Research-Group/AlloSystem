/*
Allocore Example: Front and back views

Description:
This example demonstrates how to render both front and back views from a single
navigation point.

Author:
Lance Putnam, March 2015
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:

	// A Viewpoint combines a position/orientation in world space and a viewport
	Viewpoint vpFront, vpBack;
	Mesh mesh;

	MyApp(){

		// Create some geometry to render
		for(int j=0; j<4; ++j){
			int Nv = addSphere(mesh, (j+1)*2, 20, 20);
			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				mesh.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}
		mesh.primitive(Graphics::LINES);

		// Setup screen stretching/anchoring behavior of viewpoints
		vpFront.stretch(1, 0.5).anchor(0, 0.5);
		vpBack .stretch(1, 0.5).anchor(0, 0.0);

		// Set parent transform of viewpoints to default Nav object
		vpFront.parentTransform(nav());
		vpBack .parentTransform(nav());

		// Rotate back viewpoint 180 deg around up vector
		vpBack.transform().quat().fromAxisAngle(M_PI, 0,1,0);

		// Initialize a window
		initWindow();

		// Clear default viewpoint from window
		window().viewpoints().clear();

		// Add our custom viewpoints to window
		window().add(vpFront).add(vpBack);
	}

	void onDraw(Graphics& g){
		g.draw(mesh);
	}
};

int main(){
	MyApp().start();
}

