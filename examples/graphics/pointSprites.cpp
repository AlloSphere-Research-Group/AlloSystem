#include "allocore/al_Allocore.hpp"

using namespace al;

GraphicsGL gl;
Texture tex(gl, 256,256, Texture::RGBA, Texture::FLOAT32);

struct MyWindow : Window{

	bool onCreate(){
	
		const int N = 16;
	
		data.primitive(gl.POINTS);
	
		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			data.vertex(x,y,z);
		}}}
		
		tex.allocate();
		int Nx = tex.width();
		int Ny = tex.height();
		float * texBuf = tex.data<float>();
		
		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;
			int idx = j*Nx + i;
			
			Color col = HSV(0,1,1);
			
			texBuf[idx*4 + 0] = col.r;
			texBuf[idx*4 + 1] = col.g;
			texBuf[idx*4 + 2] = col.b;
			texBuf[idx*4 + 3] = col.a;
		}}		
		
		//gl.textureCreate(&tex);
		gl.textureSubmit(&tex);

		return true;
	}

	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-3), Vec3d(0,0,0), Vec3d(0,1,0)));

		++angleY;

		gl.pushMatrix();
		
			glEnable(GL_POINT_SPRITE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
			glPointSize(30);
			
			gl.textureBind(&tex, 0);
		
			gl.rotate(angleY, 0,1,0);
			gl.draw(data);
			
			gl.textureUnbind(&tex, 0);
			
			glDisable(GL_POINT_SPRITE);
		gl.popMatrix();

		return true;
	}
	
	float angleY;
	Mesh data;
};

int main(){

	MyWindow win1;

	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
