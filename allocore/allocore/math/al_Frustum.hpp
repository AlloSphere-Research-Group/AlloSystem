#ifndef INCLUDE_AL_FRUSTUM_HPP
#define INCLUDE_AL_FRUSTUM_HPP

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
	This is a rectangular frustum useful for computer graphics

	File author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/

#include "allocore/math/al_Plane.hpp"
#include "allocore/math/al_Vec.hpp"


namespace al {

template <class T> class Frustum;
typedef Frustum<double> Frustumd;	///< Double precision frustrum


/// Rectangular frustum

/// A frustum has the shape of a four-sided pyramid truncated at the top.
/// For graphics, this specifies the viewing frustum of a camera.
///
/// Source code adapted from:
/// "OpenGL @ Lighthouse 3D - View Frustum Culling Tutorial",
/// http://www.lighthouse3d.com/opengl/viewfrustum/index.php?intro
///
/// @ingroup allocore
template <class T>
class Frustum{
public:

	typedef T value_type;
	typedef Vec<3,T> vec;

	enum{ TOP=0, BOTTOM, LEFT, RIGHT, NEARP, FARP };
	enum{ OUTSIDE=0, INTERSECT, INSIDE };

	vec ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr; ///< Corners
	Plane<T> pl[6]; ///< Face planes (normals point inward)


	const vec& corner(int i) const { return (&ntl)[i]; }
	vec& corner(int i){ return (&ntl)[i]; }

	const vec& corner(int i0, int i1, int i2) const {
		return corner(i2<<2 | i1<<1 | i0);
	}


	/// Set from eight corners
	template <class Vec>
	Frustum& fromCorners(const Vec * corners);

	/// Set from inverse model-view-projection matrix
	template <class Mat4>
	Frustum& fromInverseMVP(const Mat4& invMVP);


	/// Get point in frustum corresponding to fraction along edges
	template <class Vec3>
	vec getPoint(const Vec3& frac) const {
		return
		lerp(frac[2],
			lerp(frac[1],
				lerp(frac[0], corner(0,0,0), corner(1,0,0)),
				lerp(frac[0], corner(0,1,0), corner(1,1,0))
			),
			lerp(frac[1],
				lerp(frac[0], corner(0,0,1), corner(1,0,1)),
				lerp(frac[0], corner(0,1,1), corner(1,1,1))
			)
		);
	}

	/// Get point in frustum corresponding to fraction along edges
	template <class U>
	vec getPoint(const U& fracx, const U& fracy, const U& fracz) const {
		return getPoint(Vec<3,U>(fracx,fracy,fracz));
	}

	/// Get center of near plane
	vec centerNear() const { return (ntr+ntl+nbl+nbr)*0.25; }

	/// Get center of far plane
	vec centerFar() const { return (ftr+ftl+fbl+fbr)*0.25; }

	/// Get center of frustum
	vec center() const { return (ntl+ntr+nbl+nbr+ftl+ftr+fbl+fbr)*0.125; }

	/// Test whether point is in frustum
	
	/// \returns OUTSIDE, INTERSECT or INSIDE
	///
	int testPoint(const vec& p) const;

	/// Test whether sphere is in frustum

	/// \returns OUTSIDE, INTERSECT or INSIDE
	///
	int testSphere(const vec& center, float radius) const;

	/// Test whether axis-aligned box is in frustum

	/// This will always tell you if the box is in or intersects the frustum.
	/// Sometimes, boxes that are really outside will not be detected as so,
	/// thus returning a false positive.
	///
	/// @param[in] min	minimum corner of box
	/// @param[in] ext	extents (diameters) of box
	/// \returns OUTSIDE, INTERSECT or INSIDE
	int testBox(const vec& min, const vec& ext) const;
	int testBoxMinMax(const vec& min, const vec& max) const;

	/// Get axis-aligned bounding box
	template <class Vec3>
	void boundingBox(Vec3& xyz, Vec3& dim) const;


	/// Compute planes based on frustum corners (planes face to inside)

	/// This must be called if any of the corners change value.
	///
	Frustum& computePlanes();

private:
	template <class Tf, class Tv>
	static Tv lerp(Tf f, const Tv& x, const Tv& y){
		return (y - x) * f + x;
	}
};



template <class T>
template <class Vec>
Frustum<T>& Frustum<T>::fromCorners(const Vec * corners){
	for(unsigned i=0; i<8; ++i){
		for(unsigned k=0; k<3; ++k){
			corner(i)[k] = corners[i][k];
		}
	}
	return computePlanes();
}

template <class T>
template <class Mat4>
Frustum<T>& Frustum<T>::fromInverseMVP(const Mat4& invMVP){
	static const Vec3f bb[8] = {
		{-1.f, 1.f,-1.f}, { 1.f, 1.f,-1.f}, {-1.f,-1.f,-1.f}, { 1.f,-1.f,-1.f},
		{-1.f, 1.f, 1.f}, { 1.f, 1.f, 1.f}, {-1.f,-1.f, 1.f}, { 1.f,-1.f, 1.f}
	};
	for(unsigned i=0; i<8; ++i){
		auto c = invMVP * Vec4f(bb[i],1.f);
		corner(i) = c.xyz() / c.w;
	}
	return computePlanes();
}

template <class T>
template <class Vec3>
void Frustum<T>::boundingBox(Vec3& xyz, Vec3& dim) const {
	auto vmin = corner(0);
	auto vmax = vmin;

	for(int i=1; i<8; ++i){
		auto v = corner(i);
		vmin = min(vmin, v);
		vmax = max(vmax, v);
	}

	xyz = vmin;
	dim = vmax - vmin;
}

template <class T>
Frustum<T>& Frustum<T>::computePlanes(){
	pl[TOP   ].from3Points(ntr,ntl,ftl);
	pl[BOTTOM].from3Points(nbl,nbr,fbr);
	pl[LEFT  ].from3Points(ntl,nbl,fbl);
	pl[RIGHT ].from3Points(nbr,ntr,fbr);
	pl[NEARP ].from3Points(ntl,ntr,nbr);
	pl[FARP  ].from3Points(ftr,ftl,fbl);
	return *this;
}

template <class T>
int Frustum<T>::testPoint(const vec& p) const {
	for(const auto& plane : pl){
		if(plane.inNegativeSpace(p)) return OUTSIDE;
	}
	return INSIDE;
}

template <class T>
int Frustum<T>::testSphere(const vec& c, float r) const {
	int result = INSIDE;
	for(const auto& plane : pl){
		auto distance = plane.distance(c);
		if(distance < -r)		return OUTSIDE;
		else if(distance < r)	result = INTERSECT;
	}
	return result;
}

template <class T>
int Frustum<T>::testBoxMinMax(const vec& min, const vec& max) const {
	int result = INSIDE;
	for(const auto& plane : pl){
		const auto plNrm = plane.normal();
/*
		The positive vertex is the vertex from the box that is further along
		the normal's direction. The negative vertex is the opposite vertex.

		If the p-vertex is on the wrong side of the plane, the box can be
		immediately rejected, as it falls completely outside the frustum. On the
		other hand, if the p-vertex is on the right side of the plane, then
		testing the whereabouts of the n-vertex tells if the box is totally on
		the right side of the plane, or if the box intersects the plane.
*/
		// Is positive vertex outside?
		vec vp;
		for(int i=0; i<3; ++i) vp[i] = plNrm[i]>T(0) ? max[i] : min[i];
		if(plane.inNegativeSpace(vp)) return OUTSIDE;

		// Is negative vertex outside?
		vec vn;
		for(int i=0; i<3; ++i) vn[i] = plNrm[i]<T(0) ? max[i] : min[i];
		if(plane.inNegativeSpace(vn)) result = INTERSECT;
	}
	return result;
}

template <class T>
int Frustum<T>::testBox(const vec& min, const vec& ext) const {
	return testBoxMinMax(min, min+ext);
}

} // al::

#endif
