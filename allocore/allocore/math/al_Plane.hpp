#ifndef INC_AL_PLANE_HPP
#define INC_AL_PLANE_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	A 3-dimensional plane encoded as a normal and scalar component

	Author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/

#include "allocore/math/al_Vec.hpp"

namespace al {

/// A plane in Euclidean space
///
/// \ingroup allocore
template <class T>
class Plane{
public:

	typedef al::Vec<3,T> Vec3;
	typedef al::Vec<4,T> Vec4;

	Plane(): mNormal(1,0,0), mD(0){}
	Plane(const Vec3& v1, const Vec3& v2, const Vec3& v3);

	/// Get normal perpendicular to plane (points in positive half-space direction)
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

	/// The normal is computed according to a right-handed coordinate system
	/// with a counter-clockwise vertex winding order.
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

} // al::
#endif
