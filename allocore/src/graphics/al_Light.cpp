#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Light.hpp"

namespace al{

// This is used to manage GL light IDs
struct LightPool{
	static const int Nlights = 8;
	bool mIDs[Nlights] = {0};


	static int glLightID(int i){
		#ifdef AL_GRAPHICS_USE_FIXED_PIPELINE
			static int x[Nlights] =
				{GL_LIGHT0,GL_LIGHT1,GL_LIGHT2,GL_LIGHT3,GL_LIGHT4,GL_LIGHT5,GL_LIGHT6,GL_LIGHT7};
			return x[i];
		#else
			return i;
		#endif
	}

	int nextID(){
		for(int i=0; i<Nlights; ++i){
			if(!mIDs[i]){
				mIDs[i] = true;
				//printf("LightPool::nextID returning %d\n", i);
				return i;
			}
		}
		return Nlights-1;
	}

	void freeID(int i){
		mIDs[i] = false;
	}
};

static LightPool lightPool;


#ifdef AL_GRAPHICS_USE_FIXED_PIPELINE
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
	glEnable(GL_LIGHTING);
	int glID = lightPool.glLightID(mID);
	glLightfv(glID, GL_AMBIENT,		mAmbient.components);
	glLightfv(glID, GL_DIFFUSE,		mDiffuse.components);
	glLightfv(glID, GL_SPECULAR,	mSpecular.components);
	glLightfv(glID, GL_POSITION,	mPos);
    glLightf (glID, GL_CONSTANT_ATTENUATION,	mAtten[0]);
    glLightf (glID, GL_LINEAR_ATTENUATION,		mAtten[1]);
    glLightf (glID, GL_QUADRATIC_ATTENUATION,	mAtten[2]);

	glEnable(glID); // MUST enable each light source after configuration
	//glShadeModel(GL_SMOOTH); // enabled by default and don't want to force...
}

Light& Light::spot(float xDir, float yDir, float zDir, float cutoff, float expo){
	int glID = lightPool.glLightID(mID);
	float direction[] = {xDir, yDir, yDir};
	glLightfv(glID, GL_SPOT_DIRECTION, direction);
	glLightf (glID, GL_SPOT_CUTOFF, cutoff);
	glLightf (glID, GL_SPOT_EXPONENT, expo);
	return *this;
}

/*static*/ void Light::globalAmbient(const Color& v){
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, v.components);
}

/*static*/ void Light::twoSided(bool v){
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, v ? GL_TRUE : GL_FALSE);
}

#else
void Material::operator()() const {}

void Light::operator()() const {}
Light& Light::spot(float xDir, float yDir, float zDir, float cutoff, float expo){
	return *this;
}
/*static*/ void Light::globalAmbient(const Color& v){}
/*static*/ void Light::twoSided(bool v){}

#endif


Material::Material(Graphics::Direction f)
:	mAmbient(0.2),		// These are the default OpenGL values. Do not change!
	mDiffuse(0.8),		// See http://www.khronos.org/opengles/sdk/1.1/docs/man/glMaterial.xml
	mEmission(0),
	mSpecular(0),		// Specular is 0 to ensure linear lighting (no popping).
	mShine(5.),
	mFace(f),
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
Material& Material::face(Graphics::Direction f){ mFace=f; return *this; }


// NOTE: all defaults match the OpenGL defaults for LIGHT0
Light::Light(float x, float y, float z)
:	mID(lightPool.nextID()), mAmbient(0), mDiffuse(1), mSpecular(1)
{
	pos(x,y,z);
	attenuation(1,0,0);
}

Light::~Light(){
	lightPool.freeID(mID);
}

Light& Light::attenuation(float c0, float c1, float c2){
	mAtten[0]=c0; mAtten[1]=c1; mAtten[2]=c2; return *this;
}

Light& Light::ambient(const Color& v){ mAmbient=v; return *this; }
Light& Light::diffuse(const Color& v){ mDiffuse=v; return *this; }
Light& Light::specular(const Color& v){ mSpecular=v; return *this; }

Light& Light::dir(float x, float y, float z){
	mPos[0]=x; mPos[1]=y; mPos[2]=z; mPos[3]=0;
	return *this;
}

Light& Light::pos(float x, float y, float z){
	mPos[0]=x; mPos[1]=y; mPos[2]=z; mPos[3]=1;
	return *this;
}

} // ::al
