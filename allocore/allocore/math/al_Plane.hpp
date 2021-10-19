#ifndef INCLUDE_AL_PLANE_HPP
#define INCLUDE_AL_PLANE_HPP

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
	A 3-dimensional plane encoded as a normal and scalar component

	File author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/


#include "allocore/math/al_Vec.hpp"

namespace al {

/// A plane in Euclidean space
///
/// @ingroup allocore
template <class T>
class Plane{
public:

	typedef al::Vec<3,T> Vec3;
	typedef al::Vec<4,T> Vec4;

	Plane(): mNormal(1,0,0), mD(0){}
	Plane(const Vec3& v1, const Vec3& v2, const Vec3& v3);

	/// Get normal perpendicular to plane (a, b, and c components)
	const Vec3& normal() const { return mNormal; }

	/// Get scalar component of plane equation
	T d() const { return mD; }

	/// Get plane as 4-vector (Nx, Ny, Nz, d)
	Vec4 vec() const { return {mNormal, mD}; }

	/// Returns distance from plane to point (measured relative to plane normal)
	T distance(const Vec3& p) const { return (mD + mNormal.dot(p)); }

	/// Returns whether a point is in the negative half-space of the plane
	bool inNegativeSpace(const Vec3& p) const { return mNormal.dot(p) < -d(); }

	/// Returns whether a point is in the positive half-space of the plane
	bool inPositiveSpace(const Vec3& p) const { return mNormal.dot(p) >=-d(); }

	/// Set from three points lying on the plane

	/// The normal is computed according to a right-handed coordinate system.
	/// For left-handed, swap the first and last vertices.
	Plane& from3Points(const Vec3& v1, const Vec3& v2, const Vec3& v3);

	/// Set plane from a unit length normal and point lying on the plane
	Plane& fromNormalAndPoint(const Vec3& normal, const Vec3& point);

	/// Set plane from coefficients
	Plane& fromCoefficients(T a, T b, T c, T d);

protected:
	Vec3 mNormal;	// plane orientation as perp. unit vector
	T mD;			// plane position as translation factor along normal
};


template <class T>
Plane<T>::Plane(const Vec3& v1, const Vec3& v2, const Vec3& v3){
	from3Points(v1,v2,v3);
}

template <class T>
Plane<T>& Plane<T>::from3Points(const Vec3& v1, const Vec3& v2, const Vec3& v3){
//	return fromNormalAndPoint(cross(v1-v2, v3-v2).normalize(), v2); // left-handed
	return fromNormalAndPoint(cross(v3-v2, v1-v2).normalize(), v2); // right-handed
}

template <class T>
Plane<T>& Plane<T>::fromNormalAndPoint(const Vec3& nrm, const Vec3& point){
	mNormal = nrm;
	mD = -(mNormal.dot(point));
	return *this;
}

template <class T>
Plane<T>& Plane<T>::fromCoefficients(T a, T b, T c, T d){
	mNormal(a,b,c);
	T l = mNormal.mag();
	mNormal(a/l,b/l,c/l);
	mD = d/l;
	return *this;
}

} // ::al::

#endif
