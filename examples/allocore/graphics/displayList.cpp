/*
Allocore Example: Display List

Description:
This demonstrates how to efficiently render static objects using a display list.

Author:
Lance Putnam, 1/2012 (putnam.lance at gmail dot com)
*/

#include "allocore/io/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	Mesh verts;
	Light light;
	DisplayList dlist;

	MyApp(){
		// create a sphere
		verts.primitive(Graphics::TRIANGLES);
		addSphere(verts, 0.5);
		verts.generateNormals();

		nav().pos(5,5,20);
		initWindow();
	}

	virtual void onCreate(const ViewpointWindow& win){
		// compile the display list
		dlist.begin();
		graphics().draw(verts);
		dlist.end();
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		light.dir(1,1,1);
		light();
		
		// call our display list multiple times
		for(int k=0; k<10; ++k){
		for(int j=0; j<10; ++j){
		for(int i=0; i<10; ++i){
			g.pushMatrix(g.MODELVIEW);
				g.translate(i,j,k);
				dlist.draw();
			g.popMatrix();
		}}}
	}
};

int main(){ MyApp().start(); }
