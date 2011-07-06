#include "allocore/al_Allocore.hpp"

using namespace al;

GraphicsGL gl;
Texture tex(gl, 64,64, Texture::RGBA, Texture::FLOAT32);

struct MyWindow : Window{

	bool onCreate(){
		tex.filter(Texture::NEAREST);
		tex.allocate();
		int Nx = tex.width();
		int Ny = tex.height();
		float * texBuf = tex.data<float>();
		
		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;

			float m = 1 - al::clip(hypot(x,y));
			float a = al::wrap(atan2(y,x)/M_2PI);
			
			Color col = HSV(a,1,m);
			
			int idx = j*Nx + i;
			texBuf[idx*4 + 0] = col.r;
			texBuf[idx*4 + 1] = col.g;
			texBuf[idx*4 + 2] = col.b;
			texBuf[idx*4 + 3] = col.a;
		}}

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

int main(){
	MyWindow win;

	win.add(new StandardWindowKeyControls);
	win.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
