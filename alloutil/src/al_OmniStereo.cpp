
#include "allocore/io/al_Socket.hpp" // for hostname
#include "alloutil/al_Lua.hpp" // for hostname
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/io/al_File.hpp"
#include "alloutil/al_OmniStereo.hpp"

using namespace al;

static Lua L;


static float fovy = M_PI;
static float aspect = 2.;

static void fillFishEye(float * value, double normx, double normy) {
	Vec3f& out = *(Vec3f *)value;
	
	// move (0,0) to center of texture:
	float sx = normx - 0.5;
	float sy = normy - 0.5;
	// azimuth covers full 360':
	float az = fovy * aspect * sx;
	// elevation covers 180':
	float el = fovy * sy;
	
	float sel = sin(el);
	float cel = cos(el);
	float saz = sin(az);
	float caz = cos(az);
	
	// assumes standard OpenGL coordinate frame
	// -Z forward, Y up, X right
	out.x =  cel*saz;
	out.y =  sel;
	out.z = -cel*caz;
	out.normalize();
	
	value[3] = 1.;	
}

static void fillCylinder(float * value, double normx, double normy) {
	Vec3f& out = *(Vec3f *)value;
	
	// move (0,0) to center of texture:
	float sx = normx - 0.5;
	float sy = normy - 0.5;
	
	float y1 = sy * fovy * 2.;
	float y0 = 1. - fabs(y1);
	
	// azimuth covers full 360':
	float az = fovy * M_PI * aspect * sx;
	float saz = sin(az);
	float caz = cos(az);
	
	// assumes standard OpenGL coordinate frame
	// -Z forward, Y up, X right
	out.x =  y0*saz;
	out.y =  y1;
	out.z = -y0*caz;
	out.normalize();
	
	value[3] = 1.;	
}

static void fillRect(float * value, double normx, double normy) {
	Vec3f& out = *(Vec3f *)value;
	
	// move (0,0) to center of texture:
	float sx = normx - 0.5;
	float sy = normy - 0.5;
	
	float f = 1./tan(fovy * 0.5);
	
	// assumes standard OpenGL coordinate frame
	// -Z forward, Y up, X right
	out.x =  f * sx * aspect;
	out.y =  f * sy;
	out.z = -1.;
	out.normalize();
	
	value[3] = 1.;	
}

static void softEdge(uint8_t * value, double normx, double normy) {
	static const double mult = 20;
	
	// fade out at edges:
	value[0] = 255. * sin(M_PI_2 * al::min(1., mult*(0.5 - fabs(normx-0.5)))) * sin(M_PI_2 * al::min(1., mult*(0.5 - fabs(normy-0.5)))); 
}

#pragma mark GLSL
static const char * vGeneric = AL_STRINGIFY(
	varying vec2 T;
	void main(void) {
		// pass through the texture coordinate (normalized pixel):
		T = vec2(gl_MultiTexCoord0);
		gl_Position = vec4(T*2.-1., 0, 1);
	}
);

#pragma mark Cube GLSL
static const char * fCube = AL_STRINGIFY(
	uniform sampler2D pixelMap;
	uniform sampler2D alphaMap;
	uniform samplerCube cubeMap;
	
	varying vec2 T;
	
	void main (void){
		// ray location (calibration space):
		// this should be already normalized in the texture
		vec3 v = texture2D(pixelMap, T).rgb;
		
		// index into cubemap:
		vec3 rgb = textureCube(cubeMap, v).rgb * texture2D(alphaMap, T).rgb;
		
		gl_FragColor = vec4(rgb, 1.);
	}
);

#pragma mark Sphere GLSL
static const char * fSphere = AL_STRINGIFY(	
	uniform sampler2D pixelMap;
	uniform sampler2D alphaMap;
	uniform sampler2D sphereMap;	
	// navigation:
	//uniform vec3 pos;
	uniform vec4 quat;	
	varying vec2 T;
	
	float one_over_pi = 0.318309886183791;
	
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
	
	void main (void){
		// ray location (calibration space):
		// this should be already normalized in the texture
		vec3 v = texture2D(pixelMap, T).rgb;
		
		// ray direction (world space);
		vec3 rd = quat_rotate(quat, v);

		// derive new texture coordinates from polar direction:
		float elevation = acos(rd.y) * one_over_pi;
		float azimuth = -atan(rd.z, rd.x) * one_over_pi;
		azimuth = (azimuth + 1.)*0.5;	// scale from -1,1 to 0,1
		vec2 sphereT = vec2(azimuth, elevation);
		
		// read maps:
		vec3 rgb = texture2D(sphereMap, sphereT).rgb * texture2D(alphaMap, T).rgb;
		gl_FragColor = vec4(rgb, 1.);
	}
);

#pragma mark Warp GLSL
static const char * fWarp = AL_STRINGIFY(
	uniform sampler2D pixelMap;
	uniform sampler2D alphaMap;
	varying vec2 T;
	void main (void){
		vec3 v = texture2D(pixelMap, T).rgb;
		v = normalize(v);
		v = mod(v * 8., 1.);
		v *= texture2D(alphaMap, T).rgb;
		gl_FragColor = vec4(v, 1.);
	}
);

#pragma mark Demo GLSL
static const char * fDemo = AL_STRINGIFY(
	uniform sampler2D pixelMap;
	uniform sampler2D alphaMap;
	uniform vec4 quat;
	uniform vec3 pos;
	uniform float eyesep;
	varying vec2 T;
	
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

	// repetition:
	vec3 opRepeat( vec3 p, vec3 c ) {
		vec3 q = mod(p,c)-0.5*c;
		return q;
	}

	// distance of p from a box of dim b:
	float udBox( vec3 p, vec3 b ) {
	  return length(max(abs(p)-b, 0.0));
	}
	
	// MAIN SCENE //
	float map(vec3 p) {
		vec3 pr1 = opRepeat(p, vec3(5, 4, 3));
		float s1 = udBox(pr1, vec3(0.4, 0.1, 0.8));
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
		vec3 v = texture2D(pixelMap, T).rgb;
		// ray direction (world space);
		vec3 rd = quat_rotate(quat, v);
		
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

		color *= texture2D(alphaMap, T).rgb;
		
		gl_FragColor = vec4(color, 1); 
	}
);

#pragma mark Projection

OmniStereo::Projection::Projection()
:	mViewport(0, 0, 1, 1) {
	
	// allocate blend map:
	mBlend.resize(128, 128)
		.target(Texture::TEXTURE_2D)
		.format(Graphics::LUMINANCE)
		.type(Graphics::UBYTE)
		.filterMin(Texture::LINEAR)
		.allocate();
		

	// allocate warp map:
	mWarp.resize(256, 256)
		.target(Texture::TEXTURE_2D)
		.format(Graphics::RGBA)
		.type(Graphics::FLOAT)
		.filterMin(Texture::LINEAR)
		.allocate();
}

void OmniStereo::Projection::onCreate() {
	mWarp.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	mWarp.filterMag(Texture::LINEAR);
	mWarp.texelFormat(GL_RGB32F_ARB);
	mWarp.dirty();
	
	mBlend.filterMin(Texture::LINEAR_MIPMAP_LINEAR);
	mBlend.filterMag(Texture::LINEAR);
	mBlend.dirty();
}

void OmniStereo::Projection::readBlend(std::string path) {
	Image img(path);
	mBlend.allocate(img.array(), true);
}

void OmniStereo::Projection::readWarp(std::string path) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}
	
	int32_t dim[2];
	f.read((void *)dim, sizeof(int32_t), 2);
	
	int32_t w = dim[1];
	int32_t h = dim[0]/3;
	int32_t elems = w*h;
	
	int r = 0;	
	float * t = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)t, sizeof(float), elems);
	float * u = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)u, sizeof(float), elems);
	float * v = (float *)malloc(sizeof(float) * elems);
	r = f.read((void *)v, sizeof(float), elems);
	f.close();
	
	mWarp.resize(w, h)
		.target(Texture::TEXTURE_2D)
		.format(Graphics::RGBA)
		.type(Graphics::FLOAT)
		.filterMin(Texture::LINEAR)
		.allocate();
	
	Array& arr = mWarp.array();
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			// Y axis appears to be inverted
			int32_t y1 = (h-y-1);
			// input data is row-major format
			int32_t idx = y1*w+x;	
		
			float * cell = arr.cell<float>(x, y);
			Vec3f& out = *(Vec3f *)cell;
			
			// coordinate system change?
			out.x = v[idx];
			out.y = u[idx];
			out.z = -t[idx];
			
			// normalize here so the shaders don't have to
			out.normalize();
			
			// fourth element is currently unused:
			cell[3] = 1.;
		}
	}
	
	free(t);
	free(u);
	free(v);
}	

#pragma mark OmniStereo

OmniStereo::OmniStereo(unsigned resolution, bool useMipMaps)
:	mFace(5),
	mEyeParallax(0),
	mNear(0.1),
	mFar(100),
	mResolution(resolution),
	mNumProjections(1),
	mFrame(0),
	mMode(MONO),
	mStereo(0),
	mAnaglyphMode(RED_CYAN),
	mMipmap(useMipMaps)
{
	mFbo = mRbo = 0;
	mTex[0] = mTex[1] = 0;
	
	mQuad.reset();
	mQuad.primitive(gl.TRIANGLE_STRIP);
	mQuad.texCoord	( 0, 0);
	mQuad.vertex	( 0, 0, 0);
	mQuad.texCoord	( 1, 0);
	mQuad.vertex	( 1, 0, 0);
	mQuad.texCoord	( 0, 1);
	mQuad.vertex	( 0, 1, 0);
	mQuad.texCoord	( 1, 1);
	mQuad.vertex	( 1, 1, 0);
	
	mClearColor.set(0.);
	
	configure(FISHEYE).configure(SOFTEDGE);
}

OmniStereo& OmniStereo::resolution(unsigned r) {
	mResolution = r;
	// force GPU reallocation:	
	mFbo = mRbo = 0;
	mTex[0] = mTex[1] = 0;
	return *this;
	
}

OmniStereo& OmniStereo::configure(WarpMode wm, float a, float f) {
	mNumProjections = 1;
	Projection& p = mProjections[0];
	switch (wm) {
		case FISHEYE:
			fovy = f;
			aspect = a;
			p.mWarp.array().fill(fillFishEye);
			p.mWarp.dirty();
			break;
		case CYLINDER:
			fovy = f / M_PI;
			aspect = a;
			p.mWarp.array().fill(fillCylinder);
			p.mWarp.dirty();
			break;
		default:
			fovy = f / 2.;
			aspect = a;
			p.mWarp.array().fill(fillRect);
			p.mWarp.dirty();
			break;
	}
	return *this;
}

OmniStereo& OmniStereo::configure(BlendMode bm) {
	mNumProjections = 1;
	Projection& p = mProjections[0];
	switch (bm) {
		case SOFTEDGE:
			p.mBlend.array().fill(softEdge);
			p.mBlend.dirty();
			break;
		default:
			// default blend of 1:	
			uint8_t white = 255;
			p.mBlend.array().set2d(&white);
			p.mBlend.dirty();
			break;
	}
	return *this;
}

OmniStereo& OmniStereo::configure(std::string configpath, std::string configname) {
	if (L.dofile(configpath + "/" + configname + ".lua", 0)) return *this;
	
	L.getglobal("projections");
	if (!lua_istable(L, -1)) {
		printf("config file %s has no projections\n", configpath.c_str());
		return *this;
	}
	int projections = L.top();
	
	mNumProjections = lua_objlen(L, -1);
	printf("found %d viewports\n", mNumProjections);
	
	for (unsigned i=0; i<mNumProjections; i++) {
		L.push(i+1);
		lua_gettable(L, projections);
		int projection = L.top();
		//L.dump("config");
		
		lua_getfield(L, projection, "viewport");
		if (lua_istable(L, -1)) {
			int viewport = L.top();
			lua_getfield(L, viewport, "l");
			mProjections[i].mViewport.l = L.to<float>(-1);
			L.pop();
			
			lua_getfield(L, viewport, "b");
			mProjections[i].mViewport.b = L.to<float>(-1);
			L.pop();
			
			lua_getfield(L, viewport, "w");
			mProjections[i].mViewport.w = L.to<float>(-1);
			L.pop();
			
			lua_getfield(L, viewport, "h");
			mProjections[i].mViewport.h = L.to<float>(-1);
			L.pop();
			
		}
		L.pop(); // viewport
		
		lua_getfield(L, projection, "warp");
		if (lua_istable(L, -1)) {
			int warp = L.top();
			lua_getfield(L, warp, "file");
			if (lua_isstring(L, -1)) {
				// load from file
				mProjections[i].readWarp(configpath + "/" + lua_tostring(L, -1));
			}
			//L.dump("warp");
			L.pop();
		}
		L.pop(); // warp
		
		lua_getfield(L, projection, "blend");
		if (lua_istable(L, -1)) {
			int blend = L.top();
			lua_getfield(L, blend, "file");
			if (lua_isstring(L, -1)) {
				// load from file
				mProjections[i].readBlend(configpath + "/" + lua_tostring(L, -1));
			} else {
				// TODO: generate blend...
			}
			L.pop();
		}
		L.pop(); // blend
		
		L.pop(); // projector
	}
	
	L.pop(); // the projections table
	return *this;
}

void OmniStereo::onCreate() {
	
	// force allocation of warp/blend textures:
	for (unsigned i=0; i<4; i++) {
		mProjections[i].onCreate();
	}

	Shader cubeV, cubeF;
	cubeV.source(vGeneric, Shader::VERTEX).compile();
	cubeF.source(fCube, Shader::FRAGMENT).compile();
	mCubeProgram.attach(cubeV).attach(cubeF);
	mCubeProgram.link(false);	// false means do not validate
	// set uniforms before validating to prevent validation error
	mCubeProgram.begin();
		mCubeProgram.uniform("alphaMap", 2);
		mCubeProgram.uniform("pixelMap", 1);
		mCubeProgram.uniform("cubeMap", 0);
	mCubeProgram.end();
	mCubeProgram.validate();
	cubeV.printLog();
	cubeF.printLog();
	mCubeProgram.printLog();
	Graphics::error("cube program onCreate");

	Shader sphereV, sphereF;
	sphereV.source(vGeneric, Shader::VERTEX).compile();
	sphereF.source(fSphere, Shader::FRAGMENT).compile();
	mSphereProgram.attach(sphereV).attach(sphereF);
	mSphereProgram.link(false);	// false means do not validate
	// set uniforms before validating to prevent validation error
	mSphereProgram.begin();
		mSphereProgram.uniform("alphaMap", 2);
		mSphereProgram.uniform("pixelMap", 1);
		mSphereProgram.uniform("sphereMap", 0);
	mSphereProgram.end();
	mSphereProgram.validate();
	sphereV.printLog();
	sphereF.printLog();
	mSphereProgram.printLog();
	Graphics::error("cube program onCreate");

	Shader warpV, warpF;
	warpV.source(vGeneric, Shader::VERTEX).compile();
	warpF.source(fWarp, Shader::FRAGMENT).compile();
	mWarpProgram.attach(warpV).attach(warpF);
	mWarpProgram.link(false);	// false means do not validate
	// set uniforms before validating to prevent validation error
	mWarpProgram.begin();
		mWarpProgram.uniform("alphaMap", 2);
		mWarpProgram.uniform("pixelMap", 1);
	mWarpProgram.end();
	mWarpProgram.validate();
	warpV.printLog();
	warpF.printLog();
	mWarpProgram.printLog();
	Graphics::error("cube program onCreate");

	Shader demoV, demoF;
	demoV.source(vGeneric, Shader::VERTEX).compile();
	demoF.source(fDemo, Shader::FRAGMENT).compile();
	mDemoProgram.attach(demoV).attach(demoF);
	mDemoProgram.link(false);	// false means do not validate
	// set uniforms before validating to prevent validation error
	mDemoProgram.begin();
		mDemoProgram.uniform("alphaMap", 2);
		mDemoProgram.uniform("pixelMap", 1);
	mDemoProgram.end();
	mDemoProgram.validate();
	demoV.printLog();
	demoF.printLog();
	mDemoProgram.printLog();
	Graphics::error("cube program onCreate");
	
	// create cubemap textures:
	glGenTextures(2, mTex);
	for (int i=0; i<2; i++) {
		// create cubemap texture:
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[i]);
		
		// each cube face should clamp at texture edges:
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		// filtering
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		// TODO: verify? 
		// Domagoj also has:
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
		float X[4] = { 1,0,0,0 };
		float Y[4] = { 0,1,0,0 };
		float Z[4] = { 0,0,1,0 };
		glTexGenfv( GL_S, GL_OBJECT_PLANE, X );
		glTexGenfv( GL_T, GL_OBJECT_PLANE, Y );
		glTexGenfv( GL_R, GL_OBJECT_PLANE, Z );

		// RGBA8 Cubemap texture, 24 bit depth texture, mResolution x mResolution
		// NULL means reserve texture memory, but texels are undefined
		for (int f=0; f<6; f++) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X+f, 
				0, GL_RGBA8, 
				mResolution, mResolution, 
				0, GL_BGRA, GL_UNSIGNED_BYTE, 
				NULL);
		}
		
		// clean up:
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		Graphics::error("creating cubemap texture");
	}
	
	// one FBO to rule them all...
	glGenFramebuffers(1, &mFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	//Attach one of the faces of the Cubemap texture to this FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, mTex[0], 0);
	
	glGenRenderbuffers(1, &mRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mResolution, mResolution);
	// Attach depth buffer to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRbo);
	
	// ...and in the darkness bind them:
	for (mFace=0; mFace<6; mFace++) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+mFace, GL_TEXTURE_CUBE_MAP_POSITIVE_X+mFace, mTex[0], 0);
	}
	
	//Does the GPU support current FBO configuration?
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("GPU does not support required FBO configuration\n");
		exit(0);
	}
	
	// cleanup:
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	Graphics::error("OmniStereo onCreate");
}

void OmniStereo::onDestroy() {
	mCubeProgram.destroy();

	glDeleteTextures(2, mTex);
	mTex[0] = mTex[1] = 0;
	
	glDeleteRenderbuffers(1, &mRbo);
	glDeleteFramebuffers(1, &mFbo);
	mRbo = mFbo = 0;
}

void OmniStereo::capture(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose) {
	if (mCubeProgram.id() == 0) onCreate();
	
	Vec3d pos = pose.pos();
	Vec3d ux, uy, uz; 
	pose.unitVectors(ux, uy, uz);
	mModelView = Matrix4d::lookAt(ux, uy, uz, pos);
	
	mNear = lens.near();
	mFar = lens.far();
	const double eyeSep = mStereo ? lens.eyeSep() : 0.;
	
	// apply camera transform:
	gl.pushMatrix(gl.MODELVIEW);
	gl.loadMatrix(mModelView);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	gl.viewport(0, 0, mResolution, mResolution);
	
	for (int i=0; i<(mStereo+1); i++) {
		mEyeParallax = eyeSep * (i-0.5);
		for (mFace=0; mFace<6; mFace++) {
			
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + mFace);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0 + mFace, 
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + mFace, 
				mTex[i], 0);
			
			gl.clearColor(mClearColor);
			gl.depthTesting(1);
			gl.depthMask(1);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			drawable.onDrawOmni(*this);		
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();
	gl.popMatrix(gl.MODELVIEW);
	
	// FBOs don't generate mipmaps by default; do it here:
	if (mMipmap) {
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_CUBE_MAP);
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[0]);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		gl.error("generating mipmap");
		
		if (mStereo) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[1]);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			gl.error("generating mipmap");
		}
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
}	

void OmniStereo::drawEye(double eye) {
	glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[eye > 0.]);
	gl.draw(mQuad);
}

void OmniStereo::draw(const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();
	
	// TODO: which loop is more expensive?
	// which GL state changes are more expensive?
	for (int i=mNumProjections-1; i>=0; i--) {
		Projection& p = mProjections[i];
		Texture& warp = p.mWarp;
		Texture& blend = p.mBlend;
		Viewport& v = p.mViewport;
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		mCubeProgram.begin();
		blend.bind(2);
		warp.bind(1);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_CUBE_MAP);
		
		drawStereo<&OmniStereo::drawEye>(lens, pose, viewport);		
	
		//switch (mMode) {
//			case SEQUENTIAL:
//				// frame sequential:
//				glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[sequential_frame]);
//				gl.draw(mQuad);
//				break;
//				
//			case ACTIVE:
//				glDrawBuffer(GL_BACK_RIGHT);
//				glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[1]);
//				gl.draw(mQuad);
//				
//				glDrawBuffer(GL_BACK_LEFT);
//				glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[0]);
//				gl.draw(mQuad);
//				
//				glDrawBuffer(GL_BACK);
//				break;
//			
//			case DUAL:
//				// TODO:
//				break;
//				
//			case ANAGLYPH:
//				// TODO:
//				break;
//			
//			case RIGHT_EYE:
//				glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[1]);
//				gl.draw(mQuad);
//				break;
//			
//			case LEFT_EYE:
//			case MONO:
//			default:
//				glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[0]);
//				gl.draw(mQuad);
//				break;
//		}
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
		
		blend.unbind(2);
		warp.unbind(1);
		mCubeProgram.end();
		
	}
}

void OmniStereo::drawQuadEye(double eye) {
	gl.draw(mQuad);
}

void OmniStereo::drawSphereMap(Texture& map, const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();
	
	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (unsigned i=0; i<mNumProjections; i++) {
		Projection& p = mProjections[i];
		Texture& warp = p.mWarp;
		Texture& blend = p.mBlend;
		Viewport& v = p.mViewport;
		gl.viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		mSphereProgram.begin();
		mSphereProgram.uniform("quat", pose.quat());
		
		blend.bind(2);
		warp.bind(1);
		map.bind(0);
		
		drawQuad();
			
		blend.unbind(2);
		warp.unbind(1);
		map.unbind(0);
		mSphereProgram.end();
	}
}

void OmniStereo::drawDemoEye(double eye) {
	mDemoProgram.uniform("eyesep", eye);
	gl.draw(mQuad);
}

void OmniStereo::drawDemo(const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();
	
	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (unsigned i=0; i<mNumProjections; i++) {
		Projection& p = mProjections[i];
		Texture& warp = p.mWarp;
		Texture& blend = p.mBlend;
		Viewport& v = p.mViewport;
		
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		// TODO: draw according to stereo mode:
		mDemoProgram.begin();
		blend.bind(2);
		warp.bind(1);
		
		mDemoProgram.uniform("pos", pose.pos()); 
		mDemoProgram.uniform("quat", pose.quat());
		
		drawStereo<&OmniStereo::drawDemoEye>(lens, pose, viewport);	
		
		blend.unbind(2);
		warp.unbind(1);
		mDemoProgram.end();
	}
}

void OmniStereo::drawWarp(const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();
	
	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (unsigned i=0; i<mNumProjections; i++) {
		Projection& p = mProjections[i];
		Texture& warp = p.mWarp;
		Texture& blend = p.mBlend;
		Viewport& v = p.mViewport;
		gl.viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		mWarpProgram.begin();
		blend.bind(2);
		warp.bind(1);
		
		drawQuad();
		
		blend.unbind(2);
		warp.unbind(1);
		mWarpProgram.end();
	}
}

void OmniStereo::drawBlend(const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();
	
	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (unsigned i=0; i<mNumProjections; i++) {
		Projection& p = mProjections[i];
		Texture& blend = p.mBlend;
		Viewport& v = p.mViewport;
		gl.viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		gl.projection(Matrix4d::ortho(0, 1, 0, 1, -1, 1));
		gl.modelView(Matrix4d::identity());
		
		blend.bind(0);
		drawQuad();
		blend.unbind(0);
	}
}

void OmniStereo::drawQuad() {
	switch (mMode) {
		case ACTIVE:
			glDrawBuffer(GL_BACK_RIGHT);
			gl.draw(mQuad);
			
			glDrawBuffer(GL_BACK_LEFT);
			gl.draw(mQuad);
			
			glDrawBuffer(GL_BACK);
			break;
		
		case DUAL:
			// TODO:
			break;
		
		default:
			gl.draw(mQuad);
			break;
	}
}


