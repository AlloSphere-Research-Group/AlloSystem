/*
Allocore Example: Fluid

Description:
This demonstrates a 3D fluid simulation

Author:
Graham Wakefield 2011
*/


#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "alloutil/al_Field3D.hpp"

using namespace al;

// create a fluid with 3 components per voxel, on a 32x32x32 grid:
Fluid<float> fluid(3, 32);

// textures to show the fluid densities & velocities
Texture densityTex, velocityTex;

Graphics gl;
Mesh mesh;
ShaderProgram shaderP;
Shader shaderV, shaderF;

static const char * vLight = AL_STRINGIFY(
uniform sampler3D tex; 
varying vec3 vel;
void main(){
	vec3 xyz = gl_Vertex.xyz / 32.; 
	vel = texture3D(tex, xyz).rgb;
	vec4 vertex1 = gl_Vertex;
	vec2 texcoord0 = vec2(gl_MultiTexCoord0);
	if (texcoord0.s > 0.) {
		vertex1 += vec4(vel, 0);
	}
	gl_Position = gl_ModelViewProjectionMatrix * vertex1;
}
);

static const char * fLight = AL_STRINGIFY(
varying vec3 vel;
void main() {
	float mag = length(vel.xyz);
	gl_FragColor = vec4(vel+0.5, mag);
}
);

struct MyWindow : public Window {

	bool onCreate(){
	
		// reconfigure textures based on arrays:
		densityTex.submit(fluid.densities.front(), true);
		velocityTex.submit(fluid.velocities.front(), true);
		
		// shader method:
		shaderV.source(vLight, Shader::VERTEX).compile();
		shaderF.source(fLight, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();
		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		// create rendering mesh of lines
		mesh.reset();
		mesh.primitive(Graphics::LINES);
		for (int x=0; x<32; x++) {
		for (int y=0; y<32; y++) {
		for (int z=0; z<32; z++) {
			// render 2 vertices at the same location
			// using texcoord as an attribute to distinguish 
			// 'start' and 'end' vertices
			mesh.texCoord(0, 0);
			mesh.vertex(x, y, z);
			mesh.texCoord(1, 1);	
			mesh.vertex(x, y, z);	
		}}}
		
		return true;
	}
	
	bool onFrame(){
		gl.clearColor(0,0,0,0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(16, 16, 64), Vec3d(16, 16, 16), Vec3d(0,1,0)));
		
		
		// add some forces:
		float t = MainLoop::now() * 0.25;
		float r = 20;
		fluid.addForce(Vec3f(12, 12, 16), Vec3f(r*cos(t), r*sin(t), 0));
		fluid.addForce(Vec3f(20, 20, 16), Vec3f(-r*cos(t*2), -r*sin(t*2), 0));
		
		// run a fluid step:
		fluid.update();
		
		// update texture data:
		densityTex.submit(fluid.densities.front());
		velocityTex.submit(fluid.velocities.front());
		
		// draw it:
		gl.blending(true);
		gl.blendModeAdd();
		gl.lineWidth(0.5);
		
		shaderP.begin();
		velocityTex.bind();
		gl.draw(mesh);
		velocityTex.unbind();
		shaderP.end();

		return true;
	}
};


MyWindow win;

int main(){

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(640, 480));

	MainLoop::start();
	return 0;
}

