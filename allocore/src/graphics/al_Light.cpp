#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Light.hpp"

namespace al{

// This is used to manage GL light IDs
struct LightPool{
	static const int Nlights = 8;
	bool mAllocated[Nlights] = {0};

	static int glLightID(int i){
		#ifdef AL_GRAPHICS_USE_FIXED_PIPELINE
			static int x[Nlights] =
				{GL_LIGHT0,GL_LIGHT1,GL_LIGHT2,GL_LIGHT3,GL_LIGHT4,GL_LIGHT5,GL_LIGHT6,GL_LIGHT7};
			return x[i];
		#else
			return i;
		#endif
	}

	int nextIndex(){
		for(int i=0; i<Nlights; ++i){
			if(!mAllocated[i]){
				mAllocated[i] = true;
				//printf("LightPool::nextIndex returning %d\n", i);
				return i;
			}
		}
		return Nlights-1;
	}

	void freeID(int i){
		mAllocated[i] = false;
	}
};

static LightPool lightPool;


#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
void Material::operator()() const {

	GLenum glface = face();

	// Note glColorMaterial and glMaterial components are mutually exclusive;
	// if GL_COLOR_MATERIAL is enabled, the respective glMaterial components
	// are ignored and vice-versa.
	if (useColorMaterial()) {
		glEnable(GL_COLOR_MATERIAL);	// need to enable for glColor* to work
		#ifdef AL_GRAPHICS_SUPPORTS_COLOR_MATERIAL_SPEC
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		#endif
	} else {
		glDisable(GL_COLOR_MATERIAL);	// need to disable for glMaterial* to work
		glMaterialfv(glface, GL_AMBIENT, mAmbient.components);
		glMaterialfv(glface, GL_DIFFUSE, mDiffuse.components);
	}

	glMaterialfv(glface, GL_EMISSION,	mEmission.components);
	glMaterialfv(glface, GL_SPECULAR,	mSpecular.components);
	glMaterialf (glface, GL_SHININESS,	mShine);
}


void Light::operator()() const {
	auto glID = id();
	if(mStrength != 0.){
		submitPos(glID);
		submitCol(glID);
	} else {
		glDisable(glID);
	}
}

void Light::submitCol(int glID) const {
	glEnable(GL_LIGHTING);

	if(sGlobalAmbientUpdate){
		sGlobalAmbientUpdate = false;
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sGlobalAmbient.components);
	}
	if(sTwoSidedUpdate){
		sTwoSidedUpdate = false;
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, sTwoSided ? GL_TRUE : GL_FALSE);
	}

	glLightfv(glID, GL_AMBIENT,		mAmbient.components);
	glLightfv(glID, GL_DIFFUSE,		mDiffuse.components);
	glLightfv(glID, GL_SPECULAR,	mSpecular.components);

	// atten = 1 / (kc + kl d + kq d^2)
	float invStrength = 1./mStrength;
    glLightf(glID, GL_CONSTANT_ATTENUATION,	mAtten[0]*invStrength);
    glLightf(glID, GL_LINEAR_ATTENUATION,	mAtten[1]*invStrength);
    glLightf(glID, GL_QUADRATIC_ATTENUATION,mAtten[2]*invStrength);

	glEnable(glID); // MUST enable each light source after configuration
	//glShadeModel(GL_SMOOTH); // enabled by default and don't want to force...
}

// This must be called every frame if the light is to track a varying modelview
void Light::submitPos(int glID) const {
	if(mSpread < 180.){
		float pos[4];
		for(int i=0;i<3;++i){ pos[i]=mPos[i]; } pos[3]=1.;
		glLightfv(glID, GL_POSITION, pos);
		glLightfv(glID, GL_SPOT_DIRECTION, &mDir[0]);
		glLightf (glID, GL_SPOT_CUTOFF, mSpread);
		//glLightf (glID, GL_SPOT_EXPONENT, expo);
	} else {
		float pos[4];
		if(mIsDir){ for(int i=0;i<3;++i) pos[i]=mDir[i]; pos[3]=0.; }
		else      { for(int i=0;i<3;++i) pos[i]=mPos[i]; pos[3]=1.; }
		glLightfv(glID, GL_POSITION, pos);
	}
}

#else
void Material::operator()() const {}
void Light::operator()() const {}
void Light::submitCol(int glID) const {}
void Light::submitPos(int glID) const {}

#endif

Material::Material()
:	Material(Graphics::FRONT)
{}

Material::Material(int faceDir)
:	mAmbient(0.2),		// These are the default OpenGL values. Do not change!
	mDiffuse(0.8),		// See http://www.khronos.org/opengles/sdk/1.1/docs/man/glMaterial.xml
	mEmission(0),
	mSpecular(0),		// Specular is 0 to ensure linear lighting (no popping).
	mShine(5.),
	mOpticalDensity(0.),
	mIllumination(0.),
	mFace(faceDir),
	mUseColorMaterial(true)
{}

Material& Material::ambientAndDiffuse(const Color& v){
	ambient(v);
	return diffuse(v);
}
Material& Material::ambient(const Color& v){
	mAmbient=v;
	mUseColorMaterial=false; // expected since setting material property
	return *this;
}
Material& Material::diffuse(const Color& v){
	mDiffuse=v;
	mUseColorMaterial=false; // expected since setting material property
	return *this;
}
Material& Material::emission(const Color& v){ mEmission=v; return *this; }
Material& Material::specular(const Color& v){ mSpecular=v; return *this; }
Material& Material::shininess(float v){ mShine=v; return *this; }
Material& Material::face(int f){ mFace=f; return *this; }


// NOTE: all defaults match the OpenGL defaults for LIGHT0
Light::Light(float x, float y, float z)
:	mIndex(lightPool.nextIndex())
{
	pos(x,y,z);
	attenuation(1,0,0);
}

Light::~Light(){
	lightPool.freeID(mIndex);
}

int Light::id() const { return lightPool.glLightID(mIndex); }
int Light::index() const { return mIndex; }

/*static*/ Color Light::sGlobalAmbient{0.2};
/*static*/ bool Light::sGlobalAmbientUpdate = false;
/*static*/ bool Light::sTwoSided = false;
/*static*/ bool Light::sTwoSidedUpdate = false;

/*static*/ void Light::globalAmbient(const Color& v){
	sGlobalAmbient = v;
	sGlobalAmbientUpdate = true;
}

/*static*/ void Light::twoSided(bool v){
	if(v != sTwoSided){
		sTwoSided = v;
		sTwoSidedUpdate = true;
	}
}

Light& Light::ambient(const Color& v){ mAmbient=v; return *this; }
Light& Light::diffuse(const Color& v){ mDiffuse=v; return *this; }
Light& Light::specular(const Color& v){ mSpecular=v; return *this; }

Light& Light::strength(float v){ mStrength=v; return *this; }

Light& Light::pos(float x, float y, float z){
	mIsDir = false;
	mPos[0]=x; mPos[1]=y; mPos[2]=z; //mPos[3]=1;
	return *this;
}

Light& Light::dir(float x, float y, float z){
	mIsDir = true;
	mDir[0]=x; mDir[1]=y; mDir[2]=z; //mPos[3]=0;
	return *this;
}

Light& Light::attenuation(float c0, float c1, float c2){
	mAtten[0]=c0; mAtten[1]=c1; mAtten[2]=c2; return *this;
}

Light& Light::halfDist(float v){
	mHalfDist=v;
	return attenuation(1,0,1./v*v);
}

Light& Light::spread(float v){
	mSpread=v;
	return *this;
}

Light& Light::spot(float xDir, float yDir, float zDir, float cutoff, float expo){
	mSpread = cutoff;
	mDir.set(xDir, yDir, zDir);
	return *this;
}

} // ::al
