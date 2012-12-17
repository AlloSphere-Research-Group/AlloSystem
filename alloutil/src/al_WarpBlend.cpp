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

	uniform vec3 projector_position;						// the projector location
	uniform vec3 x_unit, y_unit, normal_unit; 	// the projector coordinate frame
	uniform float aspect, near, far;			// projection rendering parameters
	uniform float uvscalar;						// tweak factor
	uniform vec3 sphere_center;
	uniform float sphere_radius;
	uniform float x_scale, y_scale, x_shift, y_shift;
	
	varying vec3 C; // for debugging
	
	vec4 warp(in vec3 vertex, in vec3 projector_position, in vec3 x_unit, in vec3 y_unit, in vec3 z_unit, in float aspect, in float near, in float far) {
	
		// translate the vertex into sphere-space:
		vec3 vertex_in_sphere = vertex - sphere_center;
		
		// translate the projector into sphere-space:
		vec3 projector_in_sphere = projector_position - sphere_center;
		
		// depth based on the eye-space length:
		float distance = length(vertex_in_sphere);
		
		// find intersection point with sphere surface:
		vec3 surface_intersection = sphere_radius * vertex_in_sphere / distance;
	
		// translate & rotate this into the projector's coordinate frame
		// this will be used to determine the XY position
		mat3 rotmat = mat3(
			x_unit.x, 	y_unit.x, 	z_unit.x, 
			x_unit.y, 	y_unit.y, 	z_unit.y,
			x_unit.z, 	y_unit.z, 	z_unit.z
		);
		vec3 vertex_in_projector = rotmat * (surface_intersection - projector_in_sphere);
		
		// do perspective division on this vertex
		// according to the distance from the projector:
		vec2 uv = vertex_in_projector.xy / vertex_in_projector.z;
		
		// take into account the field of view:
		uv.x *= x_scale;
		uv.y *= y_scale;
		
		// adjust for projector lens shift
		uv.x -= x_shift;
		uv.y -= y_shift;
		
		uv.y *= -1.; // GL is upside down
		
		// reverse the XY position for points behind
		uv.xy *= sign(vertex_in_projector.z);
	
		// depth value should relate to the length of vertex_in_sphere
		// but sign of depth depends on whether the vertex is in front or behind the projection plane
		float depth = -distance * sign(vertex_in_projector.z);
		
		// scale z to the near/far planes for depth-testing / clipping
		// perspective-style depth (divided by -depth for the perspective)
		float z = ((2./depth)*far*near + far+near)/(far-near); 
		
		// assign to output
		return vec4(uv, z, 1);		
	}

	void main() {

		// input point in eyespace:
		vec4 vertex = gl_ModelViewMatrix * gl_Vertex;

		// apply warp:
		vec4 warped = warp(vertex.xyz, projector_position, x_unit, y_unit, normal_unit, aspect, near, far);
		
		gl_Position = warped;
		
		
		// debug coloring
		C = gl_Color.rgb; //abs(gl_Vertex.xyz / 8.);
	}
);
static const char * predistortFS = AL_STRINGIFY(
	varying vec3 C; // for debugging
	
	void main(){
		gl_FragColor = vec4(C, 1);
	}	
);

// this shader is just to show the geometry map:
static const char * alphaVS = AL_STRINGIFY(
	varying vec2 texcoord0;
	void main(){
		texcoord0 = vec2(gl_MultiTexCoord0);
		vec4 vertex = gl_Vertex;
		gl_Position = gl_ModelViewProjectionMatrix * vertex;
	}
);
static const char * alphaFS = AL_STRINGIFY(
	uniform sampler2D alphaMap;
	varying vec2 texcoord0;
	void main(){
		float a = texture2D(alphaMap, texcoord0).r;
		//g.y = 1.-g.y;
		gl_FragColor = vec4(0, 0, 0, 1.-a);
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
		//vec4 vertex = gl_Vertex;
		//gl_Position = gl_ModelViewProjectionMatrix * vertex;
		gl_Position = vec4(texcoord0 * 2.-1., 0., 1.);
	}
);
static const char * geomFS3D = AL_STRINGIFY(
	uniform sampler2D pixelMap, alphaMap;
	varying vec2 texcoord0;
	void main(){
		vec3 v = texture2D(pixelMap, texcoord0).rgb;
		float a = texture2D(alphaMap, texcoord0).r;
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
		//vec4 vertex = gl_Vertex;
		//gl_Position = gl_ModelViewProjectionMatrix * vertex;
		gl_Position = vec4(texcoord0 * 2.-1., 0., 1.);
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
		gl_Position = vec4(texcoord0 * 2.-1., 0., 1.);
		//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
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
		vec3 rd = quat_rotate(quat, nv);
		
		// stereo offset: 
		// should reduce to zero as the nv becomes close to (0, 1, 0)
		// take the vector of nv in the XZ plane
		// and rotate it 90' around Y:
		vec3 up = vec3(0, 1, 0);
		
		vec3 rdx = cross(normalize(rd), up);
		
		//vec3 rdx = projection_on_plane(rd, up);
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

		float a = texture2D(alphaMap, texcoord0).r;
		
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
	projector_position.print(); printf(" = projector_position\n");
	sphere_center.print(); printf(" = sphere_center\n");
	screen_center.print(); printf(" = screen_center\n");
	normal_unit.print(); printf(" = normal_unit\n");
	x_vec.print(); printf(" = x_vec\n");
	y_vec.print(); printf(" = y_vec\n");
	x_unit.print(); printf(" = x_unit\n");
	y_unit.print(); printf(" = y_unit\n");
	printf("%f = screen_radius\n", screen_radius);
	printf("%f = x_pixel\n", x_pixel);
	printf("%f = y_pixel\n", y_pixel);
}	

void WarpnBlend::Projector::init() {

	screen_radius = screen_center.mag();
	screen_center_unit = screen_center / screen_radius;
	
	Vec3f v = sphere_center + screen_center - projector_position;
	float screen_perpendicular_dist = normal_unit.dot(v);
	Vec3f compensated_center = (v) / screen_perpendicular_dist + projector_position;
	
	// calculate uv parameters
	float x_dist = x_vec.mag();
	x_unit = x_vec / x_dist;
	x_pixel = x_dist / width;
	x_offset = x_unit.dot(compensated_center - projector_position);
	
	float y_dist = y_vec.mag();
	y_unit = y_vec / y_dist;
	y_pixel = y_dist / height;
	y_offset = y_unit.dot(compensated_center - projector_position);
}	

WarpnBlend::WarpnBlend() {
	loaded = false;
	imgpath = "./img/";
	printf("created WarpnBlend %s\n", imgpath.c_str());
	
	pixelMesh.primitive(gl.POINTS);
	
	testscene.primitive(gl.LINES);
	float step = 0.02;
	for (int x=0; x<=1; x++) { //x+=step) {
	for (int y=0; y<=1; y++) { //y+=step) {
	for (float z=0; z<(1.-step); z+=step) {
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
	testscene.scale(8.);
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
	
	alphaV.source(alphaVS, Shader::VERTEX).compile();
	alphaF.source(alphaFS, Shader::FRAGMENT).compile();
	alphaP.attach(alphaV).attach(alphaF).link();
	
	alphaV.printLog();
	alphaF.printLog();
	alphaP.printLog();
	
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

void WarpnBlend::drawPreDistortDemo(const Pose& pose, float aspect, double uvscalar, bool blend) {
	if (!loaded) return;
	Graphics gl;
	
	
	//gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
	gl.projection(Matrix4d::perspective(72, aspect, 0.1, 100));
	gl.modelView(Matrix4d::lookAt(pose.pos(), pose.pos() + pose.uf(), pose.uy()));
	
	
	predistortP.begin();
	predistortP.uniform("projector_position", projector.projector_position);
	//predistortP.uniform("projector_position", Vec3f(0.5, 0.5, 0.5));
	predistortP.uniform("normal_unit", projector.normal_unit);
	predistortP.uniform("x_unit", projector.x_unit);
	predistortP.uniform("y_unit", projector.y_unit);
	predistortP.uniform("x_scale", 2./projector.x_vec.mag());
	predistortP.uniform("y_scale", 2./projector.y_vec.mag());
	predistortP.uniform("x_shift", projector.x_offset/projector.width);
	predistortP.uniform("y_shift", projector.y_offset/projector.height);
	predistortP.uniform("near", 0.1f);
	predistortP.uniform("far", 200.f);
	//predistortP.uniform("aspect", aspect);
	//predistortP.uniform("uvscalar", uvscalar);
	predistortP.uniform("sphere_center", projector.sphere_center);
	predistortP.uniform("sphere_radius", projector.screen_radius);
	//predistortP.uniform("alphaMap", 1);
	
	//alphaMap.bind(1);
	
	// draw some stuff:
	gl.draw(testscene);
	
	//alphaMap.unbind(1);
	
	predistortP.end();
	
	if (blend) {
		gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		gl.modelView(Matrix4d::identity());
		
		// now draw the blend mask:
		gl.blending(true);
		gl.blendMode(Graphics::SRC_ALPHA, Graphics::ONE_MINUS_SRC_ALPHA, Graphics::FUNC_ADD);
		gl.depthTesting(false);
		gl.depthMask(false);
		
		alphaP.begin();
		alphaMap.quad(gl);
		alphaP.end();
		
		gl.blending(false);
		gl.depthTesting(true);
		gl.depthMask(true);
	}
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

void alphademo(uint8_t * value, double normx, double normy) {
	//*value = 255.;
	// fade out at edges:
	*value = 255. * al::min(1., 10.*(0.5 - fabs(normx-0.5))) * al::min(1., 10.*(0.5 - fabs(normy-0.5))); 
}

void pixeldemo(float * value, double normx, double normy) {
	// spherical map:
	float az = M_2_PI * (normx - 0.5);
	float el = M_PI * (normy - 0.5);
	
	// is this the right axis convention?
	value[0] = sin(el)*cos(az);
	value[1] = cos(el)*sin(az);
	value[2] = sin(el);
}

void WarpnBlend::readNone() {
	printf("readNone\n");
	Array a(1, AlloUInt8Ty, 64, 64);
	//a.setall<uint8_t>(255);
	uint8_t white = 255;
	a.set2d(&white);
	alphaMap.allocate(a, true);
	//alphaMap.array().fill(alphademo);
	printf("readNone End\n");

//	// generate a blend map:
//	alphaMap.resize(64, 64);
//	alphaMap.target(Texture::TEXTURE_2D)
//			.format(Graphics::RGBA)
//			.type(Graphics::FLOAT)
//			.filterMin(Texture::LINEAR)
//			.allocate(4);
//	//alphaMap.array().fill(alphademo);
//	alphaMap.array().setall(1.f);
//	alphaMap.dirty();
//	alphaMap.print();
//	
//	// generate a map3D:
//	pixelMap.resize(64, 64);
//	pixelMap.target(Texture::TEXTURE_2D);
//	pixelMap.format(Graphics::RGB);
//	pixelMap.type(Graphics::FLOAT);
//	pixelMap.filterMin(Texture::LINEAR);
//	pixelMap.allocate(4);
//	pixelMap.array().fill(pixeldemo);
//	pixelMap.dirty();
//	
//	printf("loaded warpnblend defaults\n");
}

void WarpnBlend::readID(std::string id) {
	printf("%s %s\n", imgpath.c_str(), id.c_str());
	//readWarp(imgpath + "uv" + id + ".bin");
	read3D(imgpath + "map3D" + id + ".bin");
	readBlend(imgpath + "alpha" + id + ".png");
	readModelView(imgpath + "ModelViewMatrix" + id + ".txt");
	readPerspective(imgpath + "PerspectiveMatrix" + id+ ".txt");
	readProj(imgpath + "proj" + id + ".bin");
	loaded = true;
}

void WarpnBlend::readBlend(std::string path) {
	printf("blend:\n");
	
	Image img(path);
	img.array().print();
	alphaMap.allocate(img.array(), true);
	alphaMap.print();
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

	int total = 0;
	float sum = 0;
	
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			// Y axis appears to be inverted
			int32_t y1 = (h-y-1);
			// input data is row-major format
			int32_t idx = y1*w+x;	
		
			float * cell = arr.cell<float>(x, y);
			// coordinate system change:
			cell[0] = t[idx];//*0.5+0.5;
			cell[1] = u[idx];//*0.5+0.5;
			cell[2] = v[idx];//*0.5+0.5;
			
			Vec3f n(cell[0], cell[1], cell[2]);
			float mag = n.mag();
			sum += mag;
			total++;
			
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
	
	float avg = sum / total;
	printf("average radius %f\n", avg);
	
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
