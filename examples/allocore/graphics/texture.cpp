/*
Allocore Example: Texture

Description:
This demonstrates how to create and display a texture.

Author:
Lance Putnam, 9/13/2011
*/

#include "allocore/al_Allocore.hpp"

using namespace al;

Graphics gl;
Texture tex(63,63, Graphics::RGB, Graphics::UBYTE);

struct MyWindow : Window{

	bool onCreate(){
		// default magnification filter is linear
		//tex.filterMag(Texture::NEAREST);
		
		int Nx = tex.width();
		int Ny = tex.height();

		//tex.allocate();
		//float * texBuf = tex.data<float>();
		unsigned char * texBuf = new unsigned char[Nx*Ny*4];
		
		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;

			float m = 1 - al::clip(hypot(x,y));
			float a = al::wrap(atan2(y,x)/M_2PI);
			
			Color col = HSV(a,1,m);
			
			int idx = j*Nx + i;
			int stride = tex.numComponents();
			texBuf[idx*stride + 0] = col.r * 255.;
			texBuf[idx*stride + 1] = col.g * 255.;
			texBuf[idx*stride + 2] = col.b * 255.;
			//texBuf[idx*4 + 3] = col.a;
		}}

		tex.submit(texBuf);

		return true;
	}

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,3), Vec3d(0,0,0), Vec3d(0,1,0)));

		tex.bind();

			gl.begin(gl.QUADS);
			gl.vertex(-1, -1);
			gl.vertex( 1, -1);
			gl.vertex( 1,  1);
			gl.vertex(-1,  1);
			gl.texCoord(0,0);
			gl.texCoord(1,0);
			gl.texCoord(1,1);
			gl.texCoord(0,1);
			gl.end();

		tex.unbind();

		return true;
	}
};

MyWindow win;

int main(){

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
