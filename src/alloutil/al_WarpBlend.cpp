#include "alloutil/al_WarpBlend.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Time.hpp"

using namespace al;

Graphics gl;

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
		vec2 g = texture2D(geometryMap, texcoord0).xw;
		//g.y = 1.-g.y;
		gl_FragColor = vec4(g.x, g.y, 0., 1.);
	}
);

// this shader is just to show the 3D geometry map:
static const char * geomVS3D = AL_STRINGIFY(
	varying vec2 texcoord0;
	void main(){
		texcoord0 = vec2(gl_MultiTexCoord0);
		vec4 vertex = gl_Vertex;
		gl_Position = gl_ModelViewProjectionMatrix * vertex;
	}
);
static const char * geomFS3D = AL_STRINGIFY(
	uniform sampler2D pixelMap, alphaMap;
	varying vec2 texcoord0;
	void main(){
		vec3 v = texture2D(pixelMap, texcoord0).rgb;
		float a = texture2D(alphaMap, texcoord0).r;
		v = normalize(v);
		gl_FragColor = vec4(v, a);
	}
);

// this shader is just to show the 3D geometry map in a fancy way
static const char * demoVS = AL_STRINGIFY(
	varying vec2 texcoord0;
	void main(){
		texcoord0 = vec2(gl_MultiTexCoord0);
		vec4 vertex = gl_Vertex;
		gl_Position = gl_ModelViewProjectionMatrix * vertex;
	}
);
static const char * demoFS = AL_STRINGIFY(
	uniform sampler2D pixelMap, alphaMap;
	uniform vec3 pos;
	uniform vec4 quat;
	varying vec2 texcoord0;
	uniform float eyesep;
	
	float pi = 3.141592653589793;
	float piover2 = 1.570796326794897;
	
	// q must be a normalized quaternion
	vec3 quat_rotate(in vec4 q, in vec3 v) {
		// return quat_mul(quat_mul(q, vec4(v, 0)), quat_conj(q)).xyz;
		// reduced:
		vec4 p = vec4(
			q.w*v.x + q.y*v.z - q.z*v.y,  // x
			q.w*v.y + q.z*v.x - q.x*v.z,  // y
			q.w*v.z + q.x*v.y - q.y*v.x,  // z
		   -q.x*v.x - q.y*v.y - q.z*v.z   // w
		);
		return vec3(
			-p.w*q.x + p.x*q.w - p.y*q.z + p.z*q.y,  // x
			-p.w*q.y + p.y*q.w - p.z*q.x + p.x*q.z,  // y
			-p.w*q.z + p.z*q.w - p.x*q.y + p.y*q.x   // z
		);
	}

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

	// for distance ops, the transforms are inverted:
	vec3 translate( vec3 p, vec3 tx ) {
		return p - tx;
	}
	vec3 rotateXY( vec3 p, float theta ) {
		float c = cos(theta);
		float s = sin(theta);
		return vec3(p.x*c - p.y*s, p.x*s + p.y*c, p.z);
	}
	vec3 rotateYZ( vec3 p, float theta ) {
		float c = cos(theta);
		float s = sin(theta);
		return vec3(p.x, p.y*c - p.z*s, p.y*s + p.z*c);
	}
	vec3 rotateXZ( vec3 p, float theta ) {
		float c = cos(theta);
		float s = sin(theta);
		return vec3(p.x*c - p.z*s, p.y, p.x*s + p.z*c);
	}
	
//	// twist
//	vec3 opTwist( vec3 p ) {
//		float c = cos(twparam1); //*p.y);
//		float s = sin(twparam1); //*p.y);
//		mat2  m = mat2(c,-s,s,c);
//		vec3  q = vec3(m*p.xz,p.y);
//		return q;
//	}

	// roundedness:
	float lengthN(vec3 p, float n) {
		return pow(pow(p.x,n) + pow(p.y,n) + pow(p.z,n), 1./n);
	}

	// union is the closest of two distances:
	float opUnion( float d1, float d2 )
	{
		return min(d1,d2);
	}

	// intersection is farthest:
	float opIntersect( float d1, float d2 )
	{
		return max(d1,d2);
	}

	// subtraction:
	float opSubtract( float d1, float d2 ){
		return max(-d1,d2);
	}

	// blending:
	float opBlend( vec3 p, float d1, float d2 ){
		float bfact = smoothstep( length(p), 0., 1. );
		return mix( d1, d2, bfact );
	}

	// repetition:
	vec3 opRepeat( vec3 p, vec3 c ) {
		vec3 q = mod(p,c)-0.5*c;
		return q;
	}

	// distance of p from a sphere radius s (at origin):
	float sdSphere( vec3 p, float s ) {
		return length(p)-s;
	}

	// distance of p from a box of dim b:
	float udBox( vec3 p, vec3 b ) {
	  return length(max(abs(p)-b, 0.0));
	}

	// hexagoanal prism
	float udHexPrism( vec3 p, vec2 h )
	{
		vec3 q = abs(p);
		return max(q.z-h.y,max(q.x+q.y*0.57735,q.y)-h.x);
	}

	float map(vec3 p) {
		vec3 pr = opRepeat(p, vec3(0.8,0.8,0.8));
		float s = udBox(pr, vec3(0.1, 0.3, 0.025));
		return s;
	}

	// shadow ray:
	float shadow( in vec3 ro, in vec3 rd, float mint, float maxt, float mindt, float k ) {
		float res = 1.;
		for( float t=mint; t < maxt; ) {
			float h = map(ro + rd*t);
			// keep looking:
			t += h * 0.5; 
			// blurry penumbra:
			res = min( res, k*h/t );
			if( h<mindt ) {
				// in shadow:
				return res;
			}
		}
		return res;
	}
	
	void main(){
	
		vec3 light1 = vec3(3, 2., 1);
		vec3 light2 = vec3(2, -2., 3.);
		vec3 color1 = vec3(0.5, 1, 0.5);
		vec3 color2 = vec3(1, 0.2, 1);
		vec3 ambient = vec3(0.1, 0.1, 0.1);

		// pixel location (observer space):
		vec3 v = texture2D(pixelMap, texcoord0).rgb;
		
		vec3 nv = normalize(v);
		
		// eye location (observer space):
		float azimuth = atan(nv.x, -nv.z);
		vec3 eye = vec3(
			cos(azimuth),
			sin(azimuth),
			0
		) * eyesep * (1.-abs(nv.z)) * 0.1;
	
		// ray origin (world space)
		vec3 ro = eye + pos;
		
		// ray direction (world space)
		vec3 nev = normalize(v - eye);
		vec3 rd = quat_rotate(quat, vec3(nev.x, nev.z, -nev.y));
		
		// find object intersection:
		vec3 p = ro;
		
		// initial eye-ray to find object intersection:
		float mindt = 0.01 + 0.0001;
		float mint = mindt;
		float maxt = 2.;
		float t=mint;
		float h = maxt;
		int steps = 0;
		for (t; t<maxt;) {
			h = map(p);
			t += h;
			p = ro + t*rd;
			if (h < mindt) {
				break;
			}
			//t += h * 0.5; //(1.+0.2*t/maxt);	// slight speedup
			//steps++;
		}

		// Q: what is the normal at this point?
		// need normals for 
		// a) lighting calculation
		// b) initial nudge for shadow etc.

		// grain:
		//vec3 grain = texture2D(tex0, gl_TexCoord[0].xy * 20.).grb;
		//normal = normalize( normal + 0.5*(grain-0.5) );

		float test = 0.;

		// lighting:
		vec3 color = vec3(0, 0, 0);

		if (t<maxt) {
			
			// Normals computed by central differences on the distance field at the shading point (gradient approximation).
			// larger eps leads to softer edges
			float eps = 0.1;
			vec3 grad = vec3( 
				map(p+vec3(eps,0,0)) - map(p-vec3(eps,0,0)),
				map(p+vec3(0,eps,0)) - map(p-vec3(0,eps,0)),
				map(p+vec3(0, 0, eps)) - map(p-vec3(0,0,eps))  
			);
			vec3 normal = normalize(grad);

			

			// Ambient Occlusion:
			// sample 5 neighbors in direction of normal
			float ao = 0.;
			float dao = 0.001; // delta between AO samples
			float aok = 2.0;
			float weight = 1.;
			for (int i=0; i<5; i++) {
				float dist = float(i)*dao;
				float factor = dist - map(p + normal*dist);
				ao += weight * factor;
				weight *= 0.6;	// decreasing importance
			}
			ao = 1. - aok * ao;

			// compute ray to light source:
			vec3 ldir1 = normalize(light1 - p);
			vec3 ldir2 = normalize(light2 - p);
			
			// abs for bidirectional surfaces
			float ln1 = max(0.,dot(ldir1, normal));
			float ln2 = max(0.,dot(ldir2, normal));

			// shadow penumbra coefficient:
			float k = 16.;

			// check for shadow:
			float smint = 0.01;
			float nudge = 0.1;
			float smaxt = 12.;
			
			color = //v
					ambient
					+ color1 * ln1 * shadow(p+normal*nudge, ldir1, smint, smaxt, mindt, k) 
					+ color2 * ln2 //* shadow(p+normal*smint, ldir2, smint, smaxt, mindt, k)
					;
			//color = 	ambient +
			//		color1 * ln1 + 
			//		color2 * ln2;
			//test = ao;
			color *= ao;
			//color = normal;
		}

		// render as depth:
		float depth = 1.-t/maxt;
		//color = vec3(depth, depth, depth);

		// render normal:
		//color = normal;

		// brighter pixels needed more iterations:
		//test = 1. - 10./(10.+float(steps));
		//color += test;

		gl_FragColor = vec4(color, 1);
		float a = texture2D(alphaMap, texcoord0).r;
		//gl_FragColor = vec4(test, test, test, 1);
		
		gl_FragColor *= a; //(a+0.5);
	}
);


// this shader does the warp.
static const char * warpVS = AL_STRINGIFY(
	//varying vec2 texcoord0;
	void main(){
		//texcoord0 = vec2(gl_MultiTexCoord0);
		gl_TexCoord[0]=gl_MultiTexCoord0; 
		vec4 vertex = gl_Vertex;
		gl_Position = gl_ModelViewProjectionMatrix * vertex;
	}
);
static const char * warpFS = AL_STRINGIFY(
	uniform sampler2D scene;
	uniform sampler2D geometryMap;
	uniform sampler2D alphaMap;
	//varying vec2 texcoord0;
	float pi = 3.141592653589793;
	void main(){
		vec2 texcoord0 = gl_TexCoord[0].st;
		
		vec2 g = texture2D(geometryMap, texcoord0).xw;
		//g.y = 1.-g.y;
		// x = g.x;, y = g.w
		//g.xy += 0.5*(cos((texcoord0 + 1.) * pi) + 1.);
		
		//g = g*0.0001 + texcoord0;

		vec4 s = texture2D(scene, g);
		vec2 texa = vec2(texcoord0.x, 1.-texcoord0.y);
		float a = texture2D(alphaMap, texa).r;
		gl_FragColor = s * a; //(a+0.5);
		//gl_FragColor = vec4(texcoord0, 0, 1);
	}
);

WarpnBlend::WarpnBlend() {
	loaded = false;
	imgpath = "./img/";
	printf("created WarpnBlend %s\n", imgpath.c_str());
}

void WarpnBlend::onCreate() {
	geomV.source(geomVS, Shader::VERTEX).compile();
	geomF.source(geomFS, Shader::FRAGMENT).compile();
	geomP.attach(geomV).attach(geomF).link();
	
	geomV.printLog();
	geomF.printLog();
	geomP.printLog();
	
	geomV3D.source(geomVS3D, Shader::VERTEX).compile();
	geomF3D.source(geomFS3D, Shader::FRAGMENT).compile();
	geomP3D.attach(geomV3D).attach(geomF3D).link();
	
	geomV3D.printLog();
	geomF3D.printLog();
	geomP3D.printLog();
	
	demoV.source(demoVS, Shader::VERTEX).compile();
	demoF.source(demoFS, Shader::FRAGMENT).compile();
	demoP.attach(demoV).attach(demoF).link();
	
	demoV.printLog();
	demoF.printLog();
	demoP.printLog();
	
	warpV.source(warpVS, Shader::VERTEX).compile();
	warpF.source(warpFS, Shader::FRAGMENT).compile();
	warpP.attach(warpV).attach(warpF).link();
	
	warpV.printLog();
	warpF.printLog();
	warpP.printLog();
	
	geometryMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	geometryMap.filterMag(Texture::LINEAR);
	geometryMap.texelFormat(GL_LUMINANCE_ALPHA32F_ARB);
	geometryMap.dirty();
	
	pixelMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	pixelMap.filterMag(Texture::LINEAR);
	pixelMap.texelFormat(GL_RGB32F_ARB);
	pixelMap.dirty();
	
	alphaMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	alphaMap.filterMag(Texture::LINEAR);
	alphaMap.dirty();
}

void WarpnBlend::draw(Texture& scene) {
	if (!loaded) return;
	gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
	gl.modelView(Matrix4d::identity());
	alphaMap.bind(2);
	geometryMap.bind(1);
	warpP.begin();
	warpP.uniform("scene", 0);
	warpP.uniform("geometryMap", 1);
	warpP.uniform("alphaMap", 2);
	scene.quad(gl);
	warpP.end();
	geometryMap.unbind(1);
	alphaMap.unbind(2);
}

void WarpnBlend::drawWarp() {
	if (!loaded) return;
	gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
	gl.modelView(Matrix4d::identity());
	geomP.begin();
	geomP.uniform("geometryMap", 0);
	geometryMap.quad(gl);
	geomP.end();
}

void WarpnBlend::drawWarp3D() {
	if (!loaded) return;
	gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
	gl.modelView(Matrix4d::identity());
	geomP3D.begin();
	geomP3D.uniform("pixelMap", 0);
	geomP3D.uniform("alphaMap", 1);
	alphaMap.bind(1);
	pixelMap.quad(gl);
	alphaMap.unbind(1);
	geomP3D.end();
}

void WarpnBlend::drawDemo(const Pose& pose, double eyesep) {
	if (!loaded) return;
	gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
	gl.modelView(Matrix4d::identity());
	demoP.begin();
	demoP.uniform("pixelMap", 0);
	demoP.uniform("alphaMap", 1);
	demoP.uniform("pos", pose.pos()); 
	demoP.uniform("quat", pose.quat());
	demoP.uniform("eyesep", eyesep);
	alphaMap.bind(1);
	pixelMap.quad(gl);
	alphaMap.unbind(1);
	demoP.end();
}

void WarpnBlend::drawBlend() {
	if (!loaded) return;
	gl.projection(Matrix4d::ortho(0, 1, 0, 1, -1, 1));
	gl.modelView(Matrix4d::identity());
	alphaMap.quad(gl);
}

void WarpnBlend::readID(std::string id) {
	printf("%s %s\n", imgpath.c_str(), id.c_str());
	readWarp(imgpath + "uv" + id + ".bin");
	read3D(imgpath + "map3D" + id + ".bin");
	readBlend(imgpath + "alpha" + id + ".png");
	readModelView(imgpath + "ModelViewMatrix" + id + ".txt");
	readPerspective(imgpath + "PerspectiveMatrix" + id+ ".txt");
	loaded = true;
}

void WarpnBlend::readBlend(std::string path) {
	Image img(path);
	alphaMap.allocate(img.array(), true);
}

void WarpnBlend::read3D(std::string path) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
	int32_t dim[2];
	f.read((void *)dim, sizeof(int32_t), 2);
	printf("reading map %s: %dx%d; ", path.c_str(), dim[0], dim[1]);
	
	int32_t w = dim[1];
	int32_t h = dim[0]/3;
	int32_t elems = w*h;
	printf("array %dx%d = %d\n", w, h, elems);
	
	int r = 0;	
	
	float * t = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)t, sizeof(float), elems);
	printf("read %d elements (array size %d)\n", r, elems);
	
	float * u = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)u, sizeof(float), elems);
	printf("read %d elements (array size %d)\n", r, elems);
	
	float * v = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)v, sizeof(float), elems);
	printf("read %d elements (array size %d)\n", r, elems);
	
	f.close();
	
	pixelMap.resize(w, h);
	pixelMap.target(Texture::TEXTURE_2D);
	pixelMap.format(Graphics::RGB);
	pixelMap.type(Graphics::FLOAT);
	pixelMap.filterMin(Texture::LINEAR);
	pixelMap.allocate(4);
	pixelMap.print();
	pixelMap.array().print();
	Array& arr = pixelMap.array();
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int32_t idx = y*w+x;	// row-major format
		
			float * cell = arr.cell<float>(x, y);
			cell[0] = t[idx];//*0.5+0.5;
			cell[1] = u[idx];//*0.5+0.5;
			cell[2] = v[idx];//*0.5+0.5;
			
			if (y == h/2) {
				//printf("3D %d,%d = %f, %f, %f\n", x, y, cell[0], cell[1], cell[2]);
			}
		}
	}
	
	free(t);
	free(u);
	free(v);
}

void WarpnBlend::readWarp(std::string path) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
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
	geometryMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	geometryMap.allocate(4);
	geometryMap.print();
	geometryMap.array().print();
	Array& arr = geometryMap.array();
	
	float c0, c1;
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int32_t idx = y*w+x;	// row-major format
		
			float * cell = arr.cell<float>(x, y);
			cell[0] = u[idx]*0.5+0.5;
			cell[1] = v[idx]*0.5+0.5;
			
			
			if (y == h/2) {
				//printf("x %d y %d: u %f v %f\n", x, y, cell[0]-c0, cell[1]-c1);
				c0 = cell[0];
				c1 = cell[1];
			}
		}
	}
	
	free(u);
	free(v);
}

void WarpnBlend::readModelView(std::string path){
	File f(path, "r");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
	const char * src = f.readAll();
	printf("mv %s\n", src);
	f.close();
	
	float p[16];
	sscanf(src, "%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f", 
		&p[0], &p[4], &p[8], &p[12],
		&p[1], &p[5], &p[9], &p[13],
		&p[2], &p[6], &p[10], &p[14],
		&p[3], &p[7], &p[11], &p[15]);
		
	for (int i=0; i<16; i++) {
		modelView[i] = p[i];
	}
	printf("modelView = ");
	modelView.print(stdout);
	printf("\n");
	
}
void WarpnBlend::readPerspective(std::string path, double near, double far){
	File f(path, "r");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
	const char * src = f.readAll();
	printf("mv %s\n", src);
	f.close();
	
	float p[16];
	sscanf(src, "%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f\n%f,%f,%f,%f", 
		&p[0], &p[4], &p[8], &p[12],
		&p[1], &p[5], &p[9], &p[13],
		&p[2], &p[6], &p[10], &p[14],
		&p[3], &p[7], &p[11], &p[15]);
		
	for (int i=0; i<16; i++) {
		perspective[i] = p[i];
	}
	
	// override near/far:
	const double D = far-near;	
	const double D2 = far+near;
	const double fn2 = far*near*2;
	perspective[10] = -D2/D;
	perspective[14] = -fn2/D;
	
	printf("Perspective = ");
	perspective.print(stdout);
	printf("\n");
}
	
void WarpnBlend::rotate(const Matrix4d& r) {
	modelView = modelView * r;
}