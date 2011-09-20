/*
Allocore Example: Many Shape Mesh

Description:
This demonstrates how a single mesh can be used to draw many different shapes.

Author:
Lance Putnam, 4/25/2011
*/

#include "allocore/al_Allocore.hpp"

using namespace al;

Graphics gl;
Mesh shapes;
Light light;
Material material;

struct MyWindow : Window{

	bool onCreate(){
		angle = 0;
		return true;
	}

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-8), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(1);
		gl.blending(0);
	
		material();
		light();

		angle += 1./8000;

		gl.pushMatrix(gl.MODELVIEW);
			//gl.translate(R*cos(i*angPos), R*sin(i*angPos), 0);
			gl.rotate(angle*113, 0,1,0);
			gl.rotate(angle* 79, 1,0,0);
			gl.draw(shapes);
		gl.popMatrix();

		return true;
	}
	
	double angle;
};

MyWindow win1;

int main(){

	for(int i=0; i<800; ++i){
		int Nv = rnd::prob(0.5)
					? (rnd::prob(0.5) ? addCube(shapes) : addDodecahedron(shapes))
					: addIcosahedron(shapes);
		
		Mat4f xfm;
		xfm.setIdentity();
		xfm.scale(Vec3f(rnd::uniform(1.,0.1), rnd::uniform(1.,0.1), rnd::uniform(1.,0.1)));
		xfm.translate(Vec3f(rnd::uniformS(8.), rnd::uniformS(8.), rnd::uniformS(8.)));
		//xfm.rotate(rnd::uniform(), rnd::uniform(), rnd::uniform());
		
		shapes.transform(xfm, shapes.vertices().size()-Nv);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			shapes.color(HSV(f*0.1+0.2,1,1));
		}
	}

	shapes.primitive(Graphics::TRIANGLES);
	shapes.decompress();
	shapes.generateNormals();


	win1.add(new StandardWindowKeyControls);

	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
