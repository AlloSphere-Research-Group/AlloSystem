#ifndef INCLUDE_AL_GRAPHICS_LIGHT_HPP
#define INCLUDE_AL_GRAPHICS_LIGHT_HPP

#include "types/al_Color.hpp"
#include "graphics/al_Common.hpp"

namespace al {
namespace gfx{


/// Material properties

/// These should be set before rendering the vertices of the object they
/// are to be applied to.
class Material{
public:

	Material(Face::t f=Face::Front);

	/// Send current material settings to GPU
	void operator()() const;

	/// Set the polygon face that material will be applied to
	Material& face(gfx::Face::t f);

	/// Set specular exponent [0, 128]
	Material& shininess(float v);

	Material& ambientAndDiffuse(const Color& v);
	Material& ambient(const Color& v);
	Material& diffuse(const Color& v);
	Material& emission(const Color& v);
	Material& specular(const Color& v);

protected:
	Color mAmbient;
	Color mDiffuse;
	Color mEmission;
	Color mSpecular;
	float mShine;
	int mFace;
};


class Light{
public:
	Light(float x=5, float y=2, float z=5);
	~Light();

	/// Send current light settings to GPU
	void operator()() const;

	/// Attenuation factor = 1/(c0 + c1*d + c2*d*d)
	Light& attenuation(float c0, float c1=0, float c2=0);
	Light& ambient(const Color& v);
	Light& diffuse(const Color& v);
	Light& specular(const Color& v);
	
	Light& dir(float x, float y, float z);

	template <class VEC3>
	Light& dir(const VEC3& v){ return dir(v[0], v[1], v[2]); }

	Light& pos(float x, float y, float z);

	template <class VEC3>
	Light& pos(const VEC3& v){ return pos(v[0], v[1], v[2]); }

	const Color& ambient() const { return mAmbient; }
	const Color& diffuse() const { return mDiffuse; }
	const Color& specular() const { return mSpecular; }

	/// Determines how global specular reflection angles are computed
	static void localViewer(bool v);
	
	/// Determines whether global lighting is two-sided
	static void twoSided(bool v);

protected:
	int mID;
	Color mAmbient;
	Color mDiffuse;
	Color mSpecular;
	float mPos[4];
	float mAtten[3];
};

} // ::al::gfx
} // ::al

#endif
