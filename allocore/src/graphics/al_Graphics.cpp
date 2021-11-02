#include <algorithm> // max
#include <map>
#include <memory> // unique_ptr
#include <stdio.h>
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_BufferObject.hpp"

#ifdef AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
	#include <stack> // matrix stack
	#include "allocore/graphics/al_Shader.hpp"
	#ifndef AL_MODELVIEW_STACK_SIZE
		#define AL_MODELVIEW_STACK_SIZE 8
	#endif
	#ifndef AL_PROJECTION_STACK_SIZE
		#define AL_PROJECTION_STACK_SIZE 4
	#endif
#endif

namespace al{

#define DRAW_BEGIN\
	if(0==m.count || !prepareDraw()) return;\
	\
	if(m.stroke > 0.f){\
		switch((Graphics::Primitive)m.primitive){\
		case LINES: case LINE_STRIP: case LINE_LOOP:\
			mGraphics.lineWidth(m.stroke);\
			break;\
		case POINTS:\
			mGraphics.pointSize(m.stroke);\
			break;\
		default:;\
		}\
	}


struct Graphics::RawMeshData{
	int primitive = 0;
	float stroke = 1;
	int count=0, begin=0;
	int Nv=0, Nn=0, Nc=0, Nci=0, Nt1=0, Nt2=0, Nt3=0, Ni=0;
	const char	* vertices = NULL, * normals = NULL,
				* colors = NULL, * coloris = NULL,
				* texCoord1s = NULL, * texCoord2s = NULL, * texCoord3s = NULL,
				* indices = NULL;

	RawMeshData(){}

	RawMeshData(const Mesh& m)
	:	primitive(m.primitive()), stroke(m.stroke()),
		Nv(m.vertices().size()), Nn(m.normals().size()), Nc(m.colors().size()),	Nci(m.coloris().size()), Nt1(m.texCoord1s().size()), Nt2(m.texCoord2s().size()), Nt3(m.texCoord3s().size()), Ni(m.indices().size())
	{
		count = Ni ? Ni : Nv;
		if(Nv ) vertices = decltype(vertices)(&m.vertices()[0][0]);
		if(Nn ) normals = decltype(normals)(&m.normals()[0][0]);
		if(Nc ) colors = decltype(colors)(&m.colors()[0][0]);
		if(Nci) coloris = decltype(coloris)(&m.coloris()[0][0]);
		if(Nt1) texCoord1s = decltype(texCoord1s)(&m.texCoord1s()[0]);
		if(Nt2) texCoord2s = decltype(texCoord2s)(&m.texCoord2s()[0][0]);
		if(Nt3) texCoord3s = decltype(texCoord3s)(&m.texCoord3s()[0][0]);
		if(Ni ) indices = decltype(indices)(&m.indices()[0]);
	}

	RawMeshData(const Mesh& m, int countIn, int beginIn=0)
	:	RawMeshData(m)
	{
		int Nmax = count; // max # elements (vertices or indices)
		count = countIn;
		begin = beginIn;
		// Adjust any negative amounts
		if(count < 0) count += Nmax+1;
		if(begin < 0) begin += Nmax+1;
		if(begin >= Nmax){			// Begin index past end?
			begin = Nmax;
			count = 0;
		}
		if(begin + count > Nmax){	// If end index past end, then truncate it
			count = Nmax - begin;
		}
	}
};


class GPUMesh : public Graphics::RawMeshData{
public:

	void bind(){
		mVBO.bind();
		if(Ni) mEBO.bind();
		mBound = true;
	}

	void unbind(){
		mVBO.unbind();
		if(Ni) mEBO.unbind();
		mBound = false;
	}

	bool bound() const { return mBound; }

	void update(const Mesh& m){
		Nv = m.vertices().size();
		if(0 == Nv) return;
		Nn = m.normals().size();
		Nc = m.colors().size();
		Nci= m.coloris().size();
		Nt1= m.texCoord1s().size();
		Nt2= m.texCoord2s().size();
		Nt3= m.texCoord3s().size();
		Ni = m.indices().size();

		primitive = m.primitive();
		stroke = m.stroke();
		count = Ni ? Ni : Nv;

		mVBO.usage(BufferObject::STATIC_DRAW);
		int b = 0; // current byte offset into VBO
		b = mVBO.subData(&m.vertices()[0], Nv);

		if(Nn){
			normals = vertices + b;
			b = mVBO.subData(&m.normals()[0], Nn, b);
		}

		if(Nc){
			colors = vertices + b;
			b = mVBO.subData(&m.colors()[0], Nc, b);
		} else if(Nci){
			coloris = vertices + b;
			b = mVBO.subData(&m.coloris()[0], Nci, b);
		}

		if(Nt2){
			texCoord2s = vertices + b;
			b = mVBO.subData(&m.texCoord2s()[0], Nt2, b);
		} else if(Nt1){
			texCoord1s = vertices + b;
			b = mVBO.subData(&m.texCoord1s()[0], Nt1, b);
		} else if(Nt3){
			texCoord3s = vertices + b;
			b = mVBO.subData(&m.texCoord3s()[0], Nt3, b);
		}

		// Here, b should be the total number of bytes in the buffer
		mVBO.resize(b);

		if(Ni){
			mEBO.usage(BufferObject::STATIC_DRAW);
			mEBO.data(&m.indices()[0], m.indices().size());
		}
	}

private:
	VBO mVBO;
	EBO mEBO;
	bool mBound = false;
};


std::map<const Mesh *, std::unique_ptr<GPUMesh>> gpuMeshes;
//std::map<const MeshNew *, GPUMesh *> gpuMeshes;
GPUMesh * currentGPUMesh = NULL;


#ifdef AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
class Graphics::BackendProg : public Graphics::Backend{
public:
	BackendProg(Graphics& g)
	:	Graphics::Backend(g)
	{
		for(auto& stack : mMatrixStacks){
			stack.var().emplace(1.f);
		}

		mShader.name("al::Graphics");
		g.material(); // touch so shader uniforms get inited
	}

	void enable(Capability v){
		switch(v){
		case FOG: mGraphics.mDoFog=true; mGraphics.mFog.update(); break;
		case LIGHTING: mGraphics.mDoLighting=true; break;
		default: glEnable(v);
		}
	}
	void disable(Capability v){
		switch(v){
		case FOG: mGraphics.mDoFog=false; mGraphics.mFog.update(); break;
		case LIGHTING: mGraphics.mDoLighting=false; break;
		default: glDisable(v);
		}
	}
	void currentColor(float r, float g, float b, float a){
		mCurrentColor.set(r,g,b,a);
	}

	const Mat4f& modelView() const { return mMatrixStacks[MODELVIEW].get().top(); }
	const Mat4f& projection() const { return mMatrixStacks[PROJECTION].get().top(); }

	void matrixMode(MatrixMode mode){ mMatrixMode=mode; }
	void pushMatrix(){
		//printf("push %s (%d/%d)\n", mMatrixMode==MODELVIEW?"MV":"Proj", (int)currentMatrixStack().get().size(), (int)maxStackSize(mMatrixMode));
		if(currentMatrixStack().get().size() < maxStackSize(mMatrixMode)){
			currentMatrixStack().var().emplace(currentMatrix());
		} else {
			AL_WARN_ONCE("%s matrix stack overflow", mMatrixMode==MODELVIEW?"MV":"Proj");
		}
	}
	void popMatrix(){
		if(currentMatrixStack().get().size() > 1){
			currentMatrixStack().var().pop();
		} else {
			AL_WARN_ONCE("%s matrix stack underflow", mMatrixMode==MODELVIEW?"MV":"Proj");
		}
	}
	void loadIdentity(){ currentMatrix().setIdentity(); }
	void loadMatrix(const Mat4d& m){ currentMatrix() = m; }
	void loadMatrix(const Mat4f& m){ currentMatrix() = m; }
	void multMatrix(const Mat4d& m){ currentMatrix() *= m; }
	void multMatrix(const Mat4f& m){ currentMatrix() *= m; }
	void translate(float x, float y, float z){
		multMatrix(Mat4f::translation(x,y,z));
	}
	void rotate(float angle, float x, float y, float z){
		multMatrix(Mat4f::rotation(angle*0.01745329251994, Vec3f(x,y,z).normalize()));
	}
	void scale(float s){ scale(s,s,s); }
	void scale(float x, float y, float z){
		multMatrix(Mat4f::scaling(x,y,z));
	}

	void pointSize(float v){ mPointSize = v; }
	//void pointAtten(float c2, float c1, float c0){}

	// Compile/link shader once and return whether successful
	bool validateShader(){
		if(mCompileShader){
			mCompileShader = false; // will only make one attempt

// Note: Presence of backslashes '\' anywhere in code, including comments, are
// known to trip up compilation in Firefox 74.
			mShader.preamble(
R"(
#ifdef GL_ES
precision mediump float; // req'd by ES2
#endif
)" +
mPreamble
+ R"(
const float pi = 3.141592653589793;
varying vec3 pos;		// position (eye space)
varying vec3 normal;	// normal (eye space)
varying vec3 posObj;	// position (object space)
varying vec4 color;
varying vec2 texCoord2;

uniform mat4 view;		// view matrix (convert from world to eye space)
uniform bool doTex2;

struct Fog{
	vec3 color;
	float start, end;
	float scale; // 1/(end-start)
};
uniform Fog fog;// = Fog(vec3(0.), 0., 1., 1.);
varying float fogMix;
)"
			);

			mShader.compile(
// VERTEX PROGRAM
// ==============
R"(
uniform mat4 MV;
uniform mat4 P;
uniform mat3 normalMatrix;
uniform vec4 singleColor;
uniform float pointSize;
uniform bool hasNormals;
attribute vec3 posIn;
attribute vec3 normalIn;
attribute vec4 colorIn;
attribute vec2 texCoord2In;

void main(){
	posObj = posIn;
	color = singleColor.a==8192. ? colorIn : singleColor;
	if(hasNormals){
		normal = normalMatrix * normalIn;
	} else {
		normal = vec3(1.,0.,0.);
	}
	if(doTex2){
		texCoord2 = texCoord2In;
	}
	gl_PointSize = pointSize;
)" +
	mOnVertex +
R"(
	pos = (MV * vec4(posObj,1.)).xyz; // to eye space
	gl_Position = P * vec4(pos,1.); // to screen space

	// fogMix: [0,1] -> [start, end]
	fogMix = clamp((-pos.z - fog.start) * fog.scale, 0.,1.);
}
)",


// FRAGMENT PROGRAM
// ================
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

void zero(inout LightFall l){
	l.diffuse = vec3(0.);
	l.specular = vec3(0.);
}

// Surface material
struct Material{
	vec3 diffuse;		// Component scattered off surface
	vec3 specular;		// Component bounced/reflected off surface
	vec3 emission;		// Component emitted from surface
	float shininess;	// Concentration of specular (its lack of scattering)
	float reflectance;	// Reflectance coef for Fresnel specular factor
};

uniform Light lights[MAX_LIGHTS];
uniform vec3 globalAmbient;
uniform Material materials[2];
uniform sampler2D tex2;
uniform bool lightTwoSided;
uniform bool doLighting;
uniform bool colorMaterial;
uniform bool materialOneSided;

Material lerp(in Material m1, in Material m2, float frac){
	Material m;
	m.diffuse    = mix(m1.diffuse,    m2.diffuse,    frac);
	m.specular   = mix(m1.specular,   m2.specular,   frac);
	m.emission   = mix(m1.emission,   m2.emission,   frac);
	m.shininess  = mix(m1.shininess,  m2.shininess,  frac);
	m.reflectance= mix(m1.reflectance,m2.reflectance,frac);
	return m;
}

float _pow5(float x){ float xx=x*x; return xx*xx*x; }

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
/// returns light color
LightFall light(in vec3 pos, in vec3 N, in vec3 V, in Light light, in Material mat){
	// Note: light attenuation over distance is an exponential decay (Beer-Lambert)
	vec3 lightVec = light.pos - pos;
	vec3 L = normalize(lightVec); // dir from surface to light

	float intens = light.strength;

	// Distance attenuation: 1/(1+[d/h]^2) = h^2 / (h^2 + d^2)
	{	float hh = light.halfDist*light.halfDist;
		intens *= hh / (hh + dot(lightVec,lightVec));
	}

	// Spotlight
	if(light.spread < 180.){
		float coneAmt = -dot(light.dir, L); // cos of angle: [1,-1] -> [coincident, opposing]
		//coneAmt = coneAmt*-0.5+0.5; // [1,-1] -> [0,1]
		float cosMax = cos(light.spread * pi/180.);
		float coneDist = (1.-coneAmt)/(1.-cosMax); // apx dist from cone center [0,1]
		//float coneAmp = 1.-min(coneDist,1.); // linear falloff
		float coneAmp = min(coneDist,1.)-1.; coneAmp*=coneAmp; // parabolic falloff
		intens *= coneAmp;
	}

	// Diffuse/specular
	float diffAmt = (max(dot(N,L), 0.) + light.ambient) * intens;
	vec3 H = normalize(L + V); // half-vector
	float specAmt = pow(max(dot(N,H), 0.), mat.shininess) * intens; // Blinn-Phong
	//float specAmt = pow(max(dot(reflect(-L,N),V), 0.), mat.shininess*0.25) * intens; // Phong

	/* Specular approx [Lyon, 1993. "Phong Shading Reformulation"]
	vec3 R = reflect(-L,N), D = R-V;
	//vec3 H = L+V, D = dot(H,N)*N - H; // bug: flaring effect
	//float c = 1.-min(1.,mat.shininess*0.25*0.5*dot(D,D)); // 1st order apx
	float c = 1.-min(1.,mat.shininess*0.25*0.25*dot(D,D)); c*=c; // 2nd order apx
	//float c = 1.-min(1.,mat.shininess*0.25*0.125*dot(D,D)); c*=c; c*=c; // 3rd order apx
	float specAmt = c * intens;
	//*/

	// Fresnel "specular grazing" using Schlick apx
	specAmt *= mix(mat.reflectance, 1., _pow5(1. - dot(H,V)));

	LightFall fall;
	fall.diffuse  = light.diffuse  * diffAmt;
	fall.specular = light.specular * specAmt;
)" +
	mOnLight +
R"(
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
			LightFall l = light(pos, N, V, lights[i], material);
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
	if(doTex2){
		col *= texture2D(tex2, texCoord2).rgb;
	}
	if(doLighting){
		vec3 N = normalize(normal);
		vec3 V = normalize(-pos); // surface to eye
		// Two-sided lighting: make normal always face eye
		if(lightTwoSided && !gl_FrontFacing) N=-N;
		Material material;
		if(gl_FrontFacing || materialOneSided) material = materials[0];
		else material = materials[1];
		if(colorMaterial) material.diffuse *= col;
)" +
		mOnMaterial +
R"(
		col = lightColor(pos, N, V, material);
	}
	col = mix(col, fog.color, fogMix);
	gl_FragColor = vec4(col, color.a);
	//gl_FragColor = vec4(1.,0.,0.,1.); //debug
}
)"
			);

			if(mShader.linked()){
				mLocPos       = mShader.attribute("posIn");
				mLocColor     = mShader.attribute("colorIn");
				mLocNormal    = mShader.attribute("normalIn");
				mLocTexCoord2 = mShader.attribute("texCoord2In");
				mMatrixStacks[MODELVIEW].loc() = mShader.uniform("MV");
				mMatrixStacks[PROJECTION].loc() = mShader.uniform("P");
				mGraphics.mDoLighting.loc() = mShader.uniform("doLighting");
				#define SET_LOC(name)\
					o.loc().name = mShader.uniform((pre + #name).c_str());
				for(int i=0; i<AL_MAX_LIGHTS; ++i){
					auto& o = mGraphics.mLights[i];
					std::string pre = "lights[" + std::to_string(i) + "].";
					SET_LOC(pos) SET_LOC(dir) SET_LOC(halfDist) SET_LOC(spread)
					SET_LOC(strength) SET_LOC(diffuse) SET_LOC(specular) SET_LOC(ambient)
				}
				for(int i=0; i<2; ++i){
					auto& o = mGraphics.mMaterials[i];
					std::string pre = "materials[" + std::to_string(i) + "].";
					SET_LOC(diffuse) SET_LOC(emission) SET_LOC(specular) SET_LOC(shininess) SET_LOC(reflectance)
				}
				mGraphics.mMaterialOneSided.loc() = mShader.uniform("materialOneSided");
				mGraphics.mFog.loc().color = mShader.uniform("fog.color");
				mGraphics.mFog.loc().start = mShader.uniform("fog.start");
				mGraphics.mFog.loc().end   = mShader.uniform("fog.end");
				mGraphics.mFog.loc().scale = mShader.uniform("fog.scale");
				mPointSize.loc() = mShader.uniform("pointSize");
				// init uniforms
				mShader.begin();
					mShader.uniform("colorMaterial", true);
				mShader.end();
			} else {
				printf("Critical error: al::Graphics failed to compile shader\n");
				mShader.printLog();
			}
		}

		//printf("%d %d %d %d\n", mLocPos, mLocColor, mLocNormal, mLocTexCoord2);
		// If negative, shader didn't compile
		return mLocPos >= 0;
	}

	bool prepareDraw(){

		if(!validateShader()) return false;

		auto& mLights = mGraphics.mLights;
		auto& mMaterials = mGraphics.mMaterials;
		auto& mFog = mGraphics.mFog;
		auto& mDoLighting = mGraphics.mDoLighting;
		auto& mMaterialOneSided = mGraphics.mMaterialOneSided;
		auto& mDoFog = mGraphics.mDoFog;
		auto& mView = mGraphics.mView;
		auto& mUpdateView = mGraphics.mUpdateView;

		mShader.begin();
			if(mMatrixStacks[MODELVIEW].handleUpdate()){
				// Needed to correctly convert normals into eye space
				mShader.uniform("normalMatrix", normalMatrix(modelView()));
				mShader.uniform(mMatrixStacks[MODELVIEW].loc(), modelView());
			}

			if(mMatrixStacks[PROJECTION].handleUpdate()){
				mShader.uniform(mMatrixStacks[PROJECTION].loc(), projection());
			}

			if(mUpdateView){
				mShader.uniform("view", mView);
			}

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
				mShader.uniform(mDoLighting.loc(), mDoLighting.get());
			}

			if(mDoLighting == true){
				mShader.uniform("lightTwoSided", Light::twoSided());

				auto xfm = [](const Mat4f& m, const Vec3f& v){
					return (m * Vec4f(v,1.f)).xyz();
				};
				auto viewRot = mView.sub<3>();

				for(const auto& l : mLights){
					bool updateLight = l.handleUpdate();
					bool lightActive = l.get().strength() != 0;
					if(lightActive && (updateLight || mUpdateView)){
						//printf("light %p pos\n", &l.get());
						mShader.uniform(l.loc().pos, xfm(mView, l.get().pos()));
						mShader.uniform(l.loc().dir, viewRot * l.get().dir());
					}
					if(updateLight){
						//printf("light %p rest\n", &l.get());
						if(lightActive){
							auto max = [](const al::RGB& c){ return std::max(std::max(c.r, c.g), c.b); };
							mShader.uniform(l.loc().halfDist, l.get().halfDist());
							mShader.uniform(l.loc().spread, l.get().spread());
							mShader.uniform(l.loc().diffuse, l.get().diffuse().rgb());
							mShader.uniform(l.loc().specular, l.get().specular().rgb());
							mShader.uniform(l.loc().ambient, max(l.get().ambient().rgb()));
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
					mShader.uniform(m.loc().diffuse, m.get().diffuse().rgb() + m.get().ambient().rgb());
					mShader.uniform(m.loc().emission, m.get().emission().rgb());
					mShader.uniform(m.loc().specular, m.get().specular().rgb());
					mShader.uniform(m.loc().shininess, m.get().shininess());
					mShader.uniform(m.loc().reflectance, m.get().reflectance());
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

		mShader.end();

		mUpdateView = false;

		return true;
	}

	void draw(const RawMeshData& m){

		DRAW_BEGIN;

		int Nci = m.Nci; // needed for strange OSX bug below

		// glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
		glEnableVertexAttribArray(mLocPos);
		glVertexAttribPointer(mLocPos, 3, GL_FLOAT, 0, 0, m.vertices);

		if(m.Nn >= m.Nv){
			glEnableVertexAttribArray(mLocNormal);
			glVertexAttribPointer(mLocNormal, 3, GL_FLOAT, 0, 0, m.normals);
		}

		const float colorArrayFlag = 8192.;
		Color singleColor(0,0,0,colorArrayFlag); // if unchanged, triggers read from array

		if(m.Nc >= m.Nv){
			glEnableVertexAttribArray(mLocColor);
			glVertexAttribPointer(mLocColor, 4, GL_FLOAT, 0, 0, m.colors);
		}
		else if(m.Nci >= m.Nv){
			glEnableVertexAttribArray(mLocColor);
			glVertexAttribPointer(mLocColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, m.coloris);
		}
		else if(0 == m.Nc && 0 == m.Nci){
			singleColor = mCurrentColor;
		}
		else{
			singleColor = m.Nc ? *(Color*)m.colors : Color(*(Colori*)m.coloris);
		}

		// There is a strange bug on OSX where we cannot switch between single
		// and array color reads in a shader.
		// TODO: if VBO bound, must bind a separate color VBO
		#ifdef AL_OSX
		if(singleColor.a != colorArrayFlag){ // using single color
			Colori col = singleColor;
			singleColor.a = colorArrayFlag;
			// Avoid copies if possible
			if(mColorArray.empty() || mColorArray.size() < unsigned(m.Nv) || mColorArray[0] != col){
				mColorArray.clear();
				for(int i=0; i<m.Nv; ++i) mColorArray.push_back(col);
			}
			if(currentGPUMesh && currentGPUMesh->bound()){
			} else {
				glEnableVertexAttribArray(mLocColor);
				glVertexAttribPointer(mLocColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &mColorArray[0]);
			}
			Nci = m.Nv;
		}
		#endif

		if(m.Nt2 >= m.Nv){
			glEnableVertexAttribArray(mLocTexCoord2);
			glVertexAttribPointer(mLocTexCoord2, 2, GL_FLOAT, 0, 0, m.texCoord2s);
		}

		mShader.begin();
			mShader.uniform("singleColor", singleColor);
			mShader.uniform("hasNormals", bool(m.Nn));
			mShader.uniform("doTex2", m.Nt2 >= m.Nv);

			// Should go in prepareDraw, but only seems to work here???
			if(mPointSize.handleUpdate()){
				mShader.uniform(mPointSize.loc(), mPointSize.get());
				#ifdef AL_GRAPHICS_USE_OPENGL
				glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
				#endif
			}

			if(m.Ni){
				// Here, 'count' is the number of indices to render
				glDrawElements(m.primitive, m.count, GL_UNSIGNED_INT, m.indices + m.begin*sizeof(Mesh::Index));
			}
			else{
				glDrawArrays(m.primitive, m.begin, m.count);
			}
		mShader.end();

		glDisableVertexAttribArray(mLocPos);
		if(m.Nc || Nci) glDisableVertexAttribArray(mLocColor);
		if(m.Nn) glDisableVertexAttribArray(mLocNormal);
		if(m.Nt2) glDisableVertexAttribArray(mLocTexCoord2);
	}

	void draw(const Mesh& m, int count, int begin){
		draw(RawMeshData(m,count,begin));
	}

protected:
	friend class Graphics;
	typedef ShaderData<std::stack<Mat4f>> MatrixStack;
	MatrixStack mMatrixStacks[2];

	static unsigned maxStackSize(MatrixMode mode){
		static unsigned s[] = {
			AL_MODELVIEW_STACK_SIZE, AL_PROJECTION_STACK_SIZE
		};
		return s[mode];
	};
	MatrixMode mMatrixMode = MODELVIEW;
	MatrixStack& currentMatrixStack(){
		return mMatrixStacks[mMatrixMode];
	}
	const MatrixStack& currentMatrixStack() const {
		return mMatrixStacks[mMatrixMode];
	}
	Mat4f& currentMatrix(){ return currentMatrixStack().var().top(); }
	const Mat4f& currentMatrix() const { return currentMatrixStack().get().top(); }
	ShaderProgram mShader;
	int mLocPos=-1, mLocColor, mLocNormal, mLocTexCoord2;
	std::string mPreamble, mOnVertex, mOnLight, mOnMaterial;
	Color mCurrentColor;
	ShaderData<float> mPointSize{1};
	std::vector<Colori> mColorArray;
	bool mCompileShader = true;
};
#endif // AL_GRAPHICS_SUPPORTS_PROG_PIPELINE


#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
class Graphics::BackendFixed : public Graphics::Backend{
public:
	BackendFixed(Graphics& g)
	:	Graphics::Backend(g)
	{}

	void enable(Capability v){
		switch(v){
		case FOG:				glEnable(GL_FOG); break;
		case LIGHTING:			glEnable(GL_LIGHTING); break;
		case COLOR_MATERIAL:	glEnable(GL_COLOR_MATERIAL); break;
		default:				glEnable(v);
		}
	}
	void disable(Capability v){
		switch(v){
		case FOG:				glDisable(GL_FOG); break;
		case LIGHTING:
			glDisable(GL_LIGHTING);
			// cancel updates to prevent Light from enabling GL_LIGHTING again
			for(auto& l : mGraphics.mLights) l.handleUpdate();
			break;
		case COLOR_MATERIAL:	glDisable(GL_COLOR_MATERIAL); break;
		default:				glDisable(v);
		}
	}
	void currentColor(float r, float g, float b, float a){ glColor4f(r,g,b,a); }
	void pointSize(float v){ glPointSize(v); }

	const Mat4f& modelView() const {
		glGetFloatv(GL_MODELVIEW_MATRIX, &mModelViewTemp[0]);
		return mModelViewTemp;
	}
	const Mat4f& projection() const { 
		glGetFloatv(GL_PROJECTION_MATRIX, &mProjectionTemp[0]);
		return mProjectionTemp;
	}
	
	void matrixMode(MatrixMode mode){
		switch(mode){
		case MODELVIEW:		glMatrixMode(GL_MODELVIEW); break;
		case PROJECTION:	glMatrixMode(GL_PROJECTION); break;
		default:			glMatrixMode(mode); break;
		}
	}
	void pushMatrix(){ glPushMatrix(); }
	void popMatrix(){ glPopMatrix(); }
	void loadIdentity(){ glLoadIdentity(); }
	void loadMatrix(const Mat4f& m){ glLoadMatrixf(m.elems()); }
	void multMatrix(const Mat4f& m){ glMultMatrixf(m.elems()); }
		#ifdef AL_GRAPHICS_SUPPORTS_DOUBLE
		void loadMatrix(const Mat4d& m){ glLoadMatrixd(m.elems()); }
		void multMatrix(const Mat4d& m){ glMultMatrixd(m.elems()); }
		#else
		void loadMatrix(const Mat4d& m){ loadMatrix(Mat4f(m)); }
		void multMatrix(const Mat4d& m){ multMatrix(Mat4f(m)); }
		#endif
	void translate(float x, float y, float z){ glTranslatef(x,y,z); }
	void rotate(float angle, float x, float y, float z){ glRotatef(angle,x,y,z); }
	void scale(float s){
		if(mRescaleNormal < 1){
			mRescaleNormal = 1;
			enable(RESCALE_NORMAL);
		}
		glScalef(s, s, s);
	}
	void scale(float x, float y, float z){
		if(mRescaleNormal < 3){
			mRescaleNormal = 3;
			disable(RESCALE_NORMAL);
			enable(NORMALIZE);
		}
		glScalef(x, y, z);
	}
	void pointAtten(float c2, float c1, float c0){
		GLfloat att[3] = {c0, c1, c2};
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
	}

	bool prepareDraw(){
		auto& mLights = mGraphics.mLights;
		auto& mMaterials = mGraphics.mMaterials;
		auto& mFog = mGraphics.mFog;

		// Note: Nothing submitted to GPU if light strength zero
		bool loadedViewMatrix = false;

		for(int i=0; i<AL_MAX_LIGHTS; ++i){
			const auto& l = mLights[i].get();
			auto glID = l.id();
			bool lightActive = l.strength() != 0.f;
			if(lightActive){
				// With fixed pipeline, setting light position multiplies
				// the position by the current modelview. Typically, you just
				// want to multiply by the view part, so we load it here.
				if(!loadedViewMatrix){
					loadedViewMatrix = true;
					mGraphics.pushMatrix(Graphics::MODELVIEW);
					mGraphics.loadMatrix(mGraphics.mView);
				}
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
		if(loadedViewMatrix) mGraphics.popMatrix();

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

		return true;
	}

	void draw(const RawMeshData& m){

		DRAW_BEGIN;

		// Enable arrays and set pointers...
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, m.vertices);

		if(m.Nn >= m.Nv){
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, 0, m.normals);
		}

		if(m.Nc >= m.Nv){
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, 0, m.colors);
		}
		else if(m.Nci >= m.Nv){
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, m.coloris);
			//printf("using integer colors\n");
		}
		else if(0 == m.Nc && 0 == m.Nci){
			// just use whatever the last glColor() call used!
		}
		else{
			if(m.Nc){
				auto col = *(Color *)m.colors;
				glColor4f(col.r, col.g, col.b, col.a);
			} else {
				auto col = *(Colori *)m.coloris;
				glColor4ub(col.r, col.g, col.b, col.a);
			}
		}

		if(m.Nt1 >= m.Nv){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(1, GL_FLOAT, 0, m.texCoord1s);
		}
		else if(m.Nt2 >= m.Nv){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, m.texCoord2s);
		}
		else if(m.Nt3 >= m.Nv){
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(3, GL_FLOAT, 0, m.texCoord3s);
		}

		// Draw
		if(m.Ni){
			#ifdef AL_GRAPHICS_SUPPORTS_INT32
				// Here, 'count' is the number of indices to render
				glDrawElements(m.primitive, m.count, GL_UNSIGNED_INT, m.indices + m.begin*sizeof(Mesh::Index));
			#else
				mIndices16.clear();
				for(int i=m.begin; i<m.begin+m.count; ++i){
					auto idx = m.indices()[i];
					if(idx > 65535) AL_WARN_ONCE("Mesh index value out of range (> 65535)");
					mIndices16.push_back(idx);
				}
				glDrawElements(m.primitive, m.count, GL_UNSIGNED_SHORT, &mIndices16[0]);
			#endif
		}
		else{
			glDrawArrays(m.primitive, m.begin, m.count);
		}

		// Disable arrays
		glDisableClientState(GL_VERTEX_ARRAY);
		if(m.Nn)					glDisableClientState(GL_NORMAL_ARRAY);
		if(m.Nc || m.Nci)			glDisableClientState(GL_COLOR_ARRAY);
		if(m.Nt1 || m.Nt2 || m.Nt3)	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	void draw(const Mesh& m, int count, int begin){
		draw(RawMeshData(m, count, begin));
	}

private:
	int mRescaleNormal = 0;
	std::vector<unsigned short> mIndices16;
	mutable Mat4f mModelViewTemp, mProjectionTemp;
};
#endif // AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE


Graphics::Graphics()
:	mInImmediateMode(false)
{
	#ifdef AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
		mBackends[PROG] = new BackendProg(*this);
	#endif

	#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
		mBackends[FIXED] = new BackendFixed(*this);
	#endif

	#ifdef AL_GRAPHICS_USE_PROG_PIPELINE
		pipeline(PROG);
	#else
		pipeline(FIXED);
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
}

Graphics::~Graphics(){
	for(auto * b : mBackends){
		if(b) delete b;
	}
}

void Graphics::pipeline(Pipeline p){
	switch(p){
	case FIXED:
		#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
		mBackend = mBackends[FIXED];
		#else
		AL_WARN("Fixed pipeline not supported");
		#endif
		break;
	case PROG:
		#ifdef AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
		mBackend = mBackends[PROG];
		#else
		AL_WARN("Programmable pipeline not supported");
		#endif
		break;
	}
}

ShaderProgram& Graphics::shader(){
	if(mBackends[PROG]){
		auto * backend =  dynamic_cast<BackendProg *>(mBackends[PROG]);
		backend->validateShader();
		return backend->mShader;
	} else {
		static ShaderProgram dummyShader;
		return dummyShader;
	}
}

Graphics& Graphics::shaderPreamble(const std::string& s){
	if(mBackends[PROG]){
		dynamic_cast<BackendProg *>(mBackends[PROG])->mPreamble = s;
	}
	return *this;
}
Graphics& Graphics::shaderOnVertex(const std::string& s){
	if(mBackends[PROG]){
		dynamic_cast<BackendProg *>(mBackends[PROG])->mOnVertex = s;
	}
	return *this;
}
Graphics& Graphics::shaderOnLight(const std::string& s){
	if(mBackends[PROG]){
		dynamic_cast<BackendProg *>(mBackends[PROG])->mOnLight = s;
	}
	return *this;
}
Graphics& Graphics::shaderOnMaterial(const std::string& s){
	if(mBackends[PROG]){
		dynamic_cast<BackendProg *>(mBackends[PROG])->mOnMaterial = s;
	}
	return *this;
}

void Graphics::setVertexBuffer(const Mesh& m, bool updateBuffer){
	auto it = gpuMeshes.find(&m);
	if(it != gpuMeshes.end()){ // found existing VBO for passed in mesh
		currentGPUMesh = it->second.get();
	} else { // create new buffer and update
		currentGPUMesh = new GPUMesh;
		gpuMeshes[&m] = std::unique_ptr<GPUMesh>(currentGPUMesh);
		updateBuffer = true;
	}
	
	if(updateBuffer) currentGPUMesh->update(m);
}

void Graphics::drawVertexBuffer(){
	currentGPUMesh->bind();
	mBackend->draw(*currentGPUMesh);
	currentGPUMesh->unbind();
}

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

Graphics::Format Graphics::toFormat(int n){
	static const Format f[] = {LUMINANCE, LUMINANCE_ALPHA, RGB, RGBA};
	n = n<1 ? 1 : n>4 ? 4 : n; // clamp in [1,4]
	return f[n-1];
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
	return glGetErrorString(verbose);
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

void Graphics::fog(float end, float start, const Color& c){
	//enable(FOG);
	mDoFog = true;
	auto& f = mFog.var();
	f.start = start;
	f.end = end;
	f.scale = 1./(end-start);
	f.color = c;
}

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

void Graphics::onCreate(){
	GPUObject::mID = 1; // must be non-zero to flag creation
	//mRescaleNormal = 0;
}

void Graphics::onDestroy(){
}

// deprecated
void Graphics::draw(int num_vertices, const Mesh& m){
	draw(m, num_vertices);
}

} // al::
