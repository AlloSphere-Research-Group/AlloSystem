#include "allocore/al_Allocore.hpp"

using namespace al;

GraphicsGL gl;
Mesh mesh;
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
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-8), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(1);
		gl.blending(0);
	
		material();
		light();

		angle1 += 1./71;
		angle2 += M_PI/71;
//		float angPos = 2*M_PI/5;
//		float R = 3;

		gl.pushMatrix(gl.MODELVIEW);
			//gl.translate(R*cos(i*angPos), R*sin(i*angPos), 0);
			gl.rotate(angle1, 0,1,0);
			gl.rotate(angle2, 1,0,0);
			gl.draw(mesh);
		gl.popMatrix();

		return true;
	}
	
	double angle1, angle2;
};

/*
0  4  8 12
1  5  9 13
2  6 10 14
3  7 11 15
*/
template<class T, class Vx, class Vy, class Vz>
void scale(Mat<4,T>& m, Vx vx, Vy vy, Vz vz){
	m[ 0] *= vx;
	m[ 1] *= vx;
	m[ 2] *= vx;
	m[ 4] *= vy;
	m[ 5] *= vy;
	m[ 6] *= vy;
	m[ 8] *= vz;
	m[ 9] *= vz;
	m[10] *= vz;
}

template<class T, class Vx, class Vy, class Vz>
void translate(Mat<4,T>& m, Vx vx, Vy vy, Vz vz){
	m[12] += vx;
	m[13] += vy;
	m[14] += vz;
}


int main(){

	for(int i=0; i<800; ++i){
		int Nv = addCube(mesh);
		
		Mat4f xfm;
		xfm.setIdentity();
		scale(xfm, rnd::uniform(1.,0.1), rnd::uniform(1.,0.1), rnd::uniform(1.,0.1));
		translate(xfm, rnd::uniformS(6.), rnd::uniformS(6.), rnd::uniformS(6.));
		//rotate(xfm, rnd::uniform(), rnd::uniform(), rnd::uniform());
		//scale(xfm, 1, 2, 1);
		//translate(xfm, 1,0,0);
		
		mesh.transform(xfm, mesh.vertices().size()-Nv);

		for(int i=0; i<Nv; ++i){
			float f = float(i)/Nv;
			mesh.color(HSV(f*0.1+0.2,1,1));
		}
	}

	mesh.primitive(Graphics::TRIANGLES);
	mesh.decompress();
	mesh.generateNormals();

	MyWindow win1;

	win1.add(new StandardWindowKeyControls);

	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
