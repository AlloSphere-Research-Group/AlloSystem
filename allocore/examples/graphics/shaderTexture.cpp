/*
Allocore Example: Texture Shader

Description:
This demonstrates the simplest way to access a texture in a shader.

Author(s):
Lance Putnam, May 2011
*/

#include "allocore/io/al_App.hpp"
using namespace al;

const char * vTexture = AL_STRINGIFY(
void main(){
	// OpenGL-provided varying that we will use in the fragment shader
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


class MyApp : public App{
public:
	MyApp()
	:	 tex(64,64, Graphics::RGBA, Graphics::FLOAT)
	{
		nav().pullBack(4);
		initWindow();
	}

	ShaderProgram shader;
	Texture tex;

	void onCreate(const ViewpointWindow& w){
		int Nx = tex.width();
		int Ny = tex.height();
		float * texBuf = new float[tex.numElems()];

		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1);
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1);
			int idx = j*Nx + i;
			texBuf[idx*4 + 0] = x;
			texBuf[idx*4 + 1] = y;
			texBuf[idx*4 + 2] = 0;
			texBuf[idx*4 + 3] = 1;
		}}

		tex.submit(texBuf);
		delete[] texBuf;

		shader.compile(vTexture, fTexture);
	}

	void onDraw(Graphics& g){
		
		shader.begin();
		tex.bind();

			g.begin(Graphics::TRIANGLE_STRIP);
				g.vertex(-1, -1);
				g.vertex( 1, -1);
				g.vertex(-1,  1);
				g.vertex( 1,  1);
				g.texCoord(0,0);
				g.texCoord(1,0);
				g.texCoord(0,1);
				g.texCoord(1,1);
			g.end();

		tex.unbind();
		shader.end();
	}
};

int main(){
	MyApp().start();
}

