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
		addSphere(mesh);
		mesh.color(RGB(1));
		mesh.generateNormals();

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
		g.light();
		for(int i=0; i<7; ++i){
			g.matrixScope([&](){
				float frac = float(i)/7;
				g.translate(4*sin(frac*2*M_PI), 0, 4*cos(frac*2*M_PI));
				mesh.colors()[0] = HSV(frac,1,1);
				g.draw(mesh);
			});
		}
	}
};

int main(){
	MyApp().start();
}

