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

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/types/al_Color.hpp"

namespace al{

/// Material properties

/// These should be set before rendering the vertices of the object they
/// are to be applied to.
/// @ingroup allocore
class Material {
public:

	Material(Graphics::Direction faceDir=Graphics::FRONT);

	/// Send current material settings to GPU
	void operator()() const;

	/// Set the polygon face that material will be applied to
	Material& face(Graphics::Direction faceDir);

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

	Graphics::Direction face() const { return mFace; }

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
	Graphics::Direction mFace;
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


	/// Get position array
	const float * pos() const { return mPos; }
	float * pos(){ return mPos; }

	/// Get attenuation array
	const float * attenuation() const { return mAtten; }
	float * attenuation(){ return mAtten; }

	/// Get ambient color
	const Color& ambient() const { return mAmbient; }
	Color& ambient(){ return mAmbient; }

	/// Get diffuse color
	const Color& diffuse() const { return mDiffuse; }
	Color& diffuse(){ return mDiffuse; }

	/// Get specular color
	const Color& specular() const { return mSpecular; }
	Color& specular(){ return mSpecular; }


	/// Set global ambient light intensity (default is {0.2, 0.2, 0.2, 1})
	static void globalAmbient(const Color& v);

	/// Determines whether global lighting is two-sided

	/// Setting this to true effectively reverses normals of back-facing
	/// polygons.
	static void twoSided(bool v);

protected:
	int mID;
	Color mAmbient;
	Color mDiffuse;
	Color mSpecular;
	float mPos[4];
	float mAtten[3];
};

} // ::al

#endif
