#ifndef INCLUDE_AL_GRAPHICS_LIGHT_HPP
#define INCLUDE_AL_GRAPHICS_LIGHT_HPP

#include <string>

#include "types/al_Color.hpp"
#include "graphics/al_Common.hpp"

namespace al {
namespace gfx{


/// Material properties

/// These should be set before rendering the vertices of the object they
/// are to be applied to.
class Material {
public:

	Material(Face::t f=Face::Front);

	/// Send current material settings to GPU
	void operator()() const;

	/// Set the polygon face that material will be applied to
	Material& face(gfx::Face::t f);

	/// Set specular exponent [0, 128]
	Material& shininess(float v);
	Material& opticalDensity(float v) { mOpticalDensity=v; return *this; }
	Material& illumination(float v) { mIllumination=v; return *this; }

	Material& ambientAndDiffuse(const Color& v);
	Material& ambient(const Color& v);
	Material& diffuse(const Color& v);
	Material& emission(const Color& v);
	Material& specular(const Color& v);

	Material& ambientMap(std::string map) { mMapKa = map; return *this; }
	Material& specularMap(std::string map) { mMapKs = map; return *this; }
	Material& diffuseMap(std::string map) { mMapKd = map; return *this; }
	Material& bumpMap(std::string map) { mMapBump = map; return *this; }
	Material& useColorMaterial(bool v) { mUseColorMaterial = v; return *this; }

	float shininess() const { return mShine; }
	float opticalDensity() const { return mOpticalDensity; }
	float illumination() const { return mIllumination; }
	const Color& ambient() const { return mAmbient; }
	const Color& diffuse() const  { return mDiffuse; }
	const Color& emission() const { return mEmission; }
	const Color& specular() const { return mSpecular; }

	const std::string& ambientMap() const { return mMapKa; }
	const std::string& specularMap() const { return mMapKs; }
	const std::string& diffuseMap() const { return mMapKd; }
	const std::string& bumpMap() const { return mMapBump; }
	const bool useColorMaterial() const { return mUseColorMaterial; }

protected:
	Color mAmbient;
	Color mDiffuse;
	Color mEmission;
	Color mSpecular;
	float mShine, mOpticalDensity, mIllumination;
	int mFace;
	std::string mMapKa, mMapKs, mMapKd, mMapBump;
	bool mUseColorMaterial;
};


class Light{
public:
	Light(float x=5, float y=2, float z=-5);
	~Light();

	/// Send current light settings to GPU
	void operator()() const;

	/// Attenuation factor = 1/(c0 + c1*d + c2*d*d)
	Light& attenuation(float c0, float c1=0, float c2=0);
	Light& ambient(const Color& v);
	Light& diffuse(const Color& v);
	Light& specular(const Color& v);

	/// Set directional light direction
	Light& dir(float x, float y, float z);

	/// Set directional light direction
	template <class VEC3>
	Light& dir(const VEC3& v){ return dir(v[0],v[1],v[2]); }

	/// Set positional light position
	Light& pos(float x, float y, float z);

	/// Set positional light position
	template <class VEC3>
	Light& pos(const VEC3& v){ return pos(v[0],v[1],v[2]); }

	/// Set spotlight parameters

	/// @param[in] xDir		x direction
	/// @param[in] yDir		y direction
	/// @param[in] zDir		z direction
	/// @param[in] cutoff	angle of the cone light emitted by the spot; [0, 90], 180 (uniform)
	/// @param[in] expo		the intensity distribution of the light; [0, 128]
	Light& spot(float xDir, float yDir, float zDir, float cutoff, float expo=15);

	template <class VEC3>
	Light& spot(const VEC3& v, float cutoff, float expo=15){ return spot(v[0],v[1],v[2],cutoff,expo); }

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
