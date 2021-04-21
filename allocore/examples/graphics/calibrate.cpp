/*
Projector calibration for the AlloSphere

NOTES:

The clip space z and w can be calculated the same way as orthographic.
The xy (or uv) must be calculated from the vertex position v.

We know azimuth/elevation to the corners ABCD. From this we can derive their spatial locations.
Assume the allosphere is a unit sphere, then the result is simple polar to cartesian.
	This can be refined by modeling the sphere properly as two unit hemispheres joined by a cylinder of some width W.
	The adjustment W can be partially verified by ensuring that the normals of ABC and BCD are equal.
Calculate point Q as the center of ABCD (B.x-A.x, D.y-A.y)
	Again, two ways to calculate Q, which can be used to minimize error in the sphere model.
	Alternatively, we can measure Q empirically to further minimize error.
Assume a projector location F (or derive from measuring the lens position).

Vertex locations v must be projected into the view of the projector, which is a matrix multiplication.
We only care about the XY components of the result, but these must be scaled such that A is -1,-1 and C is 1, 1.
This can be achived by dividing by (the result of projecting QC into the view)

A view matrix can be constructed as:
		Matrix4(
			 ux[0], ux[1], ux[2], -(ux.dot(eyePos)),
			 uy[0], uy[1], uy[2], -(uy.dot(eyePos)),
			 uz[0], uz[1], uz[2], -(uz.dot(eyePos)),
			0, 0, 0, 1
		);

eye == F
R = norm(((B+C)/2) - Q)  // unit vector from Q to midpoint of BC
uz = norm(FQ)
ux = -cross(norm(cross(R, uz)), uz)
	or, since uz = parallel(R,uz) + perpendicular(R,uz):
	perpendicular(R,uz) = uz - parallel(R,uz)
	ux = uz - R*dot(R, uz)

uy = cross(ux, uz)

simplified:


u = dot(v-F, ux) / dot(QC-F, ux);
v = dot(v-F, uy) / dot(QC-F, uy);

note that denominator is a constant.
Not sure if v should be normalized or not, or whether this makes any difference at all.

*/

#include "allocore/io/al_Window.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Light.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Shapes.hpp"

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
	bool useShader;
	bool bGrid;

	MyWindow(): angle(0), bGrid(true) {}

	bool onCreate(){
		shaderV.source(vLight, Shader::VERTEX).compile();
		shaderF.source(fLight, Shader::FRAGMENT).compile();
		shaderP.attach(shaderV).attach(shaderF).link();

		shaderV.printLog();
		shaderF.printLog();
		shaderP.printLog();

		return true;
	}

	void grid(float w, float h, int rep) {
		gl.color(HSV(1./rep, 1., 1.));
		for (int v=w; v<width(); v+=w*2) {
			gl.vertex(v, 1, 0);
			gl.vertex(v, height(), 0);
		}
		for (int v=h; v<height(); v+=h*2) {
			gl.vertex(1, v, 0);
			gl.vertex(width(), v, 0);

		}
		if (rep > 0) {
			grid(w/2, h/2, rep-1);
		}
	}

	void drawgrid() {
		gl.projection(Matrix4d::ortho(0, width(), 0, height(), -1, 1));
			gl.modelView(Matrix4d::identity());

			// draw grid
			//gl.lineWidth(0.5);
			gl.begin(gl.LINES);
			gl.color(0.5, 0.5, 0.5, 1);
			float w = width();
			float h = height();

			gl.color(1, 1, 1);
			gl.vertex(1, 1, 0);
			gl.vertex(1, height(), 0);
			gl.vertex(width(), 1, 0);
			gl.vertex(width(), height(), 0);
			gl.vertex(1, 1, 0);
			gl.vertex(width(), 0, 0);
			gl.vertex(1, height(), 0);
			gl.vertex(width(), height(), 0);

			grid(w/2, h/2, 2);

			gl.end();
	}

	bool onFrame(){
		gl.clearColor(0,0,0,1);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());

		if (bGrid) {
			gl.viewport(0,0, width(), height()/2);
			drawgrid();
			gl.viewport(0,height()/2, width(), height()/2);
			drawgrid();
		} else {
			gl.matrixMode(gl.PROJECTION);
			gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));
			gl.matrixMode(gl.MODELVIEW);
			gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-2.5), Vec3d(0,0,0), Vec3d(0,1,0)));

			gl.depthTesting(true);

			// Create a shape
			mesh.reset();
			mesh.primitive(gl.TRIANGLES);
			//addIcosahedron(mesh);
			//addDodecahedron(mesh);
			addSphere(mesh, 1, 16, 16);

			if((angle+=0.003)>=M_2PI) angle-=M_2PI;

			Mat4f xfm;
			xfm.setIdentity();
			xfm.rotate(M_PI/2, 0,2).rotate(angle, 1,2).rotate(angle*8, 0,1);
			xfm.scale(Vec3f(0.5, 0.2, 1));

			mesh.transform(xfm);
			//mesh.decompress();
			mesh.generateNormals();

			// Render
			shaderP.begin();

				material.shininess(2);

				light.ambient(Color(0.1,0,0));
				light.diffuse(Color(0.9,0,0));
				light.specular(Color(0.5));

				material();
				light();
				gl.draw(mesh);

			shaderP.end();
		}

		return true;
	}

	virtual bool onKeyDown(const Keyboard& k){
		if(k.key('s')) shaderP.toggleActive();
		return true;
	}

};

int main(){

	MyWindow win1;
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
