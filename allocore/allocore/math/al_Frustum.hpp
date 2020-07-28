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

	enum{ TOP=0, BOTTOM, LEFT, RIGHT, NEARP, FARP };
	enum{ OUTSIDE=0, INTERSECT, INSIDE };

	Vec<3,T> ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;	///< Corners
	Plane<T> pl[6];										///< Faces


	const Vec<3,T>& corner(int i) const { return (&ntl)[i]; }
	Vec<3,T>& corner(int i){ return (&ntl)[i]; }

	const Vec<3,T>& corner(int i0, int i1, int i2) const {
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
	Vec<3,T> getPoint(const Vec3& frac) const {
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
	Vec<3,T> getPoint(const U& fracx, const U& fracy, const U& fracz) const {
		return getPoint(Vec<3,U>(fracx,fracy,fracz));
	}

	/// Test whether point is in frustum
	
	/// \returns OUTSIDE, INTERSECT or INSIDE
	///
	int testPoint(const Vec<3,T>& p) const;

	/// Test whether sphere is in frustum

	/// \returns OUTSIDE, INTERSECT or INSIDE
	///
	int testSphere(const Vec<3,T>& center, float radius) const;

	/// Test whether axis-aligned box is in frustum

	/// This will always tell you if the box is in or intersects the frustum.
	/// Sometimes, boxes that are really outside will not be detected as so,
	/// thus returning a false positive.
	///
	/// \returns OUTSIDE, INTERSECT or INSIDE
	int testBox(const Vec<3,T>& xyz, const Vec<3,T>& dim) const;

	/// Get axis-aligned bounding box
	template <class Vec3>
	void boundingBox(Vec3& xyz, Vec3& dim) const;

	/// Returns center of frustum
	Vec<3,T> center() const { return (ntl+ntr+nbl+nbr+ftl+ftr+fbl+fbr)*0.125; }


	/// Compute planes based on frustum corners (planes face to inside)

	/// This must be called if any of the corners change value.
	///	The plane normals are computed assuming a right-hand coordinate system.
	void computePlanes();

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
	computePlanes();
	return *this;
}

template <class T>
template <class Mat4>
Frustum<T>& Frustum<T>::fromInverseMVP(const Mat4& invMVP){
	static const Vec4f bb[8] = {
		{-1.f, 1.f,-1.f, 1.f}, { 1.f, 1.f,-1.f, 1.f}, {-1.f,-1.f,-1.f, 1.f}, { 1.f,-1.f,-1.f, 1.f},
		{-1.f, 1.f, 1.f, 1.f}, { 1.f, 1.f, 1.f, 1.f}, {-1.f,-1.f, 1.f, 1.f}, { 1.f,-1.f, 1.f, 1.f}
	};
	Vec<3,T> corners[8];
	for(unsigned i=0; i<8; ++i){
		auto c = invMVP * bb[i];
		corners[i] = sub<3>(c) / c.w;
	}
	return fromCorners(corners);
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
void Frustum<T>::computePlanes(){
	pl[TOP   ].from3Points(ntr,ntl,ftl);
	pl[BOTTOM].from3Points(nbl,nbr,fbr);
	pl[LEFT  ].from3Points(ntl,nbl,fbl);
	pl[RIGHT ].from3Points(nbr,ntr,fbr);
	pl[NEARP ].from3Points(ntl,ntr,nbr);
	pl[FARP  ].from3Points(ftr,ftl,fbl);
}

template <class T>
int Frustum<T>::testPoint(const Vec<3,T>& p) const {
	for(const auto& plane : pl){
		if(plane.inNegativeSpace(p)) return OUTSIDE;
	}
	return INSIDE;
}

template <class T>
int Frustum<T>::testSphere(const Vec<3,T>& c, float r) const {
	int result = INSIDE;
	for(const auto& plane : pl){
		auto distance = plane.distance(c);
		if(distance < -r)		return OUTSIDE;
		else if(distance < r)	result = INTERSECT;
	}
	return result;
}

template <class T>
int Frustum<T>::testBox(const Vec<3,T>& xyz, const Vec<3,T>& dim) const {
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
		auto vp = xyz;
		if(plNrm[0] > 0) vp[0] += dim[0];
		if(plNrm[1] > 0) vp[1] += dim[1];
		if(plNrm[2] > 0) vp[2] += dim[2];
		if(plane.inNegativeSpace(vp)) return OUTSIDE;

		// Is negative vertex outside?
		auto vn = xyz;
		if(plNrm[0] < 0) vn[0] += dim[0];
		if(plNrm[1] < 0) vn[1] += dim[1];
		if(plNrm[2] < 0) vn[2] += dim[2];
		if(plane.inNegativeSpace(vn)) result = INTERSECT;
	}
	return result;
}

} // al::

#endif
