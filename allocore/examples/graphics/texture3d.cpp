/*
Allocore Example: Texture3D

Description:
This demonstrates a dynamic 3D texture.

Author:
Graham Wakefield 2011
Lance Putnam, Nov. 2015 (port to App class)
*/


#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/system/al_Time.hpp"
using namespace al;

// function used to initialize array data:
void arrayfiller(float * values, double normx, double normy, double normz) {
	double snormx = normx-0.5;
	double snormy = normy-0.5;
	double snormz = normz-0.5;
	double snorm3 = snormx*snormx + snormy*snormy + snormz*snormz;
	double t = al_time();

	values[0] = sin((2*snorm3 + t) * M_PI);
	values[1] = cos((2*snorm3 + t) * 4);
	values[2] = sin((2*snorm3 + t) * 1.511717);
}

class MyApp : public App {
public:

	static const int N = 32; // grid resolution
	
	Array data;		// 3d array of triplet floats (e.g. color)
	Texture tex;	// Texture that will be filled with this data
	Mesh mesh;
	ShaderProgram shader;
	int renderMode;

	MyApp()
	:	data(3, AlloFloat32Ty, N, N, N), renderMode(0)
	{
		// create rendering mesh; a 3D grid of points:
		mesh.primitive(Graphics::POINTS);
		for (int k=0; k<N; k++) {
		for (int j=0; j<N; j++) {
		for (int i=0; i<N; i++) {
			float x = float(i)/N;
			float y = float(j)/N;
			float z = float(k)/N;
			mesh.texCoord(x, y, z);
			mesh.vertex(x*2-1, y*2-1, z*2-1);
		}}}

		nav().pullBack(4);
		initWindow();
	}

	void onCreate(const ViewpointWindow& w){
		// fill array:
		data.fill(arrayfiller);

		// reconfigure texture based on array:
		tex.submit(data, true);

		// shader method:
		shader.compile(
			AL_STRINGIFY(
			varying vec3 texcoord0;
			void main(){
				texcoord0 = vec3(gl_MultiTexCoord0);
				gl_Position = ftransform();
			}
			),
			AL_STRINGIFY(
			uniform sampler3D tex;
			varying vec3 texcoord0;
			void main() {
				vec4 color = texture3D(tex, texcoord0);
				gl_FragColor = color;
			}
			)
		);
	}

	void onKeyDown(const Keyboard& k){
		switch (k.key()) {
			case ' ': renderMode = (renderMode+1)%3; break;
		}
	}

	void onAnimate(double dt){
		// update data:
		data.fill(arrayfiller);

		// how to resubmit data (if it is changing):
		tex.submit(data);	
	}

	void onDraw(Graphics& g){
		g.pointSize(renderMode+0.5);
		g.color(RGB(1));
		mesh.colors().reset();
		switch (renderMode) {
			case 0:
				// do it on the CPU:
				for (int k=0; k<N; k++) {
				for (int j=0; j<N; j++) {
				for (int i=0; i<N; i++) {
					Color color;
					data.read_interp(color.components, i,j,k);
					mesh.color(color);
				}}}
				g.draw(mesh);
				break;
			case 1:
				// use 3D texcoords:
				tex.bind();
				g.draw(mesh);
				tex.unbind();
				break;
			case 2:
				// use shader:
				shader.begin();
				tex.bind();
				g.draw(mesh);
				tex.unbind();
				shader.end();
				break;
			default:;
		}
	}
};

int main(){
	MyApp().start();
}

