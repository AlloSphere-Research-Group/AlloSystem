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
	Texture tex{16,16, Graphics::LUMINANCE, Graphics::FLOAT};
	Mesh geom;
	float angle = 0;

	MyApp(){

		// Create sprite positions
		geom.primitive(Graphics::POINTS);
		for(int k=0; k<N; ++k){ float z = float(k)/(N-1)*2-1;
		for(int j=0; j<N; ++j){ float y = float(j)/(N-1)*2-1;
		for(int i=0; i<N; ++i){ float x = float(i)/(N-1)*2-1;
			geom.vertex(x,y,z);
			geom.color(x*0.1+0.1, y*0.1+0.1, z*0.1+0.1);
		}}}

		// Create sprite texture
		int Nx = tex.width();
		int Ny = tex.height();
		float * pix = tex.data<float>();

		for(int j=0; j<Ny; ++j){ float y = float(j)/(Ny-1)*2-1;
		for(int i=0; i<Nx; ++i){ float x = float(i)/(Nx-1)*2-1;
			float m = 1-al::clip(x*x + y*y);
			pix[j*Nx + i] = m; // spherical looking
			//pix[j*Nx + i] = m*m; // Gaussian-like
			//pix[j*Nx + i] = pow(2, 1.-1./m); // bright bump
		}}

		nav().pullBack(6);
		initWindow();
	}

	void onCreate(const ViewpointWindow& w){

		// Geometry inputs/outputs must be specified BEFORE compiling shader
		shader.setGeometryInputPrimitive(Graphics::POINTS);
		shader.setGeometryOutputPrimitive(Graphics::TRIANGLE_STRIP);
		shader.setGeometryOutputVertices(4);

		// Compile vertex, fragment, and geometry shaders
		shader.compile(
		R"(
			void main(){
				gl_FrontColor = gl_Color;
				gl_Position = gl_Vertex;
			}
		)", R"(
			uniform sampler2D texSampler0;
			void main(){
				gl_FragColor = texture2D(texSampler0, gl_TexCoord[0].xy) * gl_Color;
			}
		)", R"(
			#version 120
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
				gl_Position = gl_PositionIn[0];
			}
		)"
		);
	}

	void onAnimate(double dt){
		angle += dt*8;
	}

	void onDraw(Graphics& g){

		g.blendAdd();

		shader.begin();
		tex.bind();
			shader.uniform("spriteRadius", 1./N);
			g.pushMatrix();
			g.rotate(angle, 0,1,0);
			g.draw(geom);
			g.popMatrix();
		tex.unbind();
		shader.end();
	}
};

int main(){
	MyApp().start();
}

