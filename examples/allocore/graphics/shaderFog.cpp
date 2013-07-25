/*
Allocore Example: Fog Shader

Description:
Example of per-vertex exponential fog with variable curvature.

Author(s):
Lance Putnam, 9/2011, putnam.lance@gmail.com
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"

using namespace al;

static const char * fogVert = AL_STRINGIFY(
	/* 'fogCurve' determines the distribution of fog between the near and far planes.
	Positive values give more dense fog while negative values give less dense
	fog. A value of	zero results in a linear distribution. */
	uniform float fogCurve;

	/* The fog amount in [0,1] passed to the fragment shader. */
	varying float fogFactor;

	void main(){
		gl_Position = ftransform();
		gl_FrontColor = gl_Color;

		float z = gl_Position.z;
		//float z = gl_FragCoord.z / gl_FragCoord.w; /* per-frament fog would use this */
		fogFactor = (z - gl_Fog.start) * gl_Fog.scale;
		fogFactor = clamp(fogFactor, 0., 1.);
		if(fogCurve != 0.){
			fogFactor = (1. - exp(-fogCurve*fogFactor))/(1. - exp(-fogCurve));
		}
	}
);

static const char * fogFrag = AL_STRINGIFY(
	varying float fogFactor;

	void main(){
		gl_FragColor = mix(gl_Color, gl_Fog.color, fogFactor);
	}
);

struct MyWindow : Window{

	ShaderProgram shaderP;
	Shader shaderV, shaderF;
	Mesh geom;
	Graphics gl;
	float phase;
	float nearClip, farClip;


	MyWindow()
	:	phase(0), nearClip(0.1), farClip(20)
	{}


	bool onCreate(){
		shaderV.source(fogVert, Shader::VERTEX).compile();
		shaderF.source(fogFrag, Shader::FRAGMENT).compile();
		shaderP.attach(shaderF).attach(shaderV);
		shaderP.link();

		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		geom.primitive(gl.TRIANGLES);
		geom.color(Color(0));
		Mat4f xfm;

		for(int i=0; i<200; ++i){
			xfm.setIdentity();
			xfm.scale(Vec3f(0.1, 0.1, 1));
			xfm.translate(Vec3f(rnd::uniformS(4.), rnd::uniformS(4.), rnd::uniform(farClip, nearClip)));

			int Nv = addCube(geom);
			geom.transform(xfm, geom.vertices().size()-Nv);
		}

		return true;
	}


	bool onFrame(){

		// Update model
		phase += 0.00017; if(phase>=1) --phase;

		// The fog color and background color will typically be the same
		Color fogCol(HSV(0.1, 0.5, 1));

		// Set up scene
		gl.depthTesting(true);
		gl.clearColor(fogCol);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.viewport(0,0, width(), height());
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), nearClip, farClip));
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-3), Vec3d(0,0,0), Vec3d(0,1,0)));

		// Render
		shaderP.begin();
			shaderP.uniform("fogCurve", 4*sin(8*phase*6.2832));
			gl.fog(farClip, nearClip+2, fogCol);
			gl.draw(geom);
		shaderP.end();

		return true;
	}
};


int main(){
	MyWindow w;
	w.add(new StandardWindowKeyControls);
	w.create(Window::Dim(800, 600));
	MainLoop::start();
}
