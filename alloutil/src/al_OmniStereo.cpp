#include <fstream> // Loads configuration.
#include "json/json.h" // Parses configuration.
#include <iostream>

#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/io/al_File.hpp"
#include "alloutil/al_OmniStereo.hpp"

using namespace al;

static Json::Value config;
static std::string errors;


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
static const char * vGeneric = R"(
	varying vec2 T;
	void main(void) {
		// pass through the texture coordinate (normalized pixel):
		T = vec2(gl_MultiTexCoord0);
		gl_Position = vec4(T*2.-1., 0, 1);
	}
)";

#pragma mark Cube GLSL
static const char * fCube = R"(
	uniform sampler2D pixelMap;
	uniform sampler2D alphaMap;
	uniform samplerCube cubeMap;

	varying vec2 T;

	void main (void){
		// ray location (calibration space):
		vec3 v = normalize(texture2D(pixelMap, T).rgb);

		// index into cubemap:
		vec3 rgb = textureCube(cubeMap, v).rgb * texture2D(alphaMap, T).rgb;

		gl_FragColor = vec4(rgb, 1.);
	}
)";

#pragma mark Sphere GLSL
static const char * fSphere = R"(
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
		vec3 v = normalize(texture2D(pixelMap, T).rgb);

		// ray direction (world space);
		vec3 rd = quat_rotate(quat, v);

		// derive new texture coordinates from polar direction:
		float elevation = acos(-rd.y) * one_over_pi;
		float azimuth = atan(-rd.x, -rd.z) * one_over_pi;
		azimuth = (1. - azimuth)*0.5;	// scale from -1,1 to 1,0
		vec2 sphereT = vec2(azimuth, elevation);

		// read maps:
		vec3 rgb = texture2D(sphereMap, sphereT).rgb * texture2D(alphaMap, T).rgb;
		gl_FragColor = vec4(rgb, 1.);
	}
)";

#pragma mark Warp GLSL
static const char * fWarp = R"(
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
)";

#pragma mark Demo GLSL
static const char * fDemo = R"(
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
		vec3 v = normalize(texture2D(pixelMap, T).rgb);
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
)";

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

	t = u = v = 0;
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

void OmniStereo::Projection::registrationPosition(const Vec3d& pos) {

}

void OmniStereo::Projection::readParameters(std::string path, bool verbose) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open Projector configuration file %s\n", path.c_str());
		return;
	}

	f.read((void *)(&params), sizeof(OmniStereo::Projection::Parameters), 1);
	f.close();

	initParameters(verbose);

	printf("read %s\n", path.c_str());
}

void OmniStereo::Projection::initParameters(bool verbose) {
	// initialize:
	Vec3f v = params.screen_center - params.projector_position;
	float screen_perpendicular_dist = params.normal_unit.dot(v);
	Vec3f compensated_center = (v) / screen_perpendicular_dist + params.projector_position;

	// calculate uv parameters
	float x_dist = params.x_vec.mag();
	x_unit = params.x_vec / x_dist;
	x_pixel = x_dist / params.width;
	x_offset = x_unit.dot(compensated_center - params.projector_position);

	float y_dist = params.y_vec.mag();
	y_unit = params.y_vec / y_dist;
	y_pixel = y_dist / params.height;
	y_offset = y_unit.dot(compensated_center - params.projector_position);

//	if (verbose) {
//		printf("Projector %d width %d height %d\n", (int)params.projnum, (int)params.width, (int)params.height);
//		params.projector_position.print(); printf(" = projector_position\n");
//		params.screen_center.print(); printf(" = screen_center\n");
//		params.normal_unit.print(); printf(" = normal_unit\n");
//		params.x_vec.print(); printf(" = x_vec\n");
//		params.y_vec.print(); printf(" = y_vec\n");
//		x_unit.print(); printf(" = x_unit\n");
//		y_unit.print(); printf(" = y_unit\n");
//		printf("%f = screen_radius\n", params.screen_radius);
//		printf("%f = x_pixel\n", x_pixel);
//		printf("%f = y_pixel\n", y_pixel);
//
//	}
}

void OmniStereo::Projection::readBlend(std::string path) {
	Image img(path);
	mBlend.allocate(img.array(), true);
	printf("read & allocated %s\n", path.c_str());
}

void OmniStereo::Projection::readWarp(std::string path) {
	File f(path, "rb");
	if (!f.open()) {
		printf("failed to open file %s\n", path.c_str());
		exit(-1);
	}

	if (t) free(t);
	if (u) free(u);
	if (v) free(v);

	int32_t dim[2];
	f.read((void *)dim, sizeof(int32_t), 2);

	int32_t w = dim[1];
	int32_t h = dim[0]/3;

	printf("warp dim %dx%d\n", w, h);

	int32_t elems = w*h;
	t = (float *)malloc(sizeof(float) * elems);
	u = (float *)malloc(sizeof(float) * elems);
	v = (float *)malloc(sizeof(float) * elems);

	int r = 0;
	r = f.read((void *)t, sizeof(float), elems);
	r = f.read((void *)u, sizeof(float), elems);
	r = f.read((void *)v, sizeof(float), elems);
	f.close();

	mWarp.resize(w, h)
		.target(Texture::TEXTURE_2D)
		.format(Graphics::RGBA)
		.type(Graphics::FLOAT)
		.filterMin(Texture::LINEAR)
		.allocate();

	updatedWarp();

	printf("read %s\n", path.c_str());
}

void OmniStereo::Projection::updatedWarp() {
	Array& arr = mWarp.array();
	int w = arr.width();
	int h = arr.height();
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			// Y axis appears to be inverted
			int32_t y1 = (h-y-1);
			// input data is row-major format
			int32_t idx = y1*w+x;

			float * cell = arr.cell<float>(x, y);
			Vec3f& out = *(Vec3f *)cell;

//			// coordinate system change?
//			out.x = v[idx];
//			out.y = u[idx];
//			out.z = -t[idx];

            // Matt negates x as an expedient: pablo undoes
			out.x = t[idx];
			out.y = u[idx];
			out.z = v[idx];

			// TODO:
			// out -= mRegistration.pos();
			// // & unrotate by mRegistration.quat()
			// do not normalize; instead capsule fit

			// normalize here so the shaders don't have to
			//out.normalize();

			// fourth element is currently unused:
			cell[3] = 1.;

			if (y == 32 && x == 32) {
				printf("example: %f %f %f -> %f %f %f\n",
					t[idx], u[idx], v[idx],
					cell[0], cell[1], cell[2]);
			}
		}
	}

	mWarp.dirty();

	printf("updated Warp\n");
}

#pragma mark OmniStereo

OmniStereo::OmniStereo(unsigned resolution, bool useMipMaps)
:	mFace(5),
	mEyeParallax(0),
	mSphereRadius(1e10),
	mNear(0.1),
	mFar(100),
	mResolution(resolution),
	mNumProjections(1),
	mFrame(0),
	mMode(MONO),
	mStereo(0),
	mAnaglyphMode(RED_CYAN),
	mMipmap(useMipMaps),
	mFullScreen(false)
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
			p.warp().array().fill(fillFishEye);
			p.warp().dirty();
			break;
		case CYLINDER:
			fovy = f / M_PI;
			aspect = a;
			p.warp().array().fill(fillCylinder);
			p.warp().dirty();
			break;
		default:
			fovy = f / 2.;
			aspect = a;
			p.warp().array().fill(fillRect);
			p.warp().dirty();
			break;
	}
	return *this;
}

OmniStereo& OmniStereo::configure(BlendMode bm) {
	mNumProjections = 1;
	Projection& p = mProjections[0];
	switch (bm) {
		case SOFTEDGE:
			p.blend().array().fill(softEdge);
			p.blend().dirty();
			break;
		default:
			// default blend of 1:
			uint8_t white = 255;
			p.blend().array().set2d(&white);
			p.blend().dirty();
			break;
	}
	return *this;
}

OmniStereo& OmniStereo::configure(std::string configpath, std::string configname) {
	if (configpath == "") {
		#ifdef AL_WINDOWS
		// Calibration files should be kept at the root of the C: drive on Windows.
		configpath = "C:";
		#else
		// A bit of C trickery to get the absolute path of the home directory on *nix systems.
	  FILE *pipe = popen("echo ~", "r");
	  if (pipe) {
	    char c;
	    while((c = getc(pipe)) != EOF) {
	  		if (c == '\r' || c == '\n')
	      	break;
	  		configpath += c;
	    }
	    pclose(pipe);
	  }
	  #endif

	  configpath += "/calibration-current";
	}
	std::string config_file_name = configpath + "/" + configname + ".json";

	static std::ifstream config_file(config_file_name.c_str(), std::ifstream::binary | std::ifstream::in);
	if (config_file.fail()){
		std::cout  << "Failed to find configuration file." << std::endl;
		return *this;
	} else {
		config_file >> config;
		config_file.close();
	}

  if ( config.empty() ){
    std::cout  << "Failed to parse configuration file." << std::endl;
    std::cout << errors << std::endl;
    std::cout << "Using default configuration file." << std::endl;
    return *this;
  } else {
    std::cout << "Parsed configuration file." << std::endl;
  }

	if ( config["active"].asBool() ) {
		mMode = ACTIVE;
	}

	if ( config["fullscreen"].asBool() ) {
		mFullScreen = true;
	}

	if ( config["resolution"].isUInt() ) {
		resolution(config["resolution"].asUInt());
	}

	mNumProjections = config["projections"].size();
	printf("Found %d viewports.\n", mNumProjections);

	for (unsigned i=0; i<mNumProjections; i++) {
		Json::Value& projection = config["projections"][i];
		Projection& mprojection = mProjections[i];

		Json::Value& viewport = projection["viewport"];
			if ( ! viewport.isNull() ) {
				mprojection.viewport().l = viewport["l"].asFloat();
				mprojection.viewport().b = viewport["b"].asFloat();
				mprojection.viewport().w = viewport["w"].asFloat();
				mprojection.viewport().h = viewport["h"].asFloat();
			}

		Json::Value& warp = projection["warp"];
			if ( ! warp.isNull() ) {
				if ( warp["width"].isInt() ) {
					mprojection.warpwidth = warp["width"].asInt();
				}

				if ( warp["height"].isInt() ) {
					mprojection.warpheight = warp["height"].asInt();
				}

				if ( warp["file"].isString() ) {
					mprojection.readWarp( configpath + "/" + warp["file"].asString() );
				}
			}

		Json::Value& blend = projection["blend"];
			if ( ! blend.isNull() ) {
				if ( blend["file"].isString() ) {
					mprojection.readBlend( configpath + "/" + blend["file"].asString() );
				} else {
					// TODO: generate blend...
				}
			}

		// Deprecated.
		// Json::Value& params = projection["params"];
		// 	if ( ! params.isNull() ) {
		// 		if ( params["file"].isString() ) {
		// 			mprojection.readParameters( configpath + "/" + params["file"].asString() );
		// 		}
		// 	}

		Json::Value& position = projection["position"];
			mprojection.position.x = position[0].asDouble();
			mprojection.position.y = position[1].asDouble();
			mprojection.position.z = position[2].asDouble();
	}

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
	gl.error("OmniStereo capture begin");

	Vec3d pos = pose.pos();
	Vec3d ux, uy, uz;
	pose.unitVectors(ux, uy, uz);
	mModelView = Matrix4d::lookAt(ux, uy, uz, pos);

	mNear = lens.near();
	mFar = lens.far();
	const double eyeSep = mStereo ? lens.eyeSep() : 0.;

	gl.projection(Matrix4d::identity());

	// apply camera transform:
	gl.pushMatrix(gl.MODELVIEW);
	gl.loadMatrix(mModelView);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	gl.viewport(0, 0, mResolution, mResolution);

	for (mCurrentEye = 0; mCurrentEye < (mStereo+1); mCurrentEye++) {
		mEyeParallax = eyeSep * (mCurrentEye-0.5);
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
	gl.error("OmniStereo capture end");

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
	gl.error("OmniStereo FBO mipmap end");
}

void OmniStereo::onFrameFront(OmniStereo::Drawable& drawable, const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();

	gl.error("OmniStereo onFrameFront begin");

	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);

		gl.projection(Matrix4d::perspective(lens.fovy(), viewport.w / (float)viewport.h, lens.near(), lens.far()));

		mFace = 5; // draw negative z

		{
			Vec3d pos = pose.pos();
			Vec3d ux, uy, uz;
			pose.unitVectors(ux, uy, uz);
			mModelView = Matrix4d::lookAt(-ux, -uy, uz, pos);

			mNear = lens.near();
			mFar = lens.far();
			//const double eyeSep = mStereo ? lens.eyeSep() : 0.;

			// apply camera transform:
			gl.modelView(mModelView);
			gl.clearColor(mClearColor);
			gl.depthTesting(1);
			gl.depthMask(1);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

			drawable.onDrawOmni(*this);
		}
	}
	gl.error("OmniStereo onFrameFront end");
}

void OmniStereo::drawEye(const Pose& pose, double eye) {
	if (eye > 0.) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[1]);
	} else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, mTex[0]);
	}
	gl.error("OmniStereo drawEye after texture");
	gl.draw(mQuad);
	gl.error("OmniStereo drawEye after quad");
}

void OmniStereo::draw(const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();

	gl.error("OmniStereo draw begin");

	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		p.blend().bind(2);
		p.warp().bind(1);

		gl.error("OmniStereo cube draw begin");

		mCubeProgram.begin();
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_CUBE_MAP);

		gl.error("OmniStereo cube drawStereo begin");

		drawStereo<&OmniStereo::drawEye>(lens, pose, viewport);

		gl.error("OmniStereo cube drawStereo end");

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);

		mCubeProgram.end();
		gl.error("OmniStereo cube draw end");

		p.blend().unbind(2);
		p.warp().unbind(1);
	}
	gl.error("OmniStereo draw end");
}

void OmniStereo::drawQuadEye(const Pose& pose, double eye) {
	gl.draw(mQuad);
}

void OmniStereo::drawSphereMap(Texture& map, const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();

	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		p.blend().bind(2);
		p.warp().bind(1);

		map.bind(0);
		mSphereProgram.begin();
		mSphereProgram.uniform("quat", pose.quat());

		drawQuad();

		mSphereProgram.end();
		map.unbind(0);

		p.blend().unbind(2);
		p.warp().unbind(1);
	}
}

void OmniStereo::drawDemoEye(const Pose& pose, double eye) {
	mDemoProgram.uniform("eyesep", eye);
	mDemoProgram.uniform("pos", pose.pos());
	mDemoProgram.uniform("quat", pose.quat());
	gl.draw(mQuad);
}

void OmniStereo::drawDemo(const Lens& lens, const Pose& pose, const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();

	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		p.blend().bind(2);
		p.warp().bind(1);

		mDemoProgram.begin();

		drawStereo<&OmniStereo::drawDemoEye>(lens, pose, viewport);

		mDemoProgram.end();

		p.blend().unbind(2);
		p.warp().unbind(1);
	}
}

void OmniStereo::drawWarp(const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();

	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		p.blend().bind(2);
		p.warp().bind(1);

		mWarpProgram.begin();

		drawQuad();

		mWarpProgram.end();

		p.blend().unbind(2);
		p.warp().unbind(1);
	}
}

void OmniStereo::drawBlend(const Viewport& vp) {
	mFrame++;
	if (mCubeProgram.id() == 0) onCreate();

	gl.viewport(vp);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	for (int i=0; i<numProjections(); i++) {
		Projection& p = projection(i);
		Viewport& v = p.viewport();
		Viewport viewport(
			vp.l + v.l * vp.w,
			vp.b + v.b * vp.h,
			v.w * vp.w,
			v.h * vp.h
		);
		gl.viewport(viewport);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);

		gl.projection(Matrix4d::ortho(0, 1, 0, 1, -1, 1));
		gl.modelView(Matrix4d::identity());

		p.blend().bind(0);

		drawQuad();

		p.blend().unbind(0);
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


