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

// create a fluid on a 32x32x32 grid:
Fluid3D<float> fluid(32);

// create an intensity field (on a 32x32x32 grid) 
// to be driven around:
Field3D<float> intensities(3, 32);

// textures to show the fluid densities & velocities
Texture intensityTex, velocityTex;

Graphics gl;
Mesh mesh;
ShaderProgram shaderP;
Shader shaderV, shaderF;
rnd::Random<> rng;

static const char * srcV = AL_STRINGIFY(
uniform sampler3D velocityTex; 
uniform sampler3D intensityTex; 
varying vec3 velocity;
varying vec3 intensity;
void main(){
	vec3 xyz = gl_Vertex.xyz / 32.; 
	velocity = texture3D(velocityTex, xyz).rgb;
	// Array components = 2 defaults to GL_LUMINANCE_ALPHA:
	intensity = texture3D(intensityTex, xyz).rgb;
	vec4 vertex1 = gl_Vertex;
	vec2 texcoord0 = vec2(gl_MultiTexCoord0);
	if (texcoord0.s > 0.) {
		// displace by length:
		vertex1 += vec4(velocity, 0);
	} else {
		// displace with width:
		float displace = (texcoord0.t - 0.5);
		vec3 axis = cross(normalize(velocity), vec3(0, 0, -1));
		vertex1 += vec4(displace * axis, 0);
	}

	gl_Position = gl_ModelViewProjectionMatrix * vertex1;
}
);

static const char * srcF = AL_STRINGIFY(
varying vec3 velocity;
varying vec3 intensity;
void main() {
	float mag = 0.1+length(velocity);
	gl_FragColor = vec4(0.1+intensity, mag);
}
);

struct MyWindow : public Window {

	bool onCreate(){
	
		// reconfigure textures based on arrays:
		intensityTex.submit(intensities.front(), true);
		velocityTex.submit(fluid.velocities.front(), true);
		
		// shader method:
		shaderV.source(srcV, Shader::VERTEX).compile();
		shaderF.source(srcF, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();
		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		// create rendering mesh of lines
		mesh.reset();
		mesh.primitive(Graphics::TRIANGLES);
		for (int x=0; x<32; x++) {
		for (int y=0; y<32; y++) {
		for (int z=0; z<32; z++) {
			// render 2 vertices at the same location
			// using texcoord as an attribute to distinguish 
			// 'start' and 'end' vertices
			mesh.texCoord(0, 0);
			mesh.vertex(x, y, z);
			mesh.texCoord(0, 1);	
			mesh.vertex(x, y, z);
			mesh.texCoord(1, 0);	
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
		
		
//		// add some forces:
//		float t = MainLoop::now() * 0.25;
//		float r = 20;
//		fluid.addVelocity(Vec3f(12, 12, 16), Vec3f(r*cos(t), r*sin(t), 1));
//		fluid.addVelocity(Vec3f(20, 20, 16), Vec3f(-r*cos(t*2), -r*sin(t*2), 0));
//		fluid.addVelocity(Vec3f(16, 16, 20), Vec3f(0, -r*cos(t*3), -r*sin(t*3)));


		
		// add some intensities:
		float v = 4;
		intensities.add(Vec3f(12, 20, 16), Vec3f(v, 0, 0).elems());
		intensities.add(Vec3f(20, 12, 16), Vec3f(0, v, 0).elems());
		intensities.add(Vec3f(16, 16, 12), Vec3f(0, 0, v).elems());

		float g;
		
			
			
			g = 2;
			fluid.addGradient(Vec3f(16, 17, 16), g);
			g = -2;
			fluid.addGradient(Vec3f(16, 16, 16), g);
		
		// run a fluid step:
		fluid.update();
		
		
//		// VELOCITIES:
//		// add a bit of random noise:
//		fluid.velocities.adduniformS(rng, fluid.selfbackgroundnoise);
//		// assume new data is in front();
//		// smoothen the new data:
//		fluid.velocities.diffuse(fluid.viscocity, fluid.passes);
//		// zero velocities at boundaries:
//		fluid.boundary();
//		// (diffused data now in velocities.front())
//		// stabilize: 
//		fluid.project();
////			// prepare new gradient data:
////			fluid.velocities.calculateGradientMagnitude(fluid.gradient.front());
////			
////			// diffuse it (swaps):
////			fluid.gradient.diffuse(0.5, fluid.passes/2);
////			//fluid.gradient.back().zero();
////			
////			
////			// subtract from current velocities:
////			fluid.velocities.subtractGradientMagnitude(fluid.gradient.front());
//		
//		// (projected data now in velocities.front())
//		// advect velocities:
//		fluid.velocities.advect(fluid.velocities.back(), fluid.selfadvection);
//		// zero velocities at boundaries:
//		fluid.boundary();
//		// (advected data now in velocities.front())
//		// stabilize again:
//		fluid.project();
//		// (projected data now in velocities.front())
//		fluid.velocities.scale(fluid.selfdecay);
//		// zero velocities at boundaries:
//		fluid.boundary();
		
		
		// diffuse the intensities:
		intensities.diffuse();
		
		// use the fluid to advect the intensities:
		intensities.advect(fluid.velocities.front(), 3.);
		
		// some decay
		intensities.scale(0.999);
		
		// update texture data:
		intensityTex.submit(intensities.front());
		velocityTex.submit(fluid.velocities.front());
		
		// draw it:
		gl.blending(true);
		gl.blendModeAdd();
		gl.lineWidth(0.5);
		
		gl.polygonMode(gl.LINE);
		
		shaderP.begin();
		shaderP.uniform("velocityTex", 0);
		shaderP.uniform("intensityTex", 1);
		velocityTex.bind(0);
		intensityTex.bind(1);
		gl.draw(mesh);
		intensityTex.unbind(1);
		velocityTex.unbind(0);
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

