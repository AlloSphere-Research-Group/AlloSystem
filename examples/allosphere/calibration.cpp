/*
Allocore Example: Allosphere calibration testing

Description:
Used in testing calibration of the Allosphere projectors

Author:
Graham Wakefield 2012



Geometry map UV is normalized in 0..1; values outside that range indicate pixels to be discarded. 
I think this means that it is a replacement of the NDC coordinates.

*/

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_ControlNav.hpp"
#include "alloutil/al_CubeMapFBO.hpp"
#include "allocore/graphics/al_Shader.hpp"

#define TEST

using namespace al;

Texture geometryMap;
Texture alphaMap;

ShaderProgram geomP;
Shader geomV, geomF;

ShaderProgram vscnP;
Shader vscnV, vscnF;

ShaderProgram vdisP;
Shader vdisV, vdisF;

enum {
	GEOMETRY_MAP = 1,
	ALPHA_MAP,
	PLAIN_SCENE,
	VERTEX_SCENE,
	VERTEX_DISPLACE,
};

int mode = VERTEX_SCENE;

static Graphics gl;
static Mesh mesh, grid, cube;
static Camera cam;
static Nav nav;
static CubeMapFBO cubeFBO;
unsigned drawMode = 1;
rnd::Random<> rng;

static inline void swap(char * a, char * b){ char t=*a; *a=*b; *b=t; }
	static inline void swap16(void * v){	
		char * b = (char *)v;
		swap(b  , b+1);
	}
	static inline void swap32(void * v){	
		char * b = (char *)v;
		swap(b  , b+3);
		swap(b+1, b+2);
	}
	static inline void swap64(void * v){	
		char * b = (char *)v;
		swap(b  , b+7);
		swap(b+1, b+6);
		swap(b+2, b+5);
		swap(b+3, b+4);
	}

// this shader is just to show the geometry map:
static const char * geomVS = AL_STRINGIFY(
	varying vec2 texcoord0;
	void main(){
		texcoord0 = vec2(gl_MultiTexCoord0);
		vec4 vertex = gl_Vertex;
		gl_Position = gl_ModelViewProjectionMatrix * vertex;
	}
);
static const char * geomFS = AL_STRINGIFY(
	uniform sampler2D geometryMap;
	varying vec2 texcoord0;
	void main(){
		vec4 g = texture2D(geometryMap, texcoord0);
		gl_FragColor = g.wxyz;
	}
);

// render the scene using vertex transformation
static const char * vscnVS = AL_STRINGIFY(
	uniform sampler2D geometryMap;
	
	uniform vec4 viewquat;
	uniform vec3 viewpos;
	uniform float fovy, aspect, near, far;
	
	varying vec2 texcoord0;
	
	// equiv. quat_rotate(quat_conj(q), v):
	// q must be a normalized quaternion
	vec3 quat_unrotate(in vec4 q, in vec3 v) {
		// return quat_mul(quat_mul(quat_conj(q), vec4(v, 0)), q).xyz;
		// reduced:
		vec4 p = vec4(
			q.w*v.x - q.y*v.z + q.z*v.y,  // x
			q.w*v.y - q.z*v.x + q.x*v.z,  // y
			q.w*v.z - q.x*v.y + q.y*v.x,  // z
			q.x*v.x + q.y*v.y + q.z*v.z   // w
		);
		return vec3(
			p.w*q.x + p.x*q.w + p.y*q.z - p.z*q.y,  // x
			p.w*q.y + p.y*q.w + p.z*q.x - p.x*q.z,  // y
			p.w*q.z + p.z*q.w + p.x*q.y - p.y*q.x   // z
		);
	}
	
	vec4 eyespace(in vec4 vertex, in vec4 viewquat, in vec3 viewpos) {
		return vec4(quat_unrotate(viewquat, vertex.xyz - viewpos), vertex.w);
	}
	
	// optimized application of perspective projection matrix:
	vec4 perspectivize(in vec4 v, in float fovy, in float aspect, in float near, in float far) {
		// magic number is (degrees => radians)/2.:
		float f = 1./tan(fovy*0.008726646259972);
		return vec4(
			v.x * f/aspect,
			v.y * f,
			(-v.z*(far+near) + v.w*-2.*far*near) / (far-near),
			-v.z
		);
	}
	
	void main(){
		const float M_1_PI = 0.31830988618379;
		
		vec4 vertex = gl_Vertex;
		// this is now the vertex relative to the eye:
		vertex = eyespace(vertex, viewquat, viewpos);
		
		// cartesian to polar:
		vec2 gcoord = vec2(
			atan(vertex.x, -vertex.z),
			atan(vertex.y, length(vertex.xz))
		) * M_1_PI;
		
		// replace projection matrix:
		vertex = perspectivize(vertex, fovy, aspect, near, far);

		gl_Position = vertex;
		gl_FrontColor = vec4(gcoord+0.5, 0.5, 1);
	}
);
static const char * vscnFS = AL_STRINGIFY(
	void main(){
		gl_FragColor = gl_Color;
	}
);


// this shader does the vertex displacement:
static const char * vdisVS = AL_STRINGIFY(
	uniform sampler2D geometryMap;
	
	uniform vec4 viewquat;
	uniform vec3 viewpos;
	uniform float fovy, aspect, near, far;
	
	varying vec2 texcoord0, uv;
	
	// equiv. quat_rotate(quat_conj(q), v):
	// q must be a normalized quaternion
	vec3 quat_unrotate(in vec4 q, in vec3 v) {
		// return quat_mul(quat_mul(quat_conj(q), vec4(v, 0)), q).xyz;
		// reduced:
		vec4 p = vec4(
			q.w*v.x - q.y*v.z + q.z*v.y,  // x
			q.w*v.y - q.z*v.x + q.x*v.z,  // y
			q.w*v.z - q.x*v.y + q.y*v.x,  // z
			q.x*v.x + q.y*v.y + q.z*v.z   // w
		);
		return vec3(
			p.w*q.x + p.x*q.w + p.y*q.z - p.z*q.y,  // x
			p.w*q.y + p.y*q.w + p.z*q.x - p.x*q.z,  // y
			p.w*q.z + p.z*q.w + p.x*q.y - p.y*q.x   // z
		);
	}
	
	vec4 eyespace(in vec4 vertex, in vec4 viewquat, in vec3 viewpos) {
		return vec4(quat_unrotate(viewquat, vertex.xyz - viewpos), vertex.w);
	}
	
	// optimized application of perspective projection matrix:
	vec4 perspectivize(in vec4 v, in float fovy, in float aspect, in float near, in float far) {
		// magic number is (degrees => radians)/2.:
		float f = 1./tan(fovy*0.008726646259972);
		return vec4(
			v.x * f/aspect,
			v.y * f,
			(-v.z*(far+near) + v.w*-2.*far*near) / (far-near),
			-v.z
		);
	}
	
	void main(){
		
		vec4 vertex = gl_Vertex;
		// this is now the vertex relative to the eye:
		vertex = eyespace(vertex, viewquat, viewpos);
		
		// cartesian to polar:
		vec2 gcoord = vec2(
			atan(vertex.x, -vertex.z),
			atan(vertex.y, length(vertex.xz))
		);
		
		// perform normal projection matrix:
		vertex = perspectivize(vertex, fovy, aspect, near, far);
		
		// use this to index the texture:
		uv = texture2D(geometryMap, vertex.xy*0.5+0.5).xw;
		vertex.xy = uv*2.-1.;
		
		// apply:
		gl_Position = vertex;
		
		/*
		float z = vertex.z;
		float w = vertex.w;
		gl_Position = vec4(
			// clip space:
			(uv.x*2.-1.)/aspect,
			(uv.y*2.-1.), 
			// perspective effect:
			z * (far+near)/(near-far) + w*(2.*far*near)/(near-far),
			-z
		);
		*/
		
		gl_FrontColor = gl_Color;
		texcoord0 = vec2(gl_MultiTexCoord0);
	}
);
static const char * vdisFS = AL_STRINGIFY(
	uniform sampler2D alphaMap;
	varying vec2 texcoord0, uv;
	void main(){
	
		//if (uv.x <= 0. || uv.x >= 1.) discard;
		//if (uv.y <= 0. || uv.y >= 1.) discard; 
	
		float a = texture2D(alphaMap, texcoord0).a;
		gl_FragColor = gl_Color;
		gl_FragColor.a *= a;
	}
);

struct MyWindow : Window, public Drawable{

	bool onKeyDown(const Keyboard& k){
		int key = k.key();
		printf("key %d %c\n", k.key(), k.key());
		if (key >= 48 && key < 58) {
			mode = key-48;	// number keys
		}
		return true;
	}
	
	bool onCreate(){
		geomV.source(geomVS, Shader::VERTEX).compile();
		geomF.source(geomFS, Shader::FRAGMENT).compile();
		geomP.attach(geomV).attach(geomF).link();
		
		geomV.printLog();
		geomF.printLog();
		geomP.printLog();
		
		vdisV.source(vdisVS, Shader::VERTEX).compile();
		vdisF.source(vdisFS, Shader::FRAGMENT).compile();
		vdisP.attach(vdisV).attach(vdisF).link();
		
		vdisV.printLog();
		vdisF.printLog();
		vdisP.printLog();
		
		vscnV.source(vscnVS, Shader::VERTEX).compile();
		vscnF.source(vscnFS, Shader::FRAGMENT).compile();
		vscnP.attach(vscnV).attach(vscnF).link();
		
		vscnV.printLog();
		vscnF.printLog();
		vscnP.printLog();
		
		Image img("../../share/sphere_data/alpha1.jpg");
		alphaMap.submit(img.array(), true);

		return true;
	}

	bool onFrame(){
		nav.step();
		float aspect = width()/(double)height();
	
		gl.viewport(0, 0, width(), height());
		gl.clearColor(0., 0., 0., 0);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.depthTesting(false);
		gl.lighting(false);
		gl.blending(true);
		gl.antialiasing(Graphics::NICEST);
		
		Vec3d pos = nav.pos();
		Vec3d ux, uy, uz; 
		nav.unitVectors(ux, uy, uz);
		
		switch (mode) {
			case GEOMETRY_MAP:
				gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
				gl.modelView(Matrix4d::identity());
				geomP.begin();
				geomP.uniform("geometryMap", 0);
				geometryMap.quad(gl);
				geomP.end();
				break;
			case ALPHA_MAP:
				gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
				gl.modelView(Matrix4d::identity());
				alphaMap.quad(gl);
				break;
			case PLAIN_SCENE: 
				{
					gl.projection(Matrix4d::perspective(cam.fovy(), aspect, cam.near(), cam.far()));
					gl.modelView(Matrix4d::lookAt(ux, uy, uz, pos));
					
					onDraw(gl);
				} 
				break;
			case VERTEX_SCENE: 
				{
					gl.projection(Matrix4d::perspective(cam.fovy(), aspect, cam.near(), cam.far()));
					gl.modelView(Matrix4d::lookAt(ux, uy, uz, pos));
				
					// draw the scene, pushing through vdis shader:
					vscnP.begin();
					vscnP.uniform("viewquat", nav.quat());
					vscnP.uniform("viewpos", nav.pos());
					
					vscnP.uniform("fovy", cam.fovy());
					vscnP.uniform("aspect", aspect);
					vscnP.uniform("near", cam.near());
					vscnP.uniform("far", cam.far());
					
					// draw scene
					onDraw(gl);
					
					vscnP.end();
				} 
				break;
			case VERTEX_DISPLACE: 
				{
					gl.projection(Matrix4d::perspective(90, width()/(double)height(), cam.near(), cam.far()));
					gl.modelView(Matrix4d::lookAt(ux, uy, uz, pos));
				
					// draw the scene, pushing through vdis shader:
					vdisP.begin();
					vdisP.uniform("geometryMap", 0);
					vdisP.uniform("alphaMap", 1);
					
					vdisP.uniform("viewquat", nav.quat());
					vdisP.uniform("viewpos", nav.pos());
					vdisP.uniform("fovy", cam.fovy());
					vdisP.uniform("aspect", aspect);
					vdisP.uniform("near", cam.near());
					vdisP.uniform("far", cam.far());
					
					geometryMap.bind(0);
					alphaMap.bind(1);
					
					// draw scene
					onDraw(gl);
					
					alphaMap.unbind(1);
					geometryMap.unbind(0);
					vdisP.end();
				} 
				break;
			default:
				break;
		}
		return true;
	}

	void onDraw(Graphics& gl){
		//gl.fog(cam.far(), cam.far()/2, cubeFBO.clearColor());
		gl.depthTesting(1);
		gl.draw(grid);
		gl.draw(mesh);	
	}
};

MyWindow win;

void demodata(float * cell, double normx, double normy) {
	cell[0] = sin(M_PI * (normx) * 3.);
	cell[1] = cos(M_PI * (normy-0.5) * 3.);
} 

int main(int argc, char ** argv){

	chdir("./");

	double world_radius = 50;
	
	nav.smooth(0.8);
	cam.near(1).far(world_radius).fovy(90);
	
	{
		File f("../../share/sphere_data/map1.bin", "rb", true); 
		
		int32_t dim[2];
		f.read((void *)dim, sizeof(int32_t), 2);
		printf("reading map %dx%d; ", dim[0], dim[1]);
		
		int32_t w = dim[1];
		int32_t h = dim[0]/2;
		int32_t elems = w*h;
		printf("array %dx%d = %d\n", w, h, elems);
		
		int r = 0;	
		
		float * u = (float *)malloc(sizeof(float) * elems);
		r = f.read((void *)u, sizeof(float), elems);
		printf("read %d elements (array size %d)\n", r, elems);
		
		float * v = (float *)malloc(sizeof(float) * elems);
		r = f.read((void *)v, sizeof(float), elems);
		printf("read %d elements (array size %d)\n", r, elems);

		f.close();
		
		geometryMap.resize(w, h);
		geometryMap.target(Texture::TEXTURE_2D);
		geometryMap.format(Graphics::LUMINANCE_ALPHA);
		geometryMap.type(Graphics::FLOAT);
		geometryMap.allocate(4);
		geometryMap.print();
		geometryMap.array().print();
		
		Array& arr = geometryMap.array();
		
		for (int y=0; y<h; y++) {
			for (int x=0; x<w; x++) {
				int32_t idx = y*w+x;	// row-major format
			
				float * cell = arr.cell<float>(x, y);
				cell[0] = 0.; //x/(float)w;
				//cell[1] = y/(float)h;
				cell[0] = u[idx];
				cell[1] = v[idx];
				
				if (x % 50 == 0 && y % 50 == 0) {
					printf("x %d y %d: u %f v %f\n", x, y, cell[0], cell[1]);
				}
			}
		}
		
		free(u);
		free(v);
	}
	
	// set up mesh:
	mesh.primitive(Graphics::TRIANGLES);
	double tri_size = 2;
	int count = 400;
	for (int i=0; i<count; i++) {
		double x = rnd::uniformS(world_radius);
		double y = rnd::uniformS(world_radius);
		double z = rnd::uniformS(world_radius);
		for (int v=0; v<3; v++) {
			mesh.color(HSV(float(i)/count, v!=2, 1));
			mesh.vertex(x+rnd::uniformS(tri_size), y+rnd::uniformS(tri_size), z+rnd::uniformS(tri_size));
		}
	}
	
	// set up grid:
	grid.primitive(Graphics::LINES);
	double stepsize = 1./4;
	double tess = 60.;
	for (double x=-1; x<=1; x+= stepsize) {
	for (double y=-1; y<=1; y+= stepsize) {
		for (int i=0; i<=tess*2; i++) {
			float c = 0.2 + i%2;
			grid.color(c, c, c);
			grid.vertex(x, y, 1.-i/tess);
			grid.color(c, c, c);
			grid.vertex(x, y, 1.-(i+1)/tess);
		}
	}}
	for (double x=-1; x<=1; x+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
		for (int i=0; i<=tess*2; i++) {
			float c = 0.2 + i%2;
			grid.color(c, c, c);
			grid.vertex(x, 1.-i/tess, z);
			grid.color(c, c, c);
			grid.vertex(x, 1.-(i+1)/tess, z);
		}
	}}
	for (double y=-1; y<=1; y+= stepsize) {
	for (double z=-1; z<=1; z+= stepsize) {
		for (int i=0; i<=tess*2; i++) {
			float c = 0.2 + i%2;
			grid.color(c, c, c);
			grid.vertex(1.-i/tess, y, z);
			grid.color(c, c, c);
			grid.vertex(1.-(i+1)/tess, y, z);
		}
	}}
	grid.scale(world_radius);
	
	// set up cube:
	cube.color(1,1,1,1);
	cube.primitive(Graphics::TRIANGLES);
	addCube(cube);
	cube.generateNormals();
	
	win.create(Window::Dim(100, 0, 640, 480), "Allosphere", 60);
	win.displayMode(win.displayMode() | /*Window::STEREO_BUF |*/ Window::MULTISAMPLE);
	win.add(new StandardWindowKeyControls);
	win.add(new NavInputControlCosm(nav));

	MainLoop::start();
	
    return 0;
}
