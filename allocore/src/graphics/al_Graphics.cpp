#include <map>
#include <stdio.h>
#include "allocore/graphics/al_Graphics.hpp"

namespace al{

Graphics::Graphics()
:	mRescaleNormal(0), mInImmediateMode(false)
{
	#ifdef AL_GRAPHICS_USE_PROG_PIPELINE
	for(auto& stack : mMatrixStacks){
		stack.var().emplace(1.f);
	}
	#endif

	for(int i=0; i<AL_MAX_LIGHTS; ++i){
		light(i)
			.strength(0) // disables the light
			.diffuse(al::RGB(i==0?1:0))
			.specular(al::RGB(i==0?1:0))
		;
	}
	mDoLighting = false; // since accessing lights sets true
	Light::globalAmbient(al::RGB(0.2));

	#ifdef AL_GRAPHICS_USE_PROG_PIPELINE
	for(auto& m : mMaterials){
		m.var()
		;
	}
	#endif
}

Graphics::~Graphics(){}

#define CS(t) case Graphics::t: return #t;
const char * toString(Graphics::DataType v){
	switch(v){
		CS(BYTE) CS(UBYTE) CS(SHORT) CS(USHORT) CS(FLOAT)
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		CS(INT) CS(UINT)
		#endif
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		CS(DOUBLE)
		#endif
		default: return "";
	}
}

const char * toString(Graphics::Format v){
	switch(v){
		#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
		CS(DEPTH_COMPONENT)
		#endif
		CS(LUMINANCE) CS(LUMINANCE_ALPHA)
		CS(ALPHA) CS(RGB) CS(RGBA)
		default: return "";
	}
}
#undef CS

int Graphics::numComponents(Format v){
	switch(v){
		case RGBA:				return 4;
		case RGB:				return 3;
		case LUMINANCE_ALPHA:	return 2;
		#ifdef AL_GRAPHICS_SUPPORTS_DEPTH_COMP
		case DEPTH_COMPONENT:
		#endif
		case LUMINANCE:
		case ALPHA:				return 1;
		default:				return 0;
	};
}

int Graphics::numBytes(DataType v){
	#define CS(a,b) case a: return sizeof(b);
	switch(v){
		CS(BYTE, GLbyte)
		CS(UBYTE, GLubyte)
		CS(SHORT, GLshort)
		CS(USHORT, GLushort)
		CS(FLOAT, GLfloat)
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		CS(INT, GLint)
		CS(UINT, GLuint)
		#endif
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		CS(DOUBLE, GLdouble)
		#endif
		default: return 0;
	};
	#undef CS
}

template<> Graphics::DataType Graphics::toDataType<char>(){ return BYTE; }
template<> Graphics::DataType Graphics::toDataType<unsigned char>(){ return UBYTE; }
template<> Graphics::DataType Graphics::toDataType<short>(){ return SHORT; }
template<> Graphics::DataType Graphics::toDataType<unsigned short>(){ return USHORT; }
#ifdef AL_GRAPHICS_SUPPORTS_INT32
template<> Graphics::DataType Graphics::toDataType<int>(){ return INT; }
template<> Graphics::DataType Graphics::toDataType<unsigned int>(){ return UINT; }
#endif
template<> Graphics::DataType Graphics::toDataType<float>(){ return FLOAT; }
#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
template<> Graphics::DataType Graphics::toDataType<double>(){ return DOUBLE; }
#endif

Graphics::DataType Graphics::toDataType(AlloTy v){
	switch(v){
		case AlloFloat32Ty: return FLOAT;
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		case AlloFloat64Ty: return DOUBLE;
		#endif
		case AlloSInt8Ty:	return BYTE;
		case AlloUInt8Ty:	return UBYTE;
		case AlloSInt16Ty:	return SHORT;
		case AlloUInt16Ty:	return USHORT;
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		case AlloSInt32Ty:	return INT;
		case AlloUInt32Ty:	return UINT;
		#endif
		default:			return BYTE;
	}
}

AlloTy Graphics::toAlloTy(Graphics::DataType v) {
	switch(v){
		case BYTE:		return AlloSInt8Ty;
		case UBYTE:		return AlloUInt8Ty;
		case SHORT:		return AlloSInt16Ty;
		case USHORT:	return AlloUInt16Ty;
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
		case INT:		return AlloSInt32Ty;
		case UINT:		return AlloUInt32Ty;
		#endif
		case FLOAT:		return AlloFloat32Ty;
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		case DOUBLE:	return AlloFloat64Ty;
		#endif
		default:		return AlloVoidTy;
	}
}

const char * Graphics::errorString(bool verbose){
	GLenum err = glGetError();
	#define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
	switch(err){
		case GL_NO_ERROR: return "";
		CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
		CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
		CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
		CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
	#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
		CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
	#endif
	#ifdef GL_TABLE_TOO_LARGE
		CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
	#endif
	#ifdef GL_STACK_OVERFLOW
		CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
	#endif
	#ifdef GL_STACK_UNDERFLOW
		CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
	#endif
		default: return "Unknown error code.";
	}
	#undef CS
}

bool Graphics::error(const char * msg, int ID){
	const char * errStr = errorString();
	if(errStr[0]){
		if(ID>=0)	AL_WARN_ONCE("Error %s (id=%d): %s", msg, ID, errStr);
		else		AL_WARN_ONCE("Error %s: %s", msg, errStr);
		return true;
	}
	return false;
}

/*static*/ const std::string& Graphics::extensions(){
	static bool getExts = true;
	static std::string str;
	if(getExts){ // Get extensions string (once)
		str = (const char *)glGetString(GL_EXTENSIONS);
		str += " ";
		getExts = false;
	}
	return str;
}

/*static*/ bool Graphics::extensionSupported(const std::string& name, bool exactMatch){
	static std::map<std::string, bool> extMap;

	// First check for extension in the map...
	const auto searchTerm = name + (exactMatch?" ":"");
	auto it = extMap.find(searchTerm);
	if(it != extMap.end()){
		return it->second;
	}

	// Extension not in map, so search string and cache the result
	else{
		if(extensions().find(searchTerm) != std::string::npos){
			extMap[searchTerm] = true;
			return true;
		} else {
			extMap[searchTerm] = false;
			return false;
		}
	}
}

void Graphics::antialiasing(AntiAliasMode v){
	#ifdef AL_GRAPHICS_SUPPORTS_STROKE_SMOOTH
	glHint(GL_POINT_SMOOTH_HINT, v);
	glHint(GL_LINE_SMOOTH_HINT, v);
	if(FASTEST != v){
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
	} else {
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
	}
	#endif

	#ifdef AL_GRAPHICS_SUPPORTS_POLYGON_SMOOTH
	glHint(GL_POLYGON_SMOOTH_HINT, v);
	if(FASTEST != v){
		glEnable(GL_POLYGON_SMOOTH);
	} else {
		glDisable(GL_POLYGON_SMOOTH);
	}
	#endif
}

/*#ifdef AL_GRAPHICS_USE_FIXED_PIPELINE
void Graphics::fog(float end, float start, const Color& c){
	enable(FOG);
	glFogf(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, start); glFogf(GL_FOG_END, end);
	float fogColor[4] = {c.r, c.g, c.b, c.a};
	glFogfv(GL_FOG_COLOR, fogColor);
}
#else*/
void Graphics::fog(float end, float start, const Color& c){
	//enable(FOG);
	mDoFog = true;
	auto& f = mFog.var();
	f.start = start;
	f.end = end;
	f.scale = 1./(end-start);
	f.color = c;
}
//#endif

Light& Graphics::light(unsigned index){
	if(index >= AL_MAX_LIGHTS) index = AL_MAX_LIGHTS-1;
	mDoLighting = true;
	auto& l = mLights[index].var();
	if(l.strength() == 0) l.strength(1); // turn light on if off
	return l;
}

const Light& Graphics::light(unsigned index) const {
	if(index >= AL_MAX_LIGHTS) index = AL_MAX_LIGHTS-1;
	return mLights[index].get();
}

Material& Graphics::material(){
	mMaterialOneSided = true;
	return mMaterials[0].var().face(FRONT_AND_BACK);
}

const Material& Graphics::material() const {
	return mMaterials[0].get();
}

Material& Graphics::materialFront(){
	mMaterialOneSided = false;
	return mMaterials[0].var().face(FRONT);
}

const Material& Graphics::materialFront() const {
	return mMaterials[0].get();
}

Material& Graphics::materialBack(){
	mMaterialOneSided = false;
	return mMaterials[1].var().face(BACK);
}

const Material& Graphics::materialBack() const {
	return mMaterials[1].get();
}

void Graphics::viewport(int x, int y, int width, int height) {
	glViewport(x, y, width, height);
	enable(SCISSOR_TEST);
	glScissor(x, y, width, height);
}

Viewport Graphics::viewport() const {
	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	return Viewport(vp[0], vp[1], vp[2], vp[3]);
}


// Immediate Mode
void Graphics::begin(Primitive v) {
	// clear buffers
	mMesh.reset();

	// start primitive drawing
	mMesh.primitive(v);
	mInImmediateMode = true;
}

void Graphics::end() {
	draw(mMesh);
	mInImmediateMode = false;
}

void Graphics::vertex(double x, double y, double z) {
	if(mInImmediateMode) {
		// make sure all buffers are the same size if > 0
		mMesh.vertex(x, y, z);
		mMesh.equalizeBuffers();
	}
}

void Graphics::texCoord(double u, double v) {
	if(mInImmediateMode) {
		mMesh.texCoord(u, v);
	}
}

void Graphics::texCoord(double s, double t, double r) {
	if(mInImmediateMode) {
		mMesh.texCoord(s, t, r);
	}
}

void Graphics::normal(double x, double y, double z) {
	if(mInImmediateMode) {
		mMesh.normal(x, y, z);
	}
}

void Graphics::color(double r, double g, double b, double a) {
	if(mInImmediateMode) {
		mMesh.color(r, g, b, a);
	} else {
		currentColor(r, g, b, a);
	}
}

// Buffer drawing
void Graphics::draw(int num_vertices, const Mesh& m){
	draw(m, num_vertices);
}

void Graphics::draw(const Mesh& m, int count, int begin){

	const int Nv = m.vertices().size();
	if(0 == Nv) return; // nothing to draw, so just return...

	const int Ni = m.indices().size();

	const int Nmax = Ni ? Ni : Nv;

	// Adjust negative amounts
	if(count < 0) count += Nmax+1;
	if(begin < 0) begin += Nmax+1;

	// Safety checks...
	//if(count > Nmax) count = Nmax;
	//if(begin+count >= iend) return;

	if(begin >= Nmax) return;	// Begin index past end?
	if(begin + count > Nmax){	// If end index past end, then truncate it
		count = Nmax - begin;
	}

	const int Nc = m.colors().size();
	const int Nci= m.coloris().size();
	const int Nn = m.normals().size();
	const int Nt1= m.texCoord1s().size();
	const int Nt2= m.texCoord2s().size();
	const int Nt3= m.texCoord3s().size();

	//printf("client %d, GPU %d\n", clientSide, gpuSide);
	//printf("Nv %i Nc %i Nn %i Nt2 %i Nt3 %i Ni %i\n", Nv, Nc, Nn, Nt2, Nt3, Ni);

	auto prim = (Graphics::Primitive)m.primitive();

	if(m.stroke() > 0.f){
		switch(prim){
		case LINES: case LINE_STRIP: case LINE_LOOP:
			lineWidth(m.stroke());
			break;
		case POINTS:
			pointSize(m.stroke());
			break;
		default:;
		}
	}

	#ifdef AL_GRAPHICS_USE_PROG_PIPELINE

	if(mCompileShader){
		mCompileShader = false; // will only make one attempt

		mShader.preamble(
		R"(
			precision mediump float; // req'd by ES2
			const float pi = 3.141592653589793;
			varying vec3 pos;		// position (eye space)
			varying vec3 normal;	// normal (eye space)
			varying vec4 color;
			varying vec2 texCoord2;

			struct Fog{
				vec3 color;
				float start, end;
				float scale; // 1/(end-start)
			};
			uniform Fog fog;// = Fog(vec3(0.), 0., 1.);
			varying float fogMix;
		)"
		);

		mShader.compile(
		R"(
			uniform mat4 MV;
			uniform mat4 P;
			uniform mat3 normalMatrix;
			uniform vec4 singleColor;
			uniform bool hasNormals;
			attribute vec3 posIn;
			attribute vec3 normalIn;
			attribute vec4 colorIn;
			attribute vec2 texCoord2In;

			void main(){
				pos = (MV * vec4(posIn,1.)).xyz; // to eye space
				gl_Position = P * vec4(pos,1.); // to screen space
				color = singleColor.a==8192. ? colorIn : singleColor;
				if(hasNormals){
					normal = normalMatrix * normalIn;
				} else {
					normal = vec3(1.,0.,0.);
				}
				texCoord2 = texCoord2In;
				fogMix = (gl_Position.z - fog.start) * fog.scale;
				fogMix = clamp(fogMix, 0., 1.);
			}
		)",

			"#define MAX_LIGHTS " + std::to_string(AL_MAX_LIGHTS) + "\n" +
		R"(
			#ifndef MAX_LIGHTS
			#define MAX_LIGHTS 4
			#endif

			// Light source
			struct Light{
				vec3 pos;			// Position of light
				vec3 dir;			// Direction of cone
				float halfDist;		// Distance at which intensity is 50%
				float spread;		// Spread of cone, in degrees
				float strength;		// Overall strength of light
				vec3 diffuse;		// Component scattered off surface
				vec3 specular;		// Component bounced/reflected off surface
				float ambient;		// Amount of light bounced off walls (uses diffuse color)
			};

			struct LightFall{ // For storing intermediate results
				vec3 diffuse;
				vec3 specular;
			};

			void zero(out LightFall l){
				l.diffuse = vec3(0.);
				l.specular = vec3(0.);
			}

			// Surface material
			struct Material{
				vec3 diffuse;		// Component scattered off surface
				vec3 specular;		// Component bounced/reflected off surface
				vec3 emission;		// Component emitted from surface
				float shininess;	// Concentration of specular (its lack of scattering)
			};

			uniform Light lights[MAX_LIGHTS];
			uniform vec3 globalAmbient;
			uniform Material materials[2];
			uniform bool doLighting;
			uniform bool colorMaterial;
			uniform bool materialOneSided;

			Material lerp(in Material m1, in Material m2, float frac){
				Material m;
				m.diffuse  = mix(m1.diffuse,  m2.diffuse,  frac);
				m.specular = mix(m1.specular, m2.specular, frac);
				m.emission = mix(m1.emission, m2.emission, frac);
				m.shininess= mix(m1.shininess,m2.shininess,frac);
				return m;
			}

			/* OpenGL fixed pipeline lighting (li = light i, m = material):
				Em + Ag Am +
				sum_i{ ali [Ali Am + max(L.N, 0) Dli Dm + max(H.N, 0)^s Sli Sm] }
			*/

			/// Blinn-Phong lighting
			///
			/// @param[in] pos		position on surface
			/// @param[in] N		normal to surface
			/// @param[in] V		direction from surface to eye
			/// @param[in] light	Light structure
			/// \returns light color
			LightFall light(in vec3 pos, in vec3 N, in vec3 V, in Light light, in float shininess){
				// Note: light attenuation over distance is an exponential decay (Beer-Lambert)
				vec3 lightVec = light.pos - pos;
				vec3 L = normalize(lightVec); // dir from surface to light

				float intens = light.strength;

				// Distance attenuation: 1/(1+[d/h]^2) = h^2 / (h^2 + d^2)
				float hh = light.halfDist*light.halfDist;
				intens *= hh / (hh + dot(lightVec,lightVec));

				// Spotlight
				float coneAmt = -dot(light.dir, L); // cos of angle: [1,-1] -> [coincident, opposing]
				//coneAmt = coneAmt*-0.5+0.5; // [1,-1] -> [0,1]
				float cosMax = cos(light.spread * pi/180.);
				//if(coneAmt < cosMax) intens = 0.; // hard cut
				if(cosMax >-0.9999){ // to allow disabling spotlight
					float coneDist = (1.-coneAmt)/(1.-cosMax); // apx dist from cone center [0,1]
					//float coneAmp = 1.-min(coneDist,1.); // linear falloff
					float coneAmp = min(coneDist,1.)-1.; coneAmp*=coneAmp; // parabolic falloff
					intens *= coneAmp;
				}

				// Diffuse/specular
				float diffAmt = dot(N,L) * intens;
				diffAmt = max(diffAmt, 0.); // front lighting
				//diffAmt = abs(diffAmt); // front-and-back lighting
				diffAmt = diffAmt*(1.-light.ambient) + light.ambient; // mix in ambient
				vec3 H = normalize(L + V); // half-vector
				float specAmt = pow(max(dot(N,H), 0.), shininess) * intens;

				LightFall fall;
				fall.diffuse  = light.diffuse  * diffAmt;
				fall.specular = light.specular * specAmt;
				return fall;
			}

			vec3 lightColor(
				in vec3 pos, in vec3 N, in vec3 V,
				in Material material
			){
				LightFall lsum;
				zero(lsum);
				for(int i=0; i<MAX_LIGHTS; ++i){
					if(lights[i].strength != 0.){
						LightFall l = light(pos, N, V, lights[i], material.shininess);
						lsum.diffuse += l.diffuse;
						lsum.specular += l.specular;
					}
				}

				return (lsum.diffuse + globalAmbient) * material.diffuse
					+ lsum.specular * material.specular
					+ material.emission;
			}

			void main(){
				vec3 col = color.rgb;
				if(doLighting){
					vec3 N = normalize(normal);
					vec3 V = normalize(-pos); // surface to eye
					Material m;
					if(gl_FrontFacing || materialOneSided) m = materials[0];
					else m = materials[1];
					if(colorMaterial) m.diffuse *= col;
					col = lightColor(pos, N, V, m);
				}
				col = mix(col, fog.color, fogMix);
				gl_FragColor = vec4(col, color.a);
				//gl_FragColor = vec4(1.,0.,0.,1.); //debug
			}
		)");

		if(mShader.linked()){
			mLocPos       = mShader.attribute("posIn");
			mLocColor     = mShader.attribute("colorIn");
			mLocNormal    = mShader.attribute("normalIn");
			mLocTexCoord2 = mShader.attribute("texCoord2In");
			mMatrixStacks[MODELVIEW].loc() = mShader.uniform("MV");
			mMatrixStacks[PROJECTION].loc() = mShader.uniform("P");
			mDoLighting.loc() = mShader.uniform("doLighting");
			#define SET_LOC(name)\
				o.loc().name = mShader.uniform((pre + #name).c_str());
			for(int i=0; i<AL_MAX_LIGHTS; ++i){
				auto& o = mLights[i];
				std::string pre = "lights[" + std::to_string(i) + "].";
				SET_LOC(pos) SET_LOC(dir) SET_LOC(halfDist) SET_LOC(spread)
				SET_LOC(strength) SET_LOC(diffuse) SET_LOC(specular) SET_LOC(ambient)
			}
			for(int i=0; i<2; ++i){
				auto& o = mMaterials[i];
				std::string pre = "materials[" + std::to_string(i) + "].";
				SET_LOC(diffuse) SET_LOC(emission) SET_LOC(specular) SET_LOC(shininess)
			}
			mMaterialOneSided.loc() = mShader.uniform("materialOneSided");
			mFog.loc().color = mShader.uniform("fog.color");
			mFog.loc().start = mShader.uniform("fog.start");
			mFog.loc().end   = mShader.uniform("fog.end");
			mFog.loc().scale = mShader.uniform("fog.scale");
			// init uniforms
			mShader.begin();
				mShader.uniform("colorMaterial", true);
			mShader.end();
		} else {
			printf("Critical error: al::Graphics failed to compile shader\n");
		}
	}

	//printf("%d %d %d %d\n", mLocPos, mLocColor, mLocNormal, mLocTexCoord2);
	// Return if shader didn't compile
	if(mLocPos < 0) return;

	bool setPointers = true;
	if(mLastDrawnMesh){
		//setPointers = (mLastDrawnMesh == &m);
	}
	mLastDrawnMesh = &m;

	// glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
	glEnableVertexAttribArray(mLocPos);
	if(setPointers) glVertexAttribPointer(mLocPos, 3, GL_FLOAT, 0, 0, &m.vertices()[0]);

	if(Nn >= Nv){
		glEnableVertexAttribArray(mLocNormal);
		if(setPointers) glVertexAttribPointer(mLocNormal, 3, GL_FLOAT, 0, 0, &m.normals()[0]);
	}

	Color singleColor(0,0,0,8192); // if unchanged, triggers read from array

	if(Nc >= Nv){
		glEnableVertexAttribArray(mLocColor);
		if(setPointers) glVertexAttribPointer(mLocColor, 4, GL_FLOAT, 0, 0, &m.colors()[0]);
	}
	else if(Nci >= Nv){
		glEnableVertexAttribArray(mLocColor);
		if(setPointers) glVertexAttribPointer(mLocColor, 4, GL_UNSIGNED_BYTE, 0, 0, &m.coloris()[0]);
	}
	else if(0 == Nc && 0 == Nci){
		singleColor = mCurrentColor;
	}
	else{
		singleColor = Nc ? m.colors()[0] : Color(m.coloris()[0]);
	}

	if(Nt2 >= Nv){
		glEnableVertexAttribArray(mLocTexCoord2);
		if(setPointers) glVertexAttribPointer(mLocTexCoord2, 2, GL_FLOAT, 0, 0, &m.texCoord2s()[0]);
	}

	mShader.begin();
		if(mMatrixStacks[MODELVIEW].handleUpdate()){
			if(Nn){
				// Needed to correctly convert normals into eye space
				mShader.uniform("normalMatrix", normalMatrix(modelView()));
			}
			mShader.uniform(mMatrixStacks[MODELVIEW].loc(), modelView());
		}

		if(mMatrixStacks[PROJECTION].handleUpdate()){
//printf("P\n");
			mShader.uniform(mMatrixStacks[PROJECTION].loc(), projection());
		}

		mShader.uniform("singleColor", singleColor);

		/* Lighting options:
		Eye space:
			- Vertices: multiply by ModelView
			- Normals: multiply by (ModelView^-1)^T matrix
			- Light positions: multiply by View
		World space:
			- Vertices: multiply by Model matrix
			- Normals: multiply by (Model^-1)^T matrix
			- Light positions: nothing
		World space more intuitive, but more work to get model matrix (V^-1 MV)
		and eye pos (V^T -V[3]).
		*/

		if(mDoLighting.handleUpdate()){
//printf("doLighting\n");
			mShader.uniform(mDoLighting.loc(), mDoLighting.get());
		}

		if(mDoLighting == true){
			mShader.uniform("hasNormals", bool(Nn));

			auto xfm = [](const Mat4f& m, const Vec3f& v){
				return (m * Vec4f(v,1.f)).xyz();
			};
			auto viewRot = mView.sub<3>();

			for(const auto& l : mLights){
				bool updateLight = l.handleUpdate();
				bool lightActive = l.get().strength() != 0;
				if(lightActive && (updateLight || mUpdateView)){
//printf("light %p pos\n", l.get());
					mShader.uniform(l.loc().pos, xfm(mView, l.get().pos()));
					mShader.uniform(l.loc().dir, viewRot * l.get().dir());
				}
				if(updateLight){
//printf("light %p rest\n", l.get());
					if(lightActive){
						mShader.uniform(l.loc().halfDist, l.get().halfDist());
						mShader.uniform(l.loc().spread, l.get().spread());
						mShader.uniform(l.loc().diffuse, l.get().diffuse().rgb());
						mShader.uniform(l.loc().specular, l.get().specular().rgb());
						mShader.uniform(l.loc().ambient, l.get().ambient().luminance());
						if(Light::sGlobalAmbientUpdate){
							Light::sGlobalAmbientUpdate = false;
							mShader.uniform("globalAmbient", Light::globalAmbient().rgb());
						}
					}
					mShader.uniform(l.loc().strength, l.get().strength());
				}
			}
		}

		for(const auto& m : mMaterials){
			if(m.handleUpdate()){
				if(mMaterialOneSided.handleUpdate()){
					mShader.uniform(mMaterialOneSided.loc(), mMaterialOneSided.get());
				}
				mShader.uniform(m.loc().diffuse, m.get().diffuse().rgb());
				mShader.uniform(m.loc().emission, m.get().emission().rgb());
				mShader.uniform(m.loc().specular, m.get().specular().rgb());
				mShader.uniform(m.loc().shininess, m.get().shininess());
			}
		}

		if(mFog.handleUpdate()){
//printf("fog update\n");
			if(mDoFog == true){
				mShader.uniform(mFog.loc().color, mFog.get().color);
				mShader.uniform(mFog.loc().start, mFog.get().start);
				mShader.uniform(mFog.loc().end, mFog.get().end);
				mShader.uniform(mFog.loc().scale, mFog.get().scale);
			} else {
				mShader.uniform(mFog.loc().scale, 0.f);
			}
		}

		mUpdateView = false;

		if(Ni){
			// Here, 'count' is the number of indices to render
			glDrawElements(prim, count, GL_UNSIGNED_INT, &m.indices()[begin]);
		}
		else{
			glDrawArrays(prim, begin, count);
		}
	mShader.end();

	glDisableVertexAttribArray(mLocPos);
	if(Nc) glDisableVertexAttribArray(mLocColor);
	if(Nn) glDisableVertexAttribArray(mLocNormal);
	if(Nt2)glDisableVertexAttribArray(mLocTexCoord2);


	//---- FIXED PIPELINE
	#else

	// Note: Nothing submitted to GPU if light strength zero
	for(int i=0; i<AL_MAX_LIGHTS; ++i){
		const auto& l = mLights[i].get();
		auto glID = l.backendID();
		bool lightActive = l.strength() != 0.f;
		if(lightActive){
			l.submitPos(glID);	// Must do this for light to track modelview
		}
		if(mLights[i].handleUpdate()){
			if(lightActive){
				l.submitCol(glID); // will enable GL lighting
			} else {
				glDisable(glID);
			}
		}
	}

	for(const auto& m : mMaterials){
		if(m.handleUpdate()){
			m.get()();
		}
	}

	if(mFog.handleUpdate()){
		enable(FOG);
		glFogf(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, mFog.get().start);
		glFogf(GL_FOG_END, mFog.get().end);
		const auto& col = mFog.get().color;
		float fogColor[4] = {col.r, col.g, col.b, 1.f};
		glFogfv(GL_FOG_COLOR, fogColor);
	}

	// Enable arrays and set pointers...
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, &m.vertices()[0]);

	if(Nn >= Nv){
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, &m.normals()[0]);
	}

	if(Nc >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, &m.colors()[0]);
	}
	else if(Nci >= Nv){
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &m.coloris()[0]);
		//printf("using integer colors\n");
	}
	else if(0 == Nc && 0 == Nci){
		// just use whatever the last glColor() call used!
	}
	else{
		if(Nc)
			glColor4f(m.colors()[0].r, m.colors()[0].g, m.colors()[0].b, m.colors()[0].a);
		else
			glColor4ub(m.coloris()[0].r, m.coloris()[0].g, m.coloris()[0].b, m.coloris()[0].a);
	}

	if(Nt1 >= Nv){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, &m.texCoord1s()[0]);
	}
	else if(Nt2 >= Nv){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &m.texCoord2s()[0]);
	}
	else if(Nt3 >= Nv){
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(3, GL_FLOAT, 0, &m.texCoord3s()[0]);
	}

	// Draw
	if(Ni){
		#ifdef AL_GRAPHICS_SUPPORTS_INT32
			// Here, 'count' is the number of indices to render
			glDrawElements(prim, count, GL_UNSIGNED_INT, &m.indices()[begin]);
		#else
			mIndices16.clear();
			for(int i=begin; i<begin+count; ++i){
				auto idx = m.indices()[i];
				if(idx > 65535) AL_WARN_ONCE("Mesh index value out of range (> 65535)");
				mIndices16.push_back(idx);
			}
			glDrawElements(prim, count, GL_UNSIGNED_SHORT, &mIndices16[0]);
		#endif
	}
	else{
		glDrawArrays(prim, begin, count);
	}

	// Disable arrays
	glDisableClientState(GL_VERTEX_ARRAY);
	if(Nn)					glDisableClientState(GL_NORMAL_ARRAY);
	if(Nc || Nci)			glDisableClientState(GL_COLOR_ARRAY);
	if(Nt1 || Nt2 || Nt3)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	#endif
}

// draw a MeshVBO
void Graphics::draw(MeshVBO& meshVBO) {
	if (!meshVBO.isBound()) meshVBO.bind();

	if (meshVBO.hasIndices()) glDrawElements(meshVBO.primitive(), meshVBO.getNumIndices(), GL_UNSIGNED_INT, NULL);
	else glDrawArrays(meshVBO.primitive(), 0, meshVBO.getNumVertices());

	meshVBO.unbind();
}


void Graphics::onCreate(){
	GPUObject::mID = 1; // must be non-zero to flag creation
	mRescaleNormal = 0;
}

void Graphics::onDestroy(){
}

} // al::
