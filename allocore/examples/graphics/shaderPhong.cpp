/*
Allocore Example: Phong Lighting Shader

Description:
This demonstrates the Phong lighting model. Phong lighting is computed per-pixel
and therefore produces smoother results than Gourad (per-vertex) lighting.

Author(s):
Lance Putnam, May 2011
*/

#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shader.hpp"
using namespace al;

class MyApp : public App {
public:
	ShaderProgram shader;
	Mesh shape;
	double angle = 0.;

	MyApp(){
		addTorus(shape);
		shape.generateNormals();

		shader.compile(
		R"(
			varying vec3 normal, lightDir, eyeDir;
			void main(){
				// Normal in eye space
				normal = gl_NormalMatrix * gl_Normal;

				// Vertex in eye space
				vec3 V = (gl_ModelViewMatrix * gl_Vertex).xyz;

				// Unit vector from vertex to eye at (0,0,0)
				eyeDir = normalize(-V);

				// First compute position of light source by dividing by w
				lightDir = gl_LightSource[0].position.xyz / (max(gl_LightSource[0].position.w, 1e-16));

				// Direction is unit vector from vertex to light
				lightDir = normalize(lightDir - V);

				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
			}
		)",
		R"(
			varying vec3 normal, lightDir, eyeDir;
			void main(){
				// Initialize color to sum of ambient components
				vec4 color = gl_FrontLightModelProduct.sceneColor + gl_FrontLightProduct[0].ambient;

				vec3 N = normalize(normal); // since length will change over fragment
				vec3 L = lightDir;

				float lambertTerm = dot(N,L);

				// Does the light hit the front of the surface?
				if(lambertTerm > 0.){

					// Add in diffuse component
					color += gl_FrontLightProduct[0].diffuse * lambertTerm;

					// Add in specular component
					float spec = pow( max(dot(reflect(-L, N), eyeDir), 0.),
									 0.3*gl_FrontMaterial.shininess + 1e-20);
					color += gl_FrontLightProduct[0].specular * spec;
				}
				gl_FragColor = color;
			}
		)"
		);

		nav().pullBack(4);
		initWindow();
	}

	void onAnimate(double dt) override {
		angle += dt*10;
		if(angle >= 360) angle -= 360;
	}

	void onDraw(Graphics& g) override {

		// Set lighting properties
		g.material()
			.ambientAndDiffuse(HSV(0,0.8))
			.specular(RGB(0.4))
			.shininess(16)
		;
		g.light().dir(1,1,1);

		// Render
		shader.scope([&](){
			g.pushMatrix();
			g.rotate(angle  , 1,0,0);
			g.rotate(angle*8, 0,1,0);
			g.draw(shape);
			g.popMatrix();
		});
	}

	void onKeyDown(const Keyboard& k) override {
		if(k.key('l')) shader.toggleActive();
	}

};

int main(){
	MyApp().start();
}
