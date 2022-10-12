/*
Allocore Example: Lighting

Description:
This example demonstrates how to use lighting within a scene. The main
components of a light are its ambient and diffuse components. The ambient
component models reflected light coming from the environment. A room will have 
more ambient light than, say, the outdoors. The diffuse component models light
scattered directly from the light source. It is also possible to add specular
highlights by using a Material object.

Author:
Lance Putnam, Dec. 2010
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App {
public:
	Vec3f lightPos;
	Mesh surface, sphere;	// Geometry to render
	double phase = 0.5;		// Animation phase

	MyApp(){

		// Create a circular wave pattern
		addSurface(surface, 64,64);
		for(int i=0; i<surface.vertices().size(); ++i){
			Mesh::Vertex& p = surface.vertices()[i];
			p.z = cos(p.mag()*4*M_PI)*0.1;
		}
		surface.color(RGB(1));

		// For all meshes we would like to light, we must generate normals.
		// This function is valid for TRIANGLES and TRIANGLE_STRIP primitives.
		surface.generateNormals();

		// Create a sphere to see the location of the light source.
		addSphere(sphere, 0.05);
		sphere.color(RGB(1));

		nav().pullBack(4);
		nav().faceToward(Vec3f(0,1,-1));
		initWindow();
	}

	void onAnimate(double dt) override {
		// Set light position
		phase += 1./1800; if(phase > 1) phase -= 1;
		float x = cos(7*phase*2*M_PI);
		float y = sin(11*phase*2*M_PI);
		float z = cos(phase*2*M_PI)*0.5 + 0.6;

		lightPos.set(x,y,z);
	}

	void onDraw(Graphics& g) override {

		// Grab a light and material from Graphics object.
		// Accessing these will activate lighting.
		auto& light = g.light();
		auto& mtrl = g.material();

		// Set up light
		light
			.pos(lightPos)
			.ambient(RGB(0))			// Ambient reflection for this light
			.diffuse(RGB(1,1,0.5))		// Light scattered directly from light
			.attenuation(1,1,0)			// Inverse distance attenuation
			//.attenuation(1,0,1)		// Inverse-squared distance attenuation
			.globalAmbient(RGB(0.1))	// Ambient reflection for all lights
		;

		// Set up material (i.e., specularity)
		mtrl
			.specular(light.diffuse()*0.2) // Specular highlight, "shine"
			.shininess(50)			// Concentration of specular component [0,128]
		;

		// Draw surface with lighting
		g.draw(surface);

		// Render sphere at light position; this will not be lit
		g.lighting(false);
		g.matrixScope([&](){
			g.translate(light.pos());
			sphere.colors()[0] = light.diffuse();
			g.draw(sphere);
		});
	}
};

int main(){
	MyApp().start();
}
