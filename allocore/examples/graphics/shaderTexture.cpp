/*
Allocore Example: Texture Shader

Description:
This demonstrates the simplest way to access a texture in a shader.

Author(s):
Lance Putnam, May 2011
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:

	ShaderProgram shader;
	Texture tex{64, 64};

	MyApp(){

		// Set texture pixels
		int Nx = tex.width();
		int Ny = tex.height();
		Colori * pix = tex.data<Colori>();

		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1);
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1);
			int idx = j*Nx + i;
			pix[idx] = RGB(x,y,0);
		}}

		// Compile the vertex and fragment shaders to display the texture
		shader.compile(
		R"(
			void main(){
				// Built-in varying that we will use in the fragment shader
				gl_TexCoord[0] = gl_MultiTexCoord0;

				// Set position as you normally do
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
			}
		)", R"(
			// A "sampler" is used to fetch texels from the texture
			uniform sampler2D texSampler;
			void main(){
				gl_FragColor = texture2D(texSampler, gl_TexCoord[0].xy);
			}
		)"
		);

		nav().pullBack(4);
		initWindow();
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

