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

// Ray for intersection tests
template <class T>
class Ray {
public:

	Vec<3,T> o,d;		// origin and direction of ray

	Ray(){};
	Ray(Vec<3,T> origin, Vec<3,T> direction){
		o.set(origin);
		d.set(direction.normalize());
	}

	// return point on ray
	Vec<3,T> operator()(T t){
		return o + d*t;
	}

	Vec<3,T>& origin(){ return o; }
	Vec<3,T>& dir(){ return d; }

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
