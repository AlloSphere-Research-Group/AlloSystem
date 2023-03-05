/*
Allocore Example: Texture Shader

Description:
This demonstrates the simplest way to access a texture in a shader.

Author(s):
Lance Putnam, May 2011
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
using namespace al;

class MyApp : public App{
public:

	ShaderProgram shader;
	Texture tex{64, 64};

	MyApp(){

		// Set texel colors
		tex.assignFromTexCoord<Color>([](float u, float v){
			return RGB(u,v,0);
		});

		// Compile the vertex and fragment shaders to display the texture
		shader.compile(R"(
			void main(){
				// Built-in varying that we will use in the fragment shader
				gl_TexCoord[0] = gl_MultiTexCoord0;

				// Set position as you normally do
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
			}
		)", R"(
			// A "sampler" is used to fetch texels from the texture
			uniform sampler2D tex;
			void main(){
				gl_FragColor = texture2D(tex, gl_TexCoord[0].xy);
			}
		)");

		nav().pullBack(4);
		initWindow();
	}

	void onDraw(Graphics& g) override {
		
		shader.scope([&](){
			tex.bind();
			auto& m = g.mesh();
			m.reset().triangleStrip();
			m.vertex(-1, -1);
			m.vertex( 1, -1);
			m.vertex(-1,  1);
			m.vertex( 1,  1);
			m.texCoord(0,0);
			m.texCoord(1,0);
			m.texCoord(0,1);
			m.texCoord(1,1);
			g.draw();
			tex.unbind();
		});
	}
};

int main(){
	MyApp().start();
}

