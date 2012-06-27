/*
Allocore Example: Texture Shader

Description:
This demonstrates the simplest way to display a texture using a shader.

Author(s):
Lance Putnam, 5/27/2011
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"
using namespace al;

const char * vTexture = AL_STRINGIFY(
void main(){
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
);

const char * fTexture = AL_STRINGIFY(
uniform sampler2D texSampler;
void main(){
	gl_FragColor = texture2D(texSampler, gl_TexCoord[0].xy);
}
);


struct MyWindow : Window{

	MyWindow()
	:	 tex(64,64, Graphics::RGBA, Graphics::FLOAT)
	{}

	Graphics gl;
	ShaderProgram shaderP;
	Shader shaderV, shaderF;
	Texture tex;

	bool onCreate(){
		int Nx = tex.width();
		int Ny = tex.height();
		float * texBuf = new float[tex.numElems()];
		
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
		
		tex.submit(texBuf);
		delete[] texBuf;

		shaderV.source(vTexture, Shader::VERTEX).compile().printLog();
		shaderP.attach(shaderV);

		shaderF.source(fTexture, Shader::FRAGMENT).compile().printLog();
		shaderP.attach(shaderF);

		shaderP.link().printLog();
		return true;
	}

	bool onFrame(){
		gl.clearColor(0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,3), Vec3d(0,0,0), Vec3d(0,1,0)));

		shaderP.begin();
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
		shaderP.end();

		return true;
	}
};

MyWindow win1;

int main(){
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));
	MainLoop::start();
	return 0;
}
