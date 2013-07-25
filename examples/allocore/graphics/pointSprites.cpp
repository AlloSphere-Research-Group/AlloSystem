/*
Allocore Example: Point Sprites

Description:
This demonstrates OpenGL's built-in point sprite capability. A point sprite
is simply a screen-aligned textured square.

Author:
Lance Putnam, 2/25/2011 (putnam.lance at gmail dot com)
*/

#include "allocore/al_Allocore.hpp"

using namespace al;

Graphics gl;
Texture tex(16,16, Graphics::LUMINANCE, Graphics::FLOAT);

struct MyWindow : Window{

	bool onCreate(){

		// Generate a grid of points
		const int N = 12;
		data.primitive(gl.POINTS);
	
		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			data.vertex(x,y,z);
			data.color(x*0.1+0.1, y*0.1+0.1, z*0.1+0.1, 1);
		}}}

		int Nx = tex.width();
		int Ny = tex.height();
		float * texBuf = new float[tex.numElems()];
		
		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;
			float m = 1 - al::clip(hypot(x,y));
			texBuf[j*Nx + i] = m;
		}}
		
		tex.submit(texBuf);
		delete[] texBuf;

		return true;
	}

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-4), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(false);
		gl.blending(true);
		gl.blendModeAdd();

		angle += 0.1; if(angle>360) angle -= 360;

		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		gl.pointSize(40);

		tex.bind();
			gl.rotate(angle*7, 0,1,0);
			gl.rotate(angle*3, 0,0,1);
			gl.draw(data);
		tex.unbind();
		
		glDisable(GL_POINT_SPRITE);

		return true;
	}
	
	float angle;
	Mesh data;
};

MyWindow win;

int main(){
	win.add(new StandardWindowKeyControls);
	win.create();
	MainLoop::start();
	return 0;
}
