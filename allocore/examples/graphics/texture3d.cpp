/*
Allocore Example: Texture3D

Description:
This demonstrates a dynamic 3D texture.

Author:
Graham Wakefield 2011
*/


#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"

using namespace al;

// 3d array of triplet floats (e.g. color)
Array data(3, AlloFloat32Ty, 32, 32, 32);
// texture that will be filled with this data:
Texture tex;

Graphics gl;
Mesh mesh;
ShaderProgram shaderP;
Shader shaderV, shaderF;

int gRenderMode = 0;


//static const char * vLight = AL_STRINGIFY(
//varying vec3 texcoord0;
//void main(){
//	texcoord0 = vec3(gl_MultiTexCoord0);
//	gl_Position = ftransform();
//}
//);

static const char * vLight = AL_STRINGIFY(
varying vec3 texcoord0;
void main(){
	texcoord0 = gl_Vertex.xyz / 32.; 
	//texcoord0 = vec3(gl_MultiTexCoord0);
	gl_Position = ftransform();
}
);

static const char * fLight = AL_STRINGIFY(
uniform sampler3D tex; 
varying vec3 texcoord0;
void main() {
	vec4 color = texture3D(tex, texcoord0);
	gl_FragColor = color;
}
);

// function used to initialize array data:
void arrayfiller(float * values, double normx, double normy, double normz) {
	double snormx = normx-0.5;
	double snormy = normy-0.5;
	double snormz = normz-0.5;
	double snorm3 = snormx*snormx + snormy*snormy + snormz*snormz;

	values[0] = sin((snorm3 + al_time()) * M_PI);
	values[1] = cos((snorm3 + al_time()) * M_2PI);
	values[2] = sin((snorm3 + al_time()) * M_PI_2);
}

struct MyWindow : public Window {

	bool onKeyDown(const Keyboard& k){	 	
		switch (k.key()) {
			case ' ': gRenderMode = (gRenderMode+1)%3; break;
		}
		return true;
	}

	bool onCreate(){
	
		// fill array:
		data.fill(arrayfiller);
		
		// reconfigure texture based on array:
		tex.submit(data, true);
		
		// shader method:
		shaderV.source(vLight, Shader::VERTEX).compile();
		shaderF.source(fLight, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();
		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		// create rendering mesh:
		mesh.reset();
		mesh.primitive(Graphics::POINTS);
		for (int x=0; x<32; x++) {
		for (int y=0; y<32; y++) {
		for (int z=0; z<32; z++) {
			mesh.texCoord(x/32., y/32., z/32.);
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
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(16, 16, 48), Vec3d(16, 16, 16), Vec3d(0,1,0)));
		
		// update data:
		data.fill(arrayfiller);
		
		// how to resubmit data (if it is changing):
		tex.submit(data);
		
		gl.pointSize(gRenderMode+0.5);
		switch (gRenderMode) {
			case 0:
				gl.begin(gl.POINTS);
				// do it on the CPU:
				for (int x=0; x<32; x++) {
				for (int y=0; y<32; y++) {
				for (int z=0; z<32; z++) {
					Color color;
					data.read_interp(color.components, x, y, z);
					gl.color(color);
					gl.vertex(x, y, z);
				}}}
				gl.end();
				break;
			case 1:
				// use 3D texcoords:
				tex.bind();
				gl.draw(mesh);
				tex.unbind();
				break;
			case 2:
				// use shader:
				shaderP.begin();
				tex.bind();
				gl.draw(mesh);
				tex.unbind();
				shaderP.end();
				break;
			default:
				break;
		}

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

