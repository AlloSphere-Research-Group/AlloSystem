/*
Allocore Example: Phong Lighting Shader

Description:
This demonstrates the Phong lighting model.

Author(s):
Lance Putnam, 5/1/2011
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"

using namespace al;

static const char * vLight = AL_STRINGIFY(
varying vec3 normal, lightDir, eyeVec;
void main(){
	normal = gl_NormalMatrix * gl_Normal;
	vec3 V = vec3(gl_ModelViewMatrix * gl_Vertex);
	eyeVec = normalize(-V);
	lightDir = normalize(vec3(gl_LightSource[0].position.xyz - V));
	gl_Position = ftransform();
}
);

static const char * fLight = AL_STRINGIFY(
varying vec3 normal, lightDir, eyeVec;
void main(){
	vec4 final_color = gl_FrontMaterial.ambient
		* (gl_FrontLightModelProduct.sceneColor + gl_LightSource[0].ambient);
	vec3 N = normalize(normal);
	vec3 L = lightDir;
	float lambertTerm = dot(N,L);
	if(lambertTerm > 0.){
		final_color += gl_LightSource[0].diffuse
		               * gl_FrontMaterial.diffuse
					   * lambertTerm;
		vec3 E = eyeVec;
		vec3 R = reflect(-L, N);
		float spec = pow( max(dot(R, E), 0.),
		                 gl_FrontMaterial.shininess + 1e-20);
		final_color += gl_LightSource[0].specular
		               * gl_FrontMaterial.specular
					   * spec;
	}
	gl_FragColor = final_color;
}
);



struct MyWindow : Window{

	ShaderProgram shaderP;
	Shader shaderV, shaderF;
	Mesh mesh;
	Graphics gl;
	Light light;
	Material material;
	double angle;

	MyWindow(): angle(0){}

	bool onCreate(){
		shaderV.source(vLight, Shader::VERTEX).compile();
		shaderF.source(fLight, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();
		
		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		return true;
	}

	bool onFrame(){
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-2.5), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(true);

		// Create a shape
		mesh.reset();
		mesh.primitive(gl.TRIANGLES);
		addSphere(mesh, 1, 16, 16);

		if((angle+=0.003)>=M_2PI) angle-=M_2PI;

		Mat4f xfm;
		xfm.setIdentity();
		xfm.rotate(M_PI/2, 0,2).rotate(angle, 1,2).rotate(angle*8, 0,1);
		xfm.scale(Vec3f(0.5, 0.2, 1));

		mesh.transform(xfm);
		mesh.generateNormals();

		// Render
		shaderP.begin();
			light.dir(1,1,1);
			material();
			light();
			gl.draw(mesh);
		shaderP.end();

		return true;
	}

	virtual bool onKeyDown(const Keyboard& k){
		if(k.key('s')) shaderP.toggleActive();
		return true;
	}

};

MyWindow win;

int main(){
	win.add(new StandardWindowKeyControls);
	win.create();
	MainLoop::start();
}
