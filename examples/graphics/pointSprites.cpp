#include "allocore/al_Allocore.hpp"

using namespace al;

GraphicsGL gl;
Texture tex(gl, 128,128, Texture::RGBA, Texture::FLOAT32);

struct MyWindow : Window{

	bool onCreate(){
	
		const int N = 12;
	
		data.primitive(gl.POINTS);
	
		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			data.vertex(x,y,z);
		}}}

		//tex.wrap(Texture::CLAMP);
		tex.filter(Texture::NEAREST);
		tex.allocate();
		int Nx = tex.width();
		int Ny = tex.height();
		float * texBuf = tex.data<float>();
		
		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;
			
			float m = 1 - al::clip(hypot(x,y));
			float a = al::wrap(atan2(y,x)/M_2PI);

			Color col = HSV(a,1,1);
			
			int idx = j*Nx + i;
			texBuf[idx*4 + 0] = col.r;
			texBuf[idx*4 + 1] = col.g;
			texBuf[idx*4 + 2] = col.b;
			texBuf[idx*4 + 3] = m;
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
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-4), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(0);
		gl.blending(1, gl.SRC_ALPHA, gl.ONE);

		++angleY;

		gl.pushMatrix();
		
			glColor4f(1,1,1,0.3);
			glEnable(GL_POINT_SPRITE);
			glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
			glPointSize(50);

			tex.bind();
		
			gl.rotate(angleY, 0,1,0);
			gl.draw(data);
			
			tex.unbind();
			
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
