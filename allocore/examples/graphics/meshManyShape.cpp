/*
Allocore Example: Many Shape Mesh

Description:
This demonstrates how a single mesh can be used to draw many different shapes.

Author:
Lance Putnam, April 2011
*/

#include "allocore/al_Allocore.hpp"
using namespace al;


class MyApp : public App {
public:

	Mesh shapes;
	Light light;
	Material material;

	MyApp(){

		for(int i=0; i<800; ++i){
			int Nv = rnd::prob(0.5)
						? (rnd::prob(0.5) ? addCube(shapes) : addDodecahedron(shapes))
						: addIcosahedron(shapes);

			// Scale and translate the newly added shape
			Mat4f xfm;
			xfm.setIdentity();
			xfm.scale(Vec3f(rnd::uniform(1.,0.1), rnd::uniform(1.,0.1), rnd::uniform(1.,0.1)));
			xfm.translate(Vec3f(rnd::uniformS(8.), rnd::uniformS(8.), rnd::uniformS(8.)));
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

		nav().pullBack(24);
		initWindow();
	}

	void onDraw(Graphics& g){
		material();
		light();
		g.draw(shapes);
	}

};

int main(){
	MyApp().start();
}
