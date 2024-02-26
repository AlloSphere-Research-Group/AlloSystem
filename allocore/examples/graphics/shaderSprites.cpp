/*
Allocore Example: Point Sprite Shader

Description:
This demonstrates how to create point sprites using a geometry shader.

Author(s):
Lance Putnam, May 2011
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
using namespace al;

class MyApp : public App{
public:

	static const int N = 16; // grid resolution
	ShaderProgram shader;
	Texture tex{16,16, Graphics::LUMINANCE};
	Mesh geom;
	float angle = 0.;

	MyApp(){

		// Create sprite positions
		geom.points();
		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			geom.vertex(x,y,z);
			geom.color(x*0.1+0.1, y*0.1+0.1, z*0.1+0.1);
		}}}

		// Create sprite texture
		tex.assignFromTexCoord<Color>([](float u, float v){
			auto xy = toVec(u,v)*2.f - 1.f;
			auto m = xy.magSqr();
			if(m > 1.) m = 1.;
			m = 1.-m;
			return Color(m); // spherical looking
			//return Color(m*m); // Gaussian-like
			//return Color(pow(2, 1.-1./m)); // bright bump
		});

		// Build shader
		// Geometry inputs/outputs must be specified BEFORE compiling shader
		shader.setGeometryInputPrimitive(Graphics::POINTS);
		shader.setGeometryOutputPrimitive(Graphics::TRIANGLE_STRIP);
		shader.setGeometryOutputVertices(4);

		shader.version(120).compile(R"(
			// Vertex program
			void main(){
				gl_FrontColor = gl_Color;
				gl_Position = gl_Vertex;
			}
		)", R"(
			// Fragment program
			uniform sampler2D tex;
			void main(){
				gl_FragColor = texture2D(tex, gl_TexCoord[0].xy) * gl_Color;
			}
		)", R"(
			// Geometry program (runs after vertex)
			#extension GL_EXT_geometry_shader4 : enable

			uniform float spriteRadius;
			void main(){
				//screen-aligned axes
				vec3 axis1 = vec3(	gl_ModelViewMatrix[0][0],
									gl_ModelViewMatrix[1][0],
									gl_ModelViewMatrix[2][0]) * spriteRadius;

				vec3 axis2 = vec3(	gl_ModelViewMatrix[0][1],
									gl_ModelViewMatrix[1][1],
									gl_ModelViewMatrix[2][1]) * spriteRadius;

				vec4 pxy = gl_ModelViewProjectionMatrix * vec4(-axis1 - axis2, 0.5);
				vec4 pXy = gl_ModelViewProjectionMatrix * vec4( axis1 - axis2, 0.5);
				vec4 pxY = gl_ModelViewProjectionMatrix * vec4(-axis1 + axis2, 0.5);
				vec4 pXY = gl_ModelViewProjectionMatrix * vec4( axis1 + axis2, 0.5);

				for(int i = 0; i < gl_VerticesIn; ++i){
					// copy color
					gl_FrontColor = gl_FrontColorIn[i];

					vec4 p = gl_ModelViewProjectionMatrix * vec4(gl_PositionIn[i].xyz, 0.5);

					gl_TexCoord[0] = vec4(0, 0, 0, 1);
					gl_Position =  p + pxy;
					EmitVertex();

					gl_TexCoord[0] = vec4(1, 0, 0, 1);
					gl_Position =  p + pXy;
					EmitVertex();

					gl_TexCoord[0] = vec4(0, 1, 0, 1);
					gl_Position =  p + pxY;
					EmitVertex();

					gl_TexCoord[0] = vec4(1, 1, 0, 1);
					gl_Position =  p + pXY;
					EmitVertex();

					EndPrimitive();
				}

				//EndPrimitive();
				//gl_Position = gl_PositionIn[0];
			}
		)"
		);

		nav().pullBack(6);
		initWindow();
	}

	void onAnimate(double dt) override {
		angle += dt*8;
	}

	void onDraw(Graphics& g) override {

		g.blendAdd();

		shader.scope([&](auto&){
			tex.bind();
				shader.uniform("spriteRadius", 1./N);
				g.matrixScope([&](){
					g.rotate(angle, 0,1,0);
					g.draw(geom);
				});
			tex.unbind();
		});
	}
};

int main(){
	MyApp().start();
}

