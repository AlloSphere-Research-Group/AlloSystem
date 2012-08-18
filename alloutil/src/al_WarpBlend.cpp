#include "alloutil/al_WarpBlend.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/math/al_Functions.hpp"
#include "allocore/system/al_Time.hpp"

using namespace al;

Graphics gl;

// this shader performs Kenny's transformation:
static const char * predistortVS = AL_STRINGIFY(
	uniform vec3 projcoord;
	uniform vec3 normal_unit;
	uniform vec3 x_unit, y_unit;
	uniform float x_pixel, y_pixel;
	uniform float width, height;
	
	varying vec2 debug;
	
	float THRESHOLD = 0.1; // could be larger
	
	void main(){
		// input point in eyespace:
		vec4 P = gl_ModelViewMatrix * gl_Vertex;
		
		vec3 V = P.xyz;
		
		// find input point's distance along the normal vector
		float perpendicular_dist = dot(V - projcoord, normal_unit);
		
		// input point is behind the plane or near the edge
		if (perpendicular_dist < THRESHOLD) {
			V -= normal_unit; // * maybe_slightly_scaled;
			perpendicular_dist = dot(V - projcoord, normal_unit);
			//isValid = false; // deassert flag
		}
		
		float ratio = 1. / perpendicular_dist; // 1 is the distance of uv plane from projcoord
		
		// calculate input point's intersection with uv plane
		vec3 compensated_input = (V-projcoord) * ratio + projcoord;
		
		// calculate xy offset from the center
		float raw_x = dot(compensated_input-projcoord, x_unit);
		float raw_y = dot(compensated_input-projcoord, y_unit);
		
		// calculate xy position
		float x = raw_x / x_pixel + width / 2.;
		float y = raw_y / y_pixel + height / 2.;
		
		debug.x = x / width;
		debug.y = y / height;
		
		// standard pipeline
		gl_Position = gl_ProjectionMatrix * P;
	}
);
static const char * predistortFS = AL_STRINGIFY(
	varying vec2 debug;
	
	void main(){
		gl_FragColor = vec4(debug, 1, 1);
	}	
);


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
		float a = texture2D(alphaMap, vec2(texcoord0.x, 1.-texcoord0.y)).r;
		v = normalize(v);
		v = mod(v * 8., 1.) * a;
		gl_FragColor = vec4(v, 1.);
	}
);

// this shader is just to show the inverse 3D geometry map:
static const char * geomVSI3D = AL_STRINGIFY(
	varying vec2 texcoord0;
	void main(){
		texcoord0 = vec2(gl_MultiTexCoord0);
		vec4 vertex = gl_Vertex;
		gl_Position = gl_ModelViewProjectionMatrix * vertex;
	}
);
static const char * geomFSI3D = AL_STRINGIFY(
	uniform sampler2D inversePixelMap;
	varying vec2 texcoord0;
	void main(){
		vec3 v = texture2D(inversePixelMap, texcoord0).rgb;
		//v = mod(v * 8., 1.);
		v = 0.5 * (v + 1.);
		gl_FragColor = vec4(v, 1.);
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
	uniform vec3 centerpos;
	uniform vec4 centerquat;
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
	
	// pn is the normal to the plane
	vec3 projection_on_plane(vec3 v, vec3 pn) {
		return v - dot(v, pn) * pn;
	}
	
	// SIGNED DISTANCE ESTIMATION FUNCTIONS //

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
	
	float sdTorus( vec3 p, vec2 t ){
		vec2 q = vec2(length(p.xz)-t.x,p.y);
		return length(q)-t.y;
	}
	
	float length8(vec3 p) {
		return pow(pow(p.x,8.) + pow(p.y,8.) + pow(p.z,8.),1./8.);
	}
	float length8(vec2 p) {
		return pow(pow(p.x,8.) + pow(p.y,8.),1./8.);
	}
	
	float sdTorus88( vec3 p, vec2 t ) {
		vec2 q = vec2(length8(p.xz)-t.x,p.y);
		return length8(q)-t.y;
	}
	
	// hexagoanal prism
	float udHexPrism( vec3 p, vec2 h )
	{
		vec3 q = abs(p);
		return max(q.z-h.y,max(q.x+q.y*0.57735,q.y)-h.x);
	}

	// MAIN SCENE //

	float map(vec3 p) {
		//vec3 pr2 = rotateYZ(p, p.x);
		vec3 pr1 = opRepeat(p, vec3(5, 4, 3));
		float s1 = udBox(pr1, vec3(0.4, 0.1, 0.8));
//		float s3 = sdSphere(pr1, 0.3);
//		float s2 = sdTorus(pr1, vec2(0.5, 0.1));
//		float s4 = opSubtract(s3, s1);
//		float s5 = mix(s1, s2, s1-s2);
//		float s6 = s5 + 0.01*sin(p.x * 40.)*sin(p.z * 40.)*sin(p.y * 40.);
		return s1;
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
		vec3 light1 = pos + vec3(1, 2, 3);
		vec3 light2 = pos + vec3(2, -3, 1);
		vec3 color1 = vec3(0.3, 0.7, 0.6);
		vec3 color2 = vec3(0.6, 0.2, 0.8);
		vec3 ambient = vec3(0.1, 0.1, 0.1);

		// pixel location (calibration space):
		vec3 v = texture2D(pixelMap, texcoord0).rgb;
		// ray direction (allosphere space):
		vec3 nv = quat_rotate(centerquat, normalize(v-centerpos));
		// ray direction (world space);
		vec3 rd = quat_rotate(quat, vec3(nv.x, nv.z, -nv.y));
		
		// stereo offset: 
		// should reduce to zero as the nv becomes close to (0, 1, 0)
		// take the vector of nv in the XZ plane
		// and rotate it 90' around Y:
		vec3 up = vec3(0, 1, 0);
		vec3 rdx = projection_on_plane(rd, up);
		vec3 eye = rdx * eyesep * 0.02;
		
		// ray origin (world space)
		vec3 ro = pos + eye;
		
		// initial eye-ray to find object intersection:
		float mindt = 0.01;	// how close to a surface we can get
		float mint = mindt;
		float maxt = 50.;
		float t=mint;
		float h = maxt;
		
		// find object intersection:
		vec3 p = ro + mint*rd;
		
		int steps = 0;
		int maxsteps = 50;
		for (t; t<maxt;) {
			h = map(p);
			t += h;
			p = ro + t*rd;
			if (h < mindt) { break; }
			//if (++steps > maxsteps) { t = maxt; break; }
		}

		// lighting:
		vec3 color = vec3(0, 0, 0);

		if (t<maxt) {

			// Normals computed by central differences on the distance field at the shading point (gradient approximation).
			// larger eps leads to softer edges
			float eps = 0.01;
			vec3 grad = vec3( 
				map(p+vec3(eps,0,0)) - map(p-vec3(eps,0,0)),
				map(p+vec3(0,eps,0)) - map(p-vec3(0,eps,0)),
				map(p+vec3(0, 0, eps)) - map(p-vec3(0,0,eps))  
			);
			vec3 normal = normalize(grad);
			
			// compute ray to light source:
			vec3 ldir1 = normalize(light1 - p);
			vec3 ldir2 = normalize(light2 - p);
			
			// abs for bidirectional surfaces
			float ln1 = max(0.,dot(ldir1, normal));
			float ln2 = max(0.,dot(ldir2, normal));

			// shadow penumbra coefficient:
			float k = 16.;

			// check for shadow:
			float smint = 0.001;
			float nudge = 0.01;
			float smaxt = maxt;
			
			color = ambient
					+ color1 * ln1 //* shadow(p+normal*nudge, ldir1, smint, smaxt, mindt, k) 
					+ color2 * ln2 //* shadow(p+normal*smint, ldir2, smint, smaxt, mindt, k)
					;
	
			//color = 	ambient +
			//		color1 * ln1 + 
			//		color2 * ln2;
			
			/*
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
			color *= ao;
			*/		
			
		
			// fog:
			float tnorm = t/maxt;
			float fog = 1. - tnorm*tnorm;
			//color *= fog;
		}

		vec2 texa = vec2(texcoord0.x, 1.-texcoord0.y);
		float a = texture2D(alphaMap, texa).r;
		
		gl_FragColor = vec4(color, 1) * a; 
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

void WarpnBlend::Projector::print() {
	printf("Projector %d width %d height %d\n", (int)projnum, (int)width, (int)height);
	projcoord.print(); printf(" = projCoord\n");
	normal_unit.print(); printf(" = normal_unit\n");
	x_vec.print(); printf(" = x_vec\n");
	y_vec.print(); printf(" = y_vec\n");
	x_unit.print(); printf(" = x_unit\n");
	y_unit.print(); printf(" = y_unit\n");
	printf("x_dist %f y_dist %f\n", x_dist, y_dist);
	printf("x_pixel %f y_pixel %f\n", x_pixel, y_pixel);

}	

void WarpnBlend::Projector::init() {
	// calculate uv parameters
	x_dist = x_vec.mag();
	x_unit = x_vec / x_dist;
	x_pixel = x_dist / width;
	
	y_dist = y_vec.mag();
	y_unit = y_vec / y_dist;
	y_pixel = y_dist / height;
}	

WarpnBlend::WarpnBlend() {
	loaded = false;
	imgpath = "./img/";
	printf("created WarpnBlend %s\n", imgpath.c_str());
	
	pixelMesh.primitive(gl.POINTS);
	
	testscene.primitive(gl.LINES);
	float step = 0.1;
	for (float x=0; x<1; x+=step) {
	for (float y=0; y<1; y+=step) {
	for (float z=0; z<1; z+=step) {
		testscene.color(x, y, z);
		testscene.vertex(x, y, z);
		testscene.color(x, y, z+step);
		testscene.vertex(x, y, z+step);
		
		testscene.color(y, z, x);
		testscene.vertex(y, z, x);
		testscene.color(y, z+step, x);
		testscene.vertex(y, z+step, x);
		
		testscene.color(z, x, y);
		testscene.vertex(z, x, y);
		testscene.color(z+step, x, y);
		testscene.vertex(z+step, x, y);
	}}}
	testscene.translate(-0.5, -0.5, -0.5);
	testscene.scale(1./step);
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
	
	geomVI3D.source(geomVSI3D, Shader::VERTEX).compile();
	geomFI3D.source(geomFSI3D, Shader::FRAGMENT).compile();
	geomPI3D.attach(geomVI3D).attach(geomFI3D).link();
	
	geomVI3D.printLog();
	geomFI3D.printLog();
	geomPI3D.printLog();
	
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
	
	predistortV.source(predistortVS, Shader::VERTEX).compile();
	predistortF.source(predistortFS, Shader::FRAGMENT).compile();
	predistortP.attach(predistortV).attach(predistortF).link();
	
	predistortV.printLog();
	predistortF.printLog();
	predistortP.printLog();
	
	geometryMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	geometryMap.filterMag(Texture::LINEAR);
	geometryMap.texelFormat(GL_LUMINANCE_ALPHA32F_ARB);
	geometryMap.dirty();
	
	pixelMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	pixelMap.filterMag(Texture::LINEAR);
	pixelMap.texelFormat(GL_RGB32F_ARB);
	pixelMap.dirty();
	
//	inversePixelMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
//	inversePixelMap.filterMag(Texture::LINEAR);
//	inversePixelMap.texelFormat(GL_RGB32F_ARB);
//	inversePixelMap.wrap(Texture::REPEAT);
//	inversePixelMap.dirty();
	
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

void WarpnBlend::drawPreDistortDemo(const Pose& pose, float aspect) {
	if (!loaded) return;
	Graphics gl;
	gl.projection(Matrix4d::perspective(72, aspect, 0.1, 100));
	gl.modelView(Matrix4d::lookAt(pose.pos(), pose.pos() + pose.uf(), pose.uy()));
	
	
	predistortP.begin();
	predistortP.uniform("projcoord", projector.projcoord);
	predistortP.uniform("normal_unit", projector.normal_unit);
	predistortP.uniform("x_unit", projector.x_unit);
	predistortP.uniform("y_unit", projector.y_unit);
	predistortP.uniform("x_pixel", projector.x_pixel);
	predistortP.uniform("y_pixel", projector.y_pixel);
	predistortP.uniform("x_unit", projector.x_unit);
	predistortP.uniform("y_unit", projector.y_unit);
	
	// draw some stuff:
	gl.draw(testscene);
	
	predistortP.end();
}

//void WarpnBlend::drawInverseWarp3D() {
//	if (!loaded) return;
//	gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
//	gl.modelView(Matrix4d::identity());
//	geomPI3D.begin();
//	geomPI3D.uniform("inversePixelMap", 0);
//	inversePixelMap.quad(gl);
//	geomPI3D.end();
//}

void WarpnBlend::drawDemo(const Pose& pose, double eyesep) {
	if (!loaded) return;
	gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
	gl.modelView(Matrix4d::identity());
	demoP.begin();
	demoP.uniform("pixelMap", 0);
	demoP.uniform("alphaMap", 1);
	demoP.uniform("pos", pose.pos()); 
	demoP.uniform("eyesep", eyesep);
	demoP.uniform("quat", pose.quat());
	demoP.uniform("centerpos", center.pos());
	demoP.uniform("centerquat", center.quat());
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
	readProj(imgpath + "proj" + id + ".bin");
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
	//printf("array %dx%d = %d\n", w, h, elems);
	
	int r = 0;	
	
	float * t = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)t, sizeof(float), elems);
	//printf("read %d elements (array size %d)\n", r, elems);
	
	float * u = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)u, sizeof(float), elems);
	//printf("read %d elements (array size %d)\n", r, elems);
	
	float * v = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)v, sizeof(float), elems);
	//printf("read %d elements (array size %d)\n", r, elems);
	
	f.close();
	
	pixelMap.resize(w, h);
	pixelMap.target(Texture::TEXTURE_2D);
	pixelMap.format(Graphics::RGB);
	pixelMap.type(Graphics::FLOAT);
	pixelMap.filterMin(Texture::LINEAR);
	pixelMap.allocate(4);
	pixelMap.print();
	//pixelMap.array().print();
	Array& arr = pixelMap.array();
	
//	inversePixelMap.resize(w, h);
//	inversePixelMap.target(Texture::TEXTURE_2D);
//	inversePixelMap.format(Graphics::RGBA);
//	inversePixelMap.type(Graphics::FLOAT);
//	inversePixelMap.filterMin(Texture::LINEAR);
//	inversePixelMap.allocate(4);
//	inversePixelMap.print();
//	inversePixelMap.array().print();
//	Array& arrinv = inversePixelMap.array();
//	arrinv.setall(0);
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			int32_t idx = y*w+x;	// row-major format
		
			float * cell = arr.cell<float>(x, y);
			// coordinate system change:
			cell[0] = t[idx];//*0.5+0.5;
			cell[1] = v[idx];//*0.5+0.5;
			cell[2] = u[idx];//*0.5+0.5;
			
			if (y == h/2) {
				//printf("3D %d,%d = %f, %f, %f\n", x, y, cell[0], cell[1], cell[2]);
			}
			
			/*
			Vec3f n(cell[0], cell[1], cell[2]);
			// TODO: subtract from center & rotate first...
			n = n.normalize();
			
			// convert to polar:
			float d = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
			float a = atan2(n.z, n.x);	// -pi..pi
			float e = acos(n.y/d);	// 0..pi
			
			if (d >= 0.) {
				
				// normalize azimuth/elevation to texture dimensions:
				int x1 = w * fmod(a + M_2PI, M_2PI) / M_2PI; //((a/M_PI)+0.5) * w;
				int y1 = (e/M_PI) * h;
				
				if (x == w/2) {
					//printf("%d, %d: ", x, y); n.print(); printf("-> %d %d (%f %f %f)\n", x1, y1, a, e, d);
				}
				
				if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
					// TODO: use a blending splat instead:
					float * cellinv = arrinv.cell<float>(x1, y1);
					cellinv[0] = n.x;
					cellinv[1] = n.y;
					cellinv[2] = n.z;
					cellinv[3] = 1.;
				}
			}
			*/
		}
	}
	
	// also write this data into a mesh:
	arr = pixelMap.array();
	for (unsigned y=0; y<arr.height(); y++) {
	for (unsigned x=0; x<arr.width(); x++) {
		Vec3f v(arr.cell<float>(x, y));
		pixelMesh.vertex(v);
		pixelMesh.color(x/float(arr.width()), y/float(arr.height()), 0.);
		pixelMesh.texCoord(x/float(arr.width()), y/float(arr.height()));
	}}
	
	free(t);
	free(u);
	free(v);
}

void WarpnBlend::readProj(std::string path) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open Projector configuration file %s\n", path.c_str());
		return;
	}
	
	f.read((void *)&projector, sizeof(Projector), 1);
	f.close();
	
	projector.init();
	projector.print();
	
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
	//printf("array %dx%d = %d\n", w, h, elems);
	
	int r = 0;	
	
	float * u = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)u, sizeof(float), elems);
	//printf("read %d elements (array size %d)\n", r, elems);
	
	float * v = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)v, sizeof(float), elems);
	//printf("read %d elements (array size %d)\n", r, elems);

	f.close();
	
	geometryMap.resize(w, h);
	geometryMap.target(Texture::TEXTURE_2D);
	geometryMap.format(Graphics::LUMINANCE_ALPHA);
	geometryMap.type(Graphics::FLOAT);
	geometryMap.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	geometryMap.allocate(4);
	geometryMap.print();
	//geometryMap.array().print();
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
	//printf("mv %s\n", src);
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
	//printf("modelView = ");
	//modelView.print(stdout);
	//printf("\n");
	
}
void WarpnBlend::readPerspective(std::string path, double near, double far){
	File f(path, "r");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
	const char * src = f.readAll();
	//printf("mv %s\n", src);
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
	
	//printf("Perspective = ");
	//perspective.print(stdout);
	//printf("\n");
}
	
void WarpnBlend::rotate(const Matrix4d& r) {
	modelView = modelView * r;
}
