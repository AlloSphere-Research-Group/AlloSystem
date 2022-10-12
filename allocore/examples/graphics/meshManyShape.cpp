/*
Allocore Example: Many Shape Mesh

Description:
This demonstrates how a single mesh can be used to draw many different shapes.

Author:
Lance Putnam, April 2011
*/

#include "allocore/io/al_App.hpp"
using namespace al;


class MyApp : public App {
public:

	Mesh shapes;

	MyApp(){

		for(int i=0; i<100; ++i){
			int Nv = rnd::prob(0.5)
						? (rnd::prob(0.5) ? addCube(shapes) : addDodecahedron(shapes))
						: addIcosahedron(shapes);

			// Scale and translate the newly added shape
			Mat4f xfm(1);
			xfm.translate(rnd::ball<Vec3f>());
			xfm.scale(0.05f + 0.05f*rnd::cube<Vec3f>());
			shapes.transform(xfm, shapes.vertices().size()-Nv);

			// Color newly added vertices
			for(int i=0; i<Nv; ++i){
				float f = float(i)/Nv;
				shapes.color(HSV(f*0.1+0.2,1,1));
			}
		}

		// Convert to non-indexed triangles to get flat shading
		shapes.decompress();
		shapes.generateNormals();

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {
		g.light();
		g.material();
		g.draw(shapes);
	}

};

int main(){
	MyApp().start();
}
