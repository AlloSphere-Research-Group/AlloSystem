/*
Allocore Example: Fog Shader

Description:
Example of per-vertex exponential fog with variable curvature.

Author(s):
Lance Putnam, Sept. 2011
*/

#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App {
public:

	ShaderProgram shader;
	Mesh geom;
	float phase;

	MyApp(){
		phase = 0;
		background(HSV(0.1, 0.5, 1));
		lens().near(0.1).far(20);

		// Create some geometry
		geom.color(RGB(0));
		Mat4f xfm;

		for(int i=0; i<200; ++i){
			xfm.setIdentity();
			xfm.scale(Vec3f(0.05, 0.05, 1));

			Vec3f t(
				rnd::uniformS(4.),
				rnd::uniformS(4.),
				rnd::uniform(-lens().far(), -lens().near())
			);
			xfm.translate(t);

			int Nv = addWireBox(geom);
			geom.transform(xfm, geom.vertices().size()-Nv);
		}

		// Specify the shader program
		shader.compile(
		R"(
			/* 'fogCurve' determines the distribution of fog between the near and far planes.
			Positive values give more dense fog while negative values give less dense
			fog. A value of	zero results in a linear distribution. */
			uniform float fogCurve;

			/* The fog amount in [0,1] passed to the fragment shader. */
			varying float fogFactor;

			void main(){
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
				gl_FrontColor = gl_Color;

				float z = gl_Position.z;
				//float z = gl_FragCoord.z / gl_FragCoord.w; /* per-frament fog would use this */
				fogFactor = (z - gl_Fog.start) * gl_Fog.scale;
				fogFactor = clamp(fogFactor, 0., 1.);
				if(fogCurve != 0.){
					fogFactor = (1. - exp(-fogCurve*fogFactor))/(1. - exp(-fogCurve));
				}
			}
		)",
		R"(
			varying float fogFactor;

			void main(){
				gl_FragColor = mix(gl_Color, gl_Fog.color, fogFactor);
			}
		)"
		);

		initWindow();
	}

	void onAnimate(double dt){
		phase += 0.00017; if(phase>=1) --phase;
	}

	void onDraw(Graphics& g){

		// Activate fog;
		// the fog and background color will typically be the same.
		g.fog(lens().far(), lens().near()+2, background());

		// Render
		shader.begin();
			shader.uniform("fogCurve", 4*cos(8*phase*6.2832));
			g.draw(geom);
		shader.end();
	}
};

int main(){
	MyApp().start();
}
