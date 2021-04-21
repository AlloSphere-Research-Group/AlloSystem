/*
Allocore Example: Shape Gallery

Description:
This shows some of the shape primitives provided by AlloCore. Many of the
shapes have parameters that can be adjusted to produce a continuum of related
shapes. Moving the mouse will adjust some of these parameters.

Author(s):
Lance Putnam, 2011, 2015
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App {
public:

	static const int Nm = 9;
	Mesh meshes[Nm];
	double angle=0;
	float mx=0.5, my=0.5;
	bool wireframe = false;
	bool vertexLight = false;

    MyApp(){
		nav().pullBack(5.5);
		initWindow(Window::Dim(600,600), "Shape Gallery");
	}

	void onAnimate(double dt){

		angle += 2 * dt;
		if(angle > 360) angle -= 360;

		// Clear all the meshes
		for(int i=0; i<Nm; ++i){
			meshes[i].reset();
		}

		int S = 0;

		// Create a sphere
		addSphere(meshes[S++],
			1,				// radius
			3 + mx*16,		// slices
			2 + my*16		// stacks
		);

		// Create an icosphere
		addIcosphere(meshes[S++],
			1,				// radius
			mx*4			// subdivisions
		);

		// Create a cone/pyramid
		addCone(meshes[S++],
			1,				// radius
			Vec3f(0,0,2*my),// apex
			3 + mx*16,		// slices (number of planes)
			1				// cycles
		);

		// Create a disc/regular polygon
		addDisc(meshes[S++],
			1,				// radius
			3 + mx*16		// slices (polygon order)
		);

		// Create a prism
		addPrism(meshes[S++],
			1,				// bottom radius
			0.5,			// top radius
			2,				// height
			3 + mx*16,		// slices
			my*0.5			// twist
		);

		// Create an annulus
		addAnnulus(meshes[S++],
			my,				// inner radius
			1,				// outer radius
			3 + mx*16,		// slices
			0				// twist
		);

		// Create an open cylinder
		addCylinder(meshes[S++],
			my,				// radius
			2,				// height
			3 + mx*16		// slices
		);

		// Create a height map using a surface
		addSurface(meshes[S++],
			3 + mx*32,		// number of points along x
			3 + mx*32		// number of points along y
		);
		{
			Mesh& m = meshes[S-1];
			for(int i=0; i<m.vertices().size(); ++i){
				Mesh::Vertex& v = m.vertices()[i];
				float r = ::hypot(v.x, v.y);
				v.z = ::exp(-8*r*r);
			}
		}

		// Create a torus
		addTorus(meshes[S++],
			0.3,			// minor radius
			0.7,			// major radius
			3 + my*16,		// minor resolution
			3 + mx*16,		// major resolution
			0.5
		);


		// Scale and generate normals
		for(int i=0; i<Nm; ++i){
			meshes[i].scale(0.4);

			int Nv = meshes[i].vertices().size();
			for(int k=0; k<Nv; ++k){
				meshes[i].color(HSV(float(k)/Nv, 0.7, 1));
			}

			if(!vertexLight && meshes[i].primitive() == Graphics::TRIANGLES){
				meshes[i].decompress();
			}
			meshes[i].generateNormals();
		}
	}

	void onDraw(Graphics& g){
		g.polygonMode(wireframe ? Graphics::LINE : Graphics::FILL);
		g.light().pos(1,4,1);

		for(int i=0; i<Nm; ++i){
			g.pushMatrix();
			float x = float(i%3)/2 * 2 - 1;
			float y = float(i/3)/2 * 2 - 1;
			g.translate(x,-y,0);
			g.rotate(angle*13, 0,0,1);
			g.rotate(angle*17, 1,0,0);
			g.draw(meshes[i]);
			g.popMatrix();
		}
	}

	void onKeyDown(const Keyboard& k){
		switch(k.key()){
		case 'f': wireframe  ^=true; break;
		case 'l': vertexLight^=true; break;
		}
	}

	void onMouseMove(const Mouse& m){
		mx = float(m.x()) / window().width();
		my = float(m.y()) / window().height();
	}
};

int main(){
    MyApp().start();
}

