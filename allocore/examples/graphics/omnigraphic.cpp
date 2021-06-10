/*
Allocore Example: omnigraphic

Description:
This demonstrates stereo panorama by omnigraphic method

Author:
Graham Wakefield 2011
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"

std::string omniVS = AL_STRINGIFY(

	// distort the scene per-vertex:
	uniform float fovy, aspect, near, far;
	uniform float omniFov, eyeSep, focal;
	uniform int omni;
	varying vec4 color;

	float M_DEG2RAD = 0.017453292519943;
	float M_1_PI = 0.31830988618379;

	// optimized application of perspective projection matrix
	// assumes input vertex w == 1
	vec4 perspective(in vec3 v, in float fovy, in float aspect, in float near, in float far) {
		float f = 1./tan(fovy*M_DEG2RAD/2.);
		float x = v.x * f/aspect;
		float y = v.y * f;
		//float d = far-near;
		//float z = (v.z * -(far+near)/d) + (v.w * -2.*far*near/d);
		float z = (-v.z*(far+near) - 2.*far*near) / (far-near);
		float w = -v.z;
		return vec4(x, y, z, w);
	}

	// omnigraphic:
	vec4 omnigraphic(in vec3 v, in float omniFov, in float aspect, in float near, in float far) {
		float f = 2./(omniFov * M_DEG2RAD);
		float azimuth = atan(v.x, -v.z);
		float elevation = atan(v.y, length(v.xz));
		float d = length(v.xyz) * sign(v.z);

		float x = f * azimuth;
		float y = f * elevation * aspect;

		// depth ortho-style:
		float z = (-2.*d - far+near) / (far-near);
		float w = 1.;	// no perspective effect
		return vec4(x, y, z, w);
	}

	vec4 omnigraphic2(in vec3 v, in float omniFov, in float aspect, in float near, in float far, in float eyeSep, in float focal) {
		float f = 2./(omniFov * M_DEG2RAD);
		float azimuth = atan(v.x, -v.z);
		float elevation = atan(v.y, length(v.xz));
		float d = length(v.xyz) * sign(v.z);

		// stereo rotation depends on depth:
		azimuth += eyeSep; // * M_1_PI * (d/focal);

		float x = f * azimuth;
		float y = f * elevation * aspect;

		// depth ortho-style:
		float z = (-2.*d - far+near) / (far-near);
		float w = 1.;	// no perspective effect
		return vec4(x, y, z, w);
	}

	void main(void) {

		// convert object to view space:
		vec4 vertex = gl_ModelViewMatrix * gl_Vertex;

		// but bypass the gl_ProjectionMatrix
		if (omni > 0) {
			vertex = omnigraphic2(vertex.xyz, omniFov, aspect, near, far, eyeSep, focal);
		} else {
			vertex = perspective(vertex.xyz, fovy, aspect, near, far);
		}

		gl_Position = vertex;

		color = gl_Color;

	}
);

std::string omniFS = AL_STRINGIFY(
	// just a typical fragment shader:

	varying vec4 color;

	void main(void) {
		gl_FragColor = color;
	}
);


using namespace al;

static Graphics gl;
static Mesh mesh, grid;
static Stereoscopic stereo;
static Lens lens;
Nav nav;
bool useShader = false;

/*
	Omnigraphic mode splits up the window into vertical slices (viewports)
	Each viewport has a slightly rotated view of the scene, such that the
	borders match up and create a seamless panoramic view.
	In effect, it presents a cylindrical map.
	With stereographics, it creates omnistereo (where stereoscopy is continuous
	around the cylinder)

	However it can be expensive, as each slice is a full scene render.

	Perhaps a similar effect can be achieved with a shader?:

		According to the polar angle of a vertex, displace it with a modified
		version of the ModelViewProjection matrix (instead of e.g. fttransform())

		The modelview/projection matrices can be calculated in the same way as
		the Stereoscopic class (perspectiveLeft/Right, lookatLeft/Right)

	The advantage would be full spherical omnistereo, and hopefully cheaper.
*/

struct MyWindow : Window, public Drawable{

	bool onKeyDown(const Keyboard& k){

		switch(k.key()){
			case 'o': stereo.omni(!stereo.omni()); return false;
			case 's': useShader = !useShader; return false;
			case Keyboard::TAB: stereo.stereo(!stereo.stereo()); return false;
			case '1': stereo.mode(Stereoscopic::ANAGLYPH); return false;
			case '2': stereo.mode(Stereoscopic::ACTIVE); return false;
			case '3': stereo.mode(Stereoscopic::DUAL); return false;
			case '4': stereo.mode(Stereoscopic::LEFT_EYE); return false;
			case '5': stereo.mode(Stereoscopic::RIGHT_EYE); return false;
			default: return true;
		}
	}

	bool onCreate(){
		Shader omniV(omniVS, Shader::VERTEX);
		Shader omniF(omniFS, Shader::FRAGMENT);
		omniV.compile();
		omniF.compile();
		omniP.attach(omniV).attach(omniF).link();

		omniV.printLog();
		omniF.printLog();
		omniP.printLog();

		stereo.clearColor(Color(1, 1, 1, 1));
		return true;
	}

	bool onFrame(){
		nav.step();
		if (useShader) {
			if (stereo.stereo()) {

				Viewport vp(width(), height());
				gl.viewport(vp);
				gl.clearColor(stereo.clearColor());
				gl.depthMask(1);
				gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
				gl.depthTesting(1);
				gl.modelView(Matrix4d::lookAt(nav.ux(), nav.uy(), nav.uz(), nav.pos()));

				glColorMask(GL_TRUE, GL_FALSE,GL_FALSE,GL_TRUE);

				omniP.begin();
				omniP.uniform("fovy", lens.fovy());
				omniP.uniform("omniFov", stereo.omniFov());
				omniP.uniform("aspect", vp.aspect());
				omniP.uniform("near", lens.near());
				omniP.uniform("far", lens.far());
				omniP.uniform("focal", lens.focalLength());
				omniP.uniform("omni", stereo.omni());
				omniP.uniform("eyeSep", lens.eyeSep());
				gl.draw(grid);
				gl.draw(mesh);
				omniP.end();

				glColorMask(GL_FALSE,GL_TRUE, GL_TRUE, GL_TRUE);
				gl.clear(Graphics::DEPTH_BUFFER_BIT);

				omniP.begin();
				omniP.uniform("fovy", lens.fovy());
				omniP.uniform("omniFov", stereo.omniFov());
				omniP.uniform("aspect", vp.aspect());
				omniP.uniform("near", lens.near());
				omniP.uniform("far", lens.far());
				omniP.uniform("focal", lens.focalLength());
				omniP.uniform("omni", stereo.omni());
				omniP.uniform("eyeSep", -lens.eyeSep());
				gl.draw(grid);
				gl.draw(mesh);
				omniP.end();

				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

			} else {
				// MONO:

				Viewport vp(width(), height());
				gl.viewport(vp);
				gl.clearColor(stereo.clearColor());
				gl.depthMask(1);
				gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
				gl.depthTesting(1);
				gl.modelView(Matrix4d::lookAt(nav.ux(), nav.uy(), nav.uz(), nav.pos()));

				omniP.begin();
				omniP.uniform("fovy", lens.fovy());
				omniP.uniform("omniFov", stereo.omniFov());
				omniP.uniform("aspect", vp.aspect());
				omniP.uniform("near", lens.near());
				omniP.uniform("far", lens.far());
				omniP.uniform("focal", lens.focalLength());
				omniP.uniform("omni", stereo.omni());
				omniP.uniform("eyeSep", 0.);
				gl.draw(grid);
				gl.draw(mesh);
				omniP.end();
			}

		} else {
			stereo.draw(gl, lens, nav, Viewport(width(), height()), *this);
		}
		return true;
	}

	void onDraw(Graphics& gl){
		gl.fog(lens.far(), lens.far()/2, stereo.clearColor());
		gl.depthTesting(1);
		gl.draw(grid);
		gl.draw(mesh);
	}

	ShaderProgram omniP;
};

MyWindow win;

int main(){

	nav.smooth(0.8);
	nav.pos(0, 0, -20);

	lens.near(1).far(100).focalLength(1).fovy(45);
	lens.eyeSep(-lens.eyeSepAuto());
	stereo.omni(true, 24, 120);
	stereo.stereo(false);
	stereo.mode(Stereoscopic::ANAGLYPH);

	// set up mesh:
	mesh.primitive(Graphics::TRIANGLES);
	double tri_size = 2;
	double world_radius = 50;
	int count = 4000;
	for (int i=0; i<count; i++) {
		double x = rnd::uniformS(world_radius);
		double y = rnd::uniformS(world_radius);
		double z = rnd::uniformS(world_radius);
		for (int v=0; v<3; v++) {
			mesh.color(HSV(float(i)/count, v!=2, 1));
			mesh.vertex(x+rnd::uniformS(tri_size), y+rnd::uniformS(tri_size), z+rnd::uniformS(tri_size));
		}
	}

	grid.primitive(Graphics::LINES);
	grid.color(0, 0, 0);
	double stepsize = 1./2;
	double tsize = 0.1;
	for (double x=-1; x<=1; x+= stepsize) {
	for (double y=-1; y<=1; y+= stepsize) {
	for (double t=-1; t<1; t+= tsize) {
		grid.vertex(x, y, t);
		grid.vertex(x, y, t+tsize);
	}}}
	for (double x=-1; x<=1; x+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
	for (double t=-1; t<1; t+= tsize) {
		grid.vertex(x, t, z);
		grid.vertex(x, t+tsize, z);
	}}}
	for (double y=-1; y<=1; y+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
	for (double t=-1; t<1; t+= tsize) {
		grid.vertex(t, y, z);
		grid.vertex(t+tsize, y, z);
	}}}
	grid.scale(world_radius);

	win.create(Window::Dim(100, 0, 640, 480), "Omnigraphic Example", 60);

	win.displayMode(win.displayMode() | Window::STEREO_BUF);

	win.append(*new StandardWindowKeyControls);
	win.append(*new NavInputControl(nav));

	MainLoop::start();
    return 0;
}
