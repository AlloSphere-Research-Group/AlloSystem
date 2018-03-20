#ifndef INCLUDE_AL_GRAPHICS_LIGHT_HPP
#define INCLUDE_AL_GRAPHICS_LIGHT_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Wrapper to OpenGL lighting & materials

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <string>

#include "allocore/math/al_Vec.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

/// Material properties

/// These should be set before rendering the vertices of the object they
/// are to be applied to.
/// @ingroup allocore
class Material {
public:
	Material();

	/// @param[in] faceDir	Face this material applies to (see Graphics::Direction)
	Material(int faceDir);

	/// Send current material settings to GPU
	void operator()() const;

	/// Set the polygon face that material will be applied to

	/// @param[in] faceDir	Face this material applies to (see Graphics::Direction)
	///
	Material& face(int faceDir);

	/// Set specular exponent [0, 128]
	Material& shininess(float v);
	Material& opticalDensity(float v) { mOpticalDensity=v; return *this; }
	Material& illumination(float v) { mIllumination=v; return *this; }

	Material& ambientAndDiffuse(const Color& v);
	Material& ambient(const Color& v);
	Material& diffuse(const Color& v);
	Material& emission(const Color& v);
	Material& specular(const Color& v);

	Material& ambientMap(const std::string& map) { mMapKa = map; return *this; }
	Material& specularMap(const std::string& map) { mMapKs = map; return *this; }
	Material& diffuseMap(const std::string& map) { mMapKd = map; return *this; }
	Material& bumpMap(const std::string& map) { mMapBump = map; return *this; }
	Material& useColorMaterial(bool v) { mUseColorMaterial = v; return *this; }

	int face() const { return mFace; }

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
	bool useColorMaterial() const { return mUseColorMaterial; }

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


/// Light
/// @ingroup allocore
class Light{
public:
	Light(float x=0, float y=0, float z=1);
	~Light();

	/// Send current light settings to GPU
	void operator()() const;

	/// Set positional light position
	Light& pos(float x, float y, float z);

	/// Set positional light position
	template <class Vec3>
	Light& pos(const Vec3& v){ return pos(v[0],v[1],v[2]); }

	/// Set directional light direction
	Light& dir(float x, float y, float z);

	/// Set directional light direction
	template <class Vec3>
	Light& dir(const Vec3& v){ return dir(v[0],v[1],v[2]); }

	Light& ambient(const Color& v);
	Light& diffuse(const Color& v);
	Light& specular(const Color& v);

	Light& strength(float v);

	/// Attenuation factor = 1/(c0 + c1*d + c2*d*d)
	Light& attenuation(float c0, float c1=0, float c2=0);

	/// Set distance at which light is at half intensity
	Light& halfDist(float v);

	/// Set cone spread

	/// This is the angle of conical spread, in degress.
	/// The special value of 180 produces a uniform distribution.
	Light& spread(float v);

	/// Set spotlight parameters

	/// @param[in] xDir		x direction
	/// @param[in] yDir		y direction
	/// @param[in] zDir		z direction
	/// @param[in] cutoff	angle of the cone light emitted by the spot; [0, 90], 180 (uniform)
	/// @param[in] expo		the intensity distribution of the light; [0, 128]
	Light& spot(float xDir, float yDir, float zDir, float cutoff, float expo=15);

	template <class VEC3>
	Light& spot(const VEC3& v, float cutoff, float expo=15){
		return spot(v[0],v[1],v[2],cutoff,expo);
	}

	/// Set global ambient component (default is {0.2, 0.2, 0.2, 1})
	static void globalAmbient(const Color& v);

	/// Determines whether global lighting is two-sided (default is false)

	/// Setting this to true effectively reverses normals of back-facing
	/// polygons.
	static void twoSided(bool v);


	/// Get position
	const Vec3f& pos() const { return mPos; }
	Vec3f& pos(){ return mPos; }

	/// Get direction
	const Vec3f& dir() const { return mDir; }
	Vec3f& dir(){ return mDir; }

	/// Get attenuation array
	const float * attenuation() const { return mAtten; }
	float * attenuation(){ return mAtten; }

	float strength() const { return mStrength; }

	/// Get ambient color
	const Color& ambient() const { return mAmbient; }
	Color& ambient(){ return mAmbient; }

	/// Get diffuse color
	const Color& diffuse() const { return mDiffuse; }
	Color& diffuse(){ return mDiffuse; }

	/// Get specular color
	const Color& specular() const { return mSpecular; }
	Color& specular(){ return mSpecular; }

	/// Get attenuation half distance
	float halfDist() const { return mHalfDist; }

	/// Get cone spread
	float spread() const { return mSpread; }

	/// Get global ambient component
	static const Color& globalAmbient(){ return sGlobalAmbient; }

	/// Get whether lighting is two-sided
	static bool twoSided(){ return sTwoSided; }

	/// Unique ID of light
	int id() const;

	int index() const;

protected:
	static Color sGlobalAmbient;
	static bool sGlobalAmbientUpdate;
	static bool sTwoSided;
	static bool sTwoSidedUpdate;
	int mIndex;
	Vec3f mPos{0,0,1}, mDir{0,0,-1};
	Color mAmbient{0};
	Color mDiffuse{1};
	Color mSpecular{1};
	float mStrength{1};
	float mSpread{180};
	float mHalfDist{1e16};
	float mAtten[3] = {1,0,0};
	bool mIsDir = false;

	friend class Graphics;
	void submitCol(int lightID) const;
	void submitPos(int lightID) const;
};

} // ::al

#endif
