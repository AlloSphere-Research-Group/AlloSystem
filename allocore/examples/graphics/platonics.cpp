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

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:
	Mesh solids[5];
	double angle1=0, angle2=0;

	MyApp(){
		int Nv;

		Nv = addTetrahedron(solids[0]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[0].color(HSV(f*0.2+0.4,1,1));
		}

		Nv = addCube(solids[1]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[1].color(HSV(f*0.1+0.2,1,1));
		}

		Nv = addOctahedron(solids[2]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[2].color(HSV(f*0.1+0.1,1,1));
		}

		Nv = addDodecahedron(solids[3]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[3].color(HSV(f*0.1,1,1));
		}

		Nv = addIcosahedron(solids[4]);
		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[4].color(HSV(f*0.1 + 0.7,1,1));
		}

		// Create face normals
		for(int i=0; i<5; ++i){
			solids[i].decompress();
			solids[i].generateNormals();
		}

		nav().pullBack(16);
		initWindow();
	}

	void onAnimate(double dt) override {
		angle1 += 1./3;
		angle2 += M_PI/3;
	}

	void onDraw(Graphics& g) override {
		g.light().pos(100, 1000, 500);

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
