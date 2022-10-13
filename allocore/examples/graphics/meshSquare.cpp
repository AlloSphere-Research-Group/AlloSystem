/*
Allocore Example: Square mesh

Description:
This demonstrates how to render a colored square using the Mesh class.

Author:
Lance Putnam, March 2015
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:

	// This mesh will contain the vertex positions and colors of our square.
	Mesh mesh;

	MyApp(){

		/* The square we want to render will have the following configuration:
		
		(-1,1,0)    (1,1,0)
		red         blue
		 o-----------o
		 |0         2|
		 |           |
		 |           |
		 |           |
		 |1         3|
		 o-----------o
		(-1,-1,0)   (1,-1,0)
		green       white
		*/

		// We will use a triangle strip primitive to create a triangle between
		// each three successive vertices.
		mesh.primitive(Graphics::TRIANGLE_STRIP);

		// First, we add the vertex positions in a zig-zag pattern.
		mesh.vertex(-1, 1, 0); // 0: top-left corner
		mesh.vertex(-1,-1, 0); // 1: bottom-left corner
		mesh.vertex( 1, 1, 0); // 2: top-right corner
		mesh.vertex( 1,-1, 0); // 3: bottom-right corner

		// Next, we add the vertex colors (in RGB color space), using the same
		// order as the positions above.
		mesh.color(1, 0, 0); // 0: red
		mesh.color(0, 1, 0); // 1: green
		mesh.color(0, 0, 1); // 2: blue
		mesh.color(1, 1, 1); // 3: white

		nav().pullBack(4);

		initWindow();
	}

	void onDraw(Graphics& g) override {

		// Here we tell the renderer to draw the mesh
		g.draw(mesh);
	}
};

int main(){
	MyApp().start();
}
