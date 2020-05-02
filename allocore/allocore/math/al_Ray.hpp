#ifndef INCLUDE_AL_RAY_HPP
#define INCLUDE_AL_RAY_HPP

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
	This is a simple Ray class with some intersection tests

	File author(s):
	Tim Wood, 2014, fishuyo@gmail.com
*/

#include "allocore/math/al_Vec.hpp"

namespace al {

template <class T> class Ray;
typedef Ray<float> Rayf;
typedef Ray<double> Rayd;

/// Ray for intersection tests
///
/// @ingroup allocore
template <class T>
class Ray {
public:

	Vec<3,T> o,d;		// origin and direction of ray

	Ray(){};
	Ray(Vec<3,T> origin, Vec<3,T> direction){
		set(origin,direction);
	}

	Ray& set(Vec<3,T> origin, Vec<3,T> direction){
		o = origin;
		d = direction.normalized();
		return *this;
	}

	// return point on ray
	Vec<3,T> operator()(T t){
		return o + d*t;
	}

	Vec<3,T>& origin(){ return o; }
	Vec<3,T>& direction(){ return d; }

	/// Get intersection with plane
	T intersectPlane(Vec<3,T> p0, Vec<3,T> n){
		T den = n.dot(d);
		if(den == T(0)) return -1;
		return n.dot(p0 - o) / den;
	}

	/// Get intersection with circle
	T intersectCircle(Vec<3,T> p0, Vec<3,T> n, T r){
		T den = n.dot(d);
		if(den == T(0)) return -1;
		T t = n.dot(p0 - o) / den;
		if( ((*this)(t)-p0).mag() <= r) return t;
		else return -1;
	}

	template <int N>
	static T intersectSphereBase(const Vec<N,T>& dir, const Vec<N,T>& pos, T radius){
		T A = dir.dot(dir);
		T B = T(2) * dir.dot(pos);
		T C = pos.dot(pos) - radius*radius;
		T det = B*B - T(4)*A*C;

		if(det > T(0)){
			T inv2A = T(-1) / (T(2)*A);
			T r1 =         B * inv2A;
			T r2 = sqrt(det) * inv2A;
			if(r1 >-r2) return r1 + r2; // (-B - det^1/2) / (2A) > 0
			if(r1 > r2) return r1 - r2; // (-B + det^1/2) / (2A) > 0
		} else if(det == T(0)){
			T t = B / (T(-2)*A);
			if(t > T(0)) return t;
		}
		return -1.; // negative intersection qualifies as a miss
	}

	/// Get intersection with sphere
	T intersectSphere(const Vec<3,T>& cen, T radius) const {
		return intersectSphereBase(d, o-cen, radius);
	}

	/// Returns whether ray intersects sphere
	bool intersectsSphere(const Vec<3,T>& cen, T radius) const {
		return intersectSphere(cen,radius) > 0.;
	}

	/// Get intersection with cylinder perpendicular to axis-aligned plane
	template <unsigned A1, unsigned A2>
	T intersectCylinderAA(T radius) const {
		static_assert(A1<3 && A2<3 && A1!=A2, "Invalid axes");
		return intersectSphereBase(Vec<2,T>(d[A1], d[A2]), Vec<2,T>(o[A1],o[A2]), radius);
	}

	/// Get intersection with cylinder positioned at origin oriented with Z axis
	T intersectCylinderXY(T radius) const {
		return intersectCylinderAA<0,1>(radius);
	}

	/// Get intersection with cylinder positioned at origin oriented with Y axis
	T intersectCylinderXZ(T radius) const {
		return intersectCylinderAA<2,0>(radius);
	}

	bool intersectsBox(const Vec<3,T>& cen, const Vec<3,T>& scl, float t0=0.f, float t1=9e9) const {
		// courtesy of http://www.cs.utah.edu/~awilliam/box/
		float tmin, tmax, tymin, tymax, tzmin, tzmax;

		Vec<3,T> parameters[2];
		Vec<3,T> min = cen - scl/2;
		Vec<3,T> max = cen + scl/2;
		parameters[0] = min;
		parameters[1] = max;

		Vec<3,T> inv_direction = 1.0/d;
		int sign[3];
		sign[0] = (inv_direction.x < 0);
		sign[1] = (inv_direction.y < 0);
		sign[2] = (inv_direction.z < 0);

		tmin = (parameters[sign[0]].x - o.x) * inv_direction.x;
		tmax = (parameters[1-sign[0]].x - o.x) * inv_direction.x;
		tymin = (parameters[sign[1]].y - o.y) * inv_direction.y;
		tymax = (parameters[1-sign[1]].y - o.y) * inv_direction.y;
		if ( (tmin > tymax) || (tymin > tmax) ) 
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;
		tzmin = (parameters[sign[2]].z - o.z) * inv_direction.z;
		tzmax = (parameters[1-sign[2]].z - o.z) * inv_direction.z;
		if ( (tmin > tzmax) || (tzmin > tmax) ) 
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;
		return ( (tmin < t1) && (tmax > t0) );
	}

	// intersect with the capsule shape of the AlloSphere
	// assumes the ray is originating near the center of the sphere
	// check this..
	T intersectAllosphere() const {
		T radius = 4.842f;
		T bridgeWidth2 = 2.09f / 2.;

		// intersect with bridge cylinder
		T t = intersectCylinderXY( radius );

		// if no intersection intersect with appropriate hemisphere
		if( t == -1.){
			if(d.z < 0.) return intersectSphere( Vec<3,T>(0,0,-bridgeWidth2), radius);
			else return intersectSphere( Vec<3,T>(0,0,bridgeWidth2), radius);
		}

		Vec<3,T> p = (*this)(t);
		if( p.z < -bridgeWidth2){
			return intersectSphere( Vec<3,T>(0,0,-bridgeWidth2), radius);
		} else if( p.z > bridgeWidth2 ){
			return intersectSphere( Vec<3,T>(0,0,bridgeWidth2), radius);
		} else return t;
	}

};

} //al::
#endif