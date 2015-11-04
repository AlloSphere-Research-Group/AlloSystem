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
	Light light;			// Necessary to light objects in the scene
	Material material;		// Necessary for specular highlights
	Mesh surface, sphere;	// Geometry to render
	double phase;			// Animation phase

	MyApp(){
		phase = 0.5;

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

	void onAnimate(double dt){
		// Set light position
		phase += 1./1800; if(phase > 1) phase -= 1;
		float x = cos(7*phase*2*M_PI);
		float y = sin(11*phase*2*M_PI);
		float z = cos(phase*2*M_PI)*0.5 + 0.6;

		light.pos(x,y,z);
	}

	void onDraw(Graphics& g){

		// Render sphere at light position; this will not be lit
		g.lighting(false);
		g.pushMatrix(Graphics::MODELVIEW);
			g.translate(Vec3f(light.pos()));
			sphere.colors()[0] = light.diffuse();
			g.draw(sphere);
		g.popMatrix();

		// Set up light
		light.globalAmbient(RGB(0.1));	// Ambient reflection for all lights
		light.ambient(RGB(0));			// Ambient reflection for this light
		light.diffuse(RGB(1,1,0.5));	// Light scattered directly from light
		light.attenuation(1,1,0);		// Inverse distance attenuation
		//light.attenuation(1,0,1);		// Inverse-squared distance attenuation

		// Activate light
		light();

		// Set up material (i.e., specularity)
		material.specular(light.diffuse()*0.2); // Specular highlight, "shine"
		material.shininess(50);			// Concentration of specular component [0,128]

		// Activate material
		material();

		// Draw surface with lighting
		g.draw(surface);
	}
};

int main(){
	MyApp().start();
}
