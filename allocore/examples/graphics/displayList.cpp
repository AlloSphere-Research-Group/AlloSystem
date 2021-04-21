/*
Allocore Example: Display List

Description:
This demonstrates how to efficiently render static objects using a display list.

Author:
Lance Putnam, 1/2012 (putnam.lance at gmail dot com)
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_DisplayList.hpp"
#include "allocore/graphics/al_Light.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Shapes.hpp"

using namespace al;

class MyApp : public App{
public:

	Mesh verts;
	Light light;
	DisplayList dlist;

	MyApp(){
		// Create a sphere
		addSphere(verts, 0.25);
		verts.generateNormals();

		nav().pos(5,5,20);
		initWindow();
	}

	void onCreate(const ViewpointWindow& win){
		// Compile the display list
		dlist.begin();
		graphics().draw(verts);
		dlist.end();
	}

	void onDraw(Graphics& g){
		light.dir(1,1,1);
		light();

		// Call our display list multiple times
		for(int k=0; k<10; ++k){
		for(int j=0; j<10; ++j){
		for(int i=0; i<10; ++i){
			g.pushMatrix();
				g.translate(i,j,k);
				dlist.draw();
			g.popMatrix();
		}}}
	}
};

int main(){
	MyApp().start();
}
