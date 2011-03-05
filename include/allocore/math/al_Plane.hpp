#ifndef INCLUDE_AL_PLANE_HPP
#define INCLUDE_AL_PLANE_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "allocore/math/al_Vec.hpp"

namespace al {

/// A plane in Euclidean space
template <class T>
class Plane{
public:

	typedef al::Vec<3,T> Vec3;

	Plane(): mNormal(1,0,0), mD(0){}
	Plane(const Vec3& v1, const Vec3& v2, const Vec3& v3);

	/// Get normal perpendicular to plane (a, b, and c components)
	const Vec3& normal() const { return mNormal; }

	/// Get scalar component of plane equation
	T d() const { return mD; }

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
