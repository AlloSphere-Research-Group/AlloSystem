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


	T intersectPlane(Vec<3,T> p0, Vec<3,T> n){
		T den = n.dot(d);
		if(den == 0) return -1;
		return n.dot(p0 - o) / den;
	}

	T intersectCircle(Vec<3,T> p0, Vec<3,T> n, T r){
		T den = n.dot(d);
		if(den == 0) return -1;
		T t = n.dot(p0 - o) / den;
		if( ((*this)(t)-p0).mag() <= r) return t;
		else return -1;
	}

	// intersect sphere
	T intersectSphere( Vec<3,T> cen, T radius ){
		Vec<3,T> o_c = o - cen;
	    T A = d.dot(d);
	    T B = 2. * ( d.dot( o_c ));
	    T C = (o_c.dot(o_c)) - radius*radius;
	    T det = B*B - 4*A*C;

	    if( det > 0. ){
	      T t1 = (-B - sqrt(det) ) / (2.*A);
	      if ( t1 > 0. ) return t1;
	      T t2 = (-B + sqrt(det) ) / (2.*A);
	      if ( t2 > 0. ) return t2;

	    } else if ( det == 0. ){
	      T t = -B / (2.*A);
	      if ( t > 0. ) return t;
	    }
		return -1.; // will be ignoring negative intersections -1 qualifies a miss
	}

	bool intersectsSphere( Vec<3,T> cen, T radius){
		return intersectSphere(cen,radius) > 0.;
	}

	bool intersectsBox(Vec<3,T> cen, Vec<3,T> scl, float t0=0.f, float t1 = 9e9){
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

	// intersect cylinder positioned at origin oriented with Z axis
	T intersectCylinderXY( T radius ){

	    T A = d.x*d.x + d.y*d.y;
	    T B = 2. * (d.x*o.x + d.y*o.y);
	    T C = (o.x*o.x + o.y*o.y) - radius*radius;
	    T det = B*B - 4*A*C;

	    if( det > 0. ){
	      T t1 = (-B - sqrt(det) ) / (2.*A);
	      if ( t1 > 0. ) return t1;
	      T t2 = (-B + sqrt(det) ) / (2.*A);
	      if ( t2 > 0. ) return t2;

	    } else if ( det == 0. ){
	      T t = -B / (2.*A);
	      if ( t > 0. ) return t;
	    }
		return -1.; // will be ignoring negative intersections so this is ok for now
	}

	// intersect cylinder positioned at origin oriented with Y axis
	T intersectCylinderXZ( T radius ){

	    T A = d.x*d.x + d.z*d.z;
	    T B = 2. * (d.x*o.x + d.z*o.z);
	    T C = (o.x*o.x + o.z*o.z) - radius*radius;
	    T det = B*B - 4*A*C;

	    if( det > 0. ){
	      T t1 = (-B - sqrt(det) ) / (2.*A);
	      if ( t1 > 0. ) return t1;
	      T t2 = (-B + sqrt(det) ) / (2.*A);
	      if ( t2 > 0. ) return t2;

	    } else if ( det == 0. ){
	      T t = -B / (2.*A);
	      if ( t > 0. ) return t;
	    }
		return -1.; // will be ignoring negative intersections so this is ok for now
	}

	// intersect with the capsule shape of the AlloSphere
	// assumes the ray is originating near the center of the sphere
	// check this..
	T intersectAllosphere(){
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
