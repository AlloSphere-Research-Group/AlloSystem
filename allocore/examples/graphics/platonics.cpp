/*
Allocore Example: Platonic Solids

Description:
This shows how to draw the five Platonic solids. A Platonic solid has
congruent faces of regular polygons with the same number of faces meeting at
each vertex. The Platonic solids are

	Tetrahedron (4 triangles)
	Cube or Hexahedron (6 squares)
	Octahedron (8 triangles)
	Dodecahedron (12 pentagons)
	Icosahedron (20 triangles)

Author:
Lance Putnam, 12/8/2010 (putnam.lance at gmail dot com)
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

class MyApp : public App{
public:
	Mesh solids[5];
	Light light;
	Material material;
	double angle1, angle2;

	MyApp(){
		angle1 = angle2 = 0;
		int Nv;

		Nv = addTetrahedron(solids[0]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[0].color(HSV(f*0.2+0.4,1,1));
		}
		solids[0].decompress();
		solids[0].generateNormals();

		Nv = addCube(solids[1]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[1].color(HSV(f*0.1+0.2,1,1));
		}
		solids[1].decompress();
		solids[1].generateNormals();

		Nv = addOctahedron(solids[2]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[2].color(HSV(f*0.1+0.1,1,1));
		}
		solids[2].decompress();
		solids[2].generateNormals();

		Nv = addDodecahedron(solids[3]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[3].color(HSV(f*0.1,1,1));
		}
		solids[3].decompress();
		solids[3].generateNormals();


		Nv = addIcosahedron(solids[4]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[4].color(HSV(f*0.1 + 0.7,1,1));
		}
		solids[4].decompress();
		solids[4].generateNormals();

		nav().pos(0,0,16);
		initWindow();
	}

	void onAnimate(double dt){
		angle1 += 1./3;
		angle2 += M_PI/3;
	}

	void onDraw(Graphics& g){
		material();
		light();

		float angPos = 2*M_PI/5;
		float R = 3;

		for(int i=0; i<5; ++i){
			g.pushMatrix();
				g.translate(R*cos(i*angPos), R*sin(i*angPos), 0);
				g.rotate(angle1, 0,1,0);
				g.rotate(angle2, 1,0,0);
				g.draw(solids[i]);
			g.popMatrix();
		}
	}
};

int main(){
	MyApp().start();
}
