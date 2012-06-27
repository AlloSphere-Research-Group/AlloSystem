/*
Allocore Example: Platonics

Description:
The five Platonic solids.

Author:
Lance Putnam, 12/8/2010 (putnam.lance at gmail dot com)
*/

#include "allocore/al_Allocore.hpp"

using namespace al;

Graphics gl;
Mesh solids[5];
Light light;
Material material;

struct MyWindow : Window{

	bool onCreate(){
		angle1 = angle2 = 0;
		return true;
	}

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(1);
		gl.blending(0);
	
		material();
		light();

		angle1 += 1./3;
		angle2 += M_PI/3;
		float angPos = 2*M_PI/5;
		float R = 3;
		
		for(int i=0; i<5; ++i){
			gl.pushMatrix(gl.MODELVIEW);
				gl.translate(R*cos(i*angPos), R*sin(i*angPos), 0);
				gl.rotate(angle1, 0,1,0);
				gl.rotate(angle2, 1,0,0);
				gl.draw(solids[i]);
			gl.popMatrix();
		}

		return true;
	}
	
	double angle1, angle2;
};

MyWindow win;

int main(){

	{
		int Nv = addTetrahedron(solids[0]);

		solids[0].primitive(Graphics::TRIANGLES);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[0].color(HSV(f*0.2+0.4,1,1));
		}

		solids[0].decompress();
		solids[0].generateNormals();
	}

	{
		int Nv = addCube(solids[1]);

		solids[1].primitive(Graphics::TRIANGLES);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[1].color(HSV(f*0.1+0.2,1,1));
		}

		solids[1].decompress();
		solids[1].generateNormals();
	}

	{
		int Nv = addOctahedron(solids[2]);
		solids[2].primitive(Graphics::TRIANGLES);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[2].color(HSV(f*0.1+0.1,1,1));
		}

		solids[2].decompress();
		solids[2].generateNormals();
	}

	{
		int Nv = addDodecahedron(solids[3]);
		solids[3].primitive(Graphics::TRIANGLES);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[3].color(HSV(f*0.1,1,1));
		}

		solids[3].decompress();
		solids[3].generateNormals();
	}

	{
		int Nv = addIcosahedron(solids[4]);
		solids[4].primitive(Graphics::TRIANGLES);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			solids[4].color(HSV(f*0.1 + 0.7,1,1));
		}

		solids[4].decompress();
		solids[4].generateNormals();
	}

	win.add(new StandardWindowKeyControls);
	win.create();
	MainLoop::start();
}
