#ifndef INCLUDE_AL_QUAT_HPP
#define INCLUDE_AL_QUAT_HPP

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
	A quaternion including many different conversion routines to matrices, etc.

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Wesley Smith, 2010, wesley.hoke@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Mat.hpp"
#include "allocore/math/al_Vec.hpp"


namespace al {


template<class T> class Quat;

typedef Quat<float>		Quatf;	///< Single-precision quaternion
typedef Quat<double>	Quatd;	///< Double-precision quaternion


/// Quaternion
template<typename T=double>
class Quat {
public:

	union{
		struct{
			T w;			///< w component
			T x;			///< x component
			T y;			///< y component
			T z;			///< z component
		};
		T components[4];	///< component vector
	};


	///
	Quat(const T& w = T(1), const T& x = T(0), const T& y = T(0), const T& z = T(0))
	:	w(w), x(x), y(y), z(z){}

	/// @param[in] v		quaternion to set values from
	template <class U>
	Quat(const Quat<U>& v)
	:	w(v.w), x(v.x), y(v.y), z(v.z){}
	
	/// Constructor of 'pure' quaternion
	template <class U>
	Quat(const Vec<3,U>& v)
	:	w(T(0)), x(v[0]), y(v[1]), z(v[2]){} 



	// Factories

	/// Calculate the rotation required to move from unit vector src to unit vector dst

	/// NOTE: both arguments must be UNIT VECTORS (normalized)
	/// rotation occurs around the axis created by the cross product of src and dst
	/// if the vectors are nearly opposing, the Y axis is used instead 
	/// if the Y axis isn't suitable, the Z axis is used instead
	/// 
	/// a typical use case: rotate object A to face object B:
	/// Vec3d src = Vec3d(A.quat().toVectorZ()).normalize();
	/// Vec3d dst = Vec3d(B.pos() - A.pos()).normalize();
	/// Quatd rot = Quatd::getRotationTo(src, dst);
	/// A.quat() = rot * A.quat();
	static Quat getRotationTo(const Vec<3,T>& usrc, const Vec<3,T>& udst);

	/// Returns identity
	static Quat identity(){ return Quat(1,0,0,0); }

	/// Get rotor from two unit vectors
	
	/// Alternatively expressed as Q = (1+gp(v1, v2))/sqrt(2*(1+dot(b, a))).
	///
	static Quat rotor(const Vec<3,T>& v1, const Vec<3,T>& v2);

	///	Spherical linear interpolation of a quaternion

	/// @param[in] from		The quaternion to interpolate from
	///	@param[in] to		The quaternion to interpolate to
	///	@param[in] amt		The amount to interpolate, range [0, 1]
	///	\returns resulting interpolated quaternion
	static Quat slerp(const Quat& from, const Quat& to, T amt);

	/// Fill an array of Quats with a full spherical interpolation
	static void slerpBuffer(const Quat& from, const Quat& to, Quat<T> * buffer, int numFrames);



	/// Set component with index
	T& operator[](int i){ return components[i];}

	/// Get component with index
	const T& operator[](int i) const { return components[i]; }

	/// Returns true if all components are equal
	bool operator ==(const Quat& v) const { return (w==v.w) && (x==v.x) && (y==v.y) && (z==v.z); }

	/// Returns true if any components are not equal
	bool operator !=(const Quat& v) const {  return !(*this == v); }

	Quat operator - () const { return Quat(-w, -x, -y, -z); }
	Quat operator + (const Quat& v) const { return Quat(*this)+=v; }
	Quat operator + (const    T& v) const { return Quat(*this)+=v; }
	Quat operator - (const Quat& v) const { return Quat(*this)-=v; }
	Quat operator - (const    T& v) const { return Quat(*this)-=v; }
	Quat operator / (const Quat& v) const { return Quat(*this)/=v; }
	Quat operator / (const    T& v) const { return Quat(*this)/=v; }
	Quat operator * (const Quat& v) const { return Quat(*this)*=v; }
	Quat operator * (const    T& v) const { return Quat(*this)*=v; }

	template <class U>
	Quat& operator  =(const Quat<U>& v){ return set(v); }
	Quat& operator  =(const    T& v){ return set(v); }
	Quat& operator +=(const Quat& v){ w+=v.w; x+=v.x; y+=v.y; z+=v.z; return *this; }
	Quat& operator +=(const    T& v){ w+=  v; x+=  v; y+=  v; z+=  v; return *this; }
	Quat& operator -=(const Quat& v){ w-=v.w; x-=v.x; y-=v.y; z-=v.z; return *this; }
	Quat& operator -=(const    T& v){ w-=  v; x-=  v; y-=  v; z-=  v; return *this; }
	Quat& operator *=(const Quat& v){ return set(multiply(v)); }
	Quat& operator *=(const    T& v){ w*=  v; x*=  v; y*=  v; z*=  v; return *this; }
	Quat& operator /=(const Quat& v){ return (*this) *= v.recip(); }
	Quat& operator /=(const    T& v){ w/=v; x/=v; y/=v; z/=v; return *this; }

	/// Returns the conjugate
	Quat conj() const { return Quat(w, -x, -y, -z); }

	/// Returns dot product with another quaternion
	T dot(const Quat& v) const { return w*v.w + x*v.x + y*v.y + z*v.z; }

	/// Returns inverse (same as conjugate if normalized as q^-1 = q_conj/q_mag^2)
	Quat inverse() const { return sgn().conj(); }

	/// Get magnitude
	T mag() const { return (T)sqrt(magSqr()); }

	/// Get magnitude squared
	T magSqr() const { return dot(*this); }

	/// Returns multiplicative inverse
	Quat recip() const { return conj()/magSqr(); }

	/// Returns signum, q/|q|, the closest point on unit 3-sphere
	Quat sgn() const { return Quat(*this).normalize(); }

	// assumes both are already normalized!
	Quat multiply(const Quat& q2) const;
	Quat reverseMultiply(const Quat& q2) const;

	/// Normalize magnitude to one
	Quat& normalize();

	/// Set components
	Quat& set(const T& w, const T& x, const T& y, const T& z){
		this->w = w; this->x = x; this->y = y; this->z = z;
		return *this;
	}

	/// Set from other quaternion
	template <class U>
	Quat& set(const Quat<U>& q){ return set(q.w, q.x, q.y, q.z); }

	/// Set to identity
	Quat& setIdentity(){ return (*this) = Quat::identity(); }

	/// Set as versor rotated by angle, in radians, around unit vector (ux,uy,uz)
	Quat& fromAxisAngle(const T& angle, const T& ux, const T& uy, const T& uz);

	/// Set as versor rotated by angle, in radians, around unit vector
	Quat& fromAxisAngle(const T& angle, const Vec<3,T>& axis);

	/// Set as versor rotated by angle, in radians, around x-axis
	Quat& fromAxisX(const T& angle) {
		T t2 = angle * 0.5;
		return set(cos(t2), sin(t2), T(0), T(0));
	}

	/// Set as versor rotated by angle, in radians, around y-axis
	Quat& fromAxisY(const T& angle){
		T t2 = angle * 0.5;
		return set(cos(t2), T(0), sin(t2), T(0));
	}

	/// Set as versor rotated by angle, in radians, around z-axis
	Quat& fromAxisZ(const T& angle){
		T t2 = angle * 0.5;
		return set(cos(t2), T(0), T(0), sin(t2));
	}

	/// Set as versor rotated by Euler angles, in radians
	Quat& fromEuler(const Vec<3,T>& aed) { return fromEuler(aed[0], aed[1], aed[2]); }
	Quat& fromEuler(const T& az, const T& el, const T& ba);
	
	/// Set as versor from column-major 4-by-4 projective space transformation matrix
	Quat& fromMatrix(const T * matrix);

	/// Set as versor from column-major 4-by-4 projective space transformation matrix
	Quat& fromMatrix(const Mat<4,T>& v){ return fromMatrix(v.ptr()); }

	/// Set as versor from row-major 4-by-4 projective space transformation matrix
	Quat& fromMatrixTransposed(const T * matrix);

	/// Set as versor from row-major 4-by-4 projective space transformation matrix
	Quat& fromMatrixTransposed(const Mat<4,T>& v){ return fromMatrixTransposed(v.ptr()); }

	/// Convert to coordinate frame unit vectors
	void toCoordinateFrame(Vec<3,T>& ux, Vec<3,T>& uy, Vec<3,T>& uz) const;

	/// Convert to column-major 4-by-4 projective space transformation matrix
	void toMatrix(T * matrix) const;

	/// Convert to row-major 4-by-4 projective space transformation matrix
	void toMatrixTransposed(T * matrix) const;

	/// Convert to axis-angle form, in radians
	void toAxisAngle(T& angle, T& ax, T& ay, T& az) const;

	void toAxisAngle(T& angle, Vec<3,T>& axis) const {
		toAxisAngle(angle, axis[0], axis[1], axis[2]);
	}

	/// Convert to Euler angles (azimuth, elevation, bank), in radians
	void toEuler(T& az, T& el, T& ba) const;

	/// Convert to Euler angle vector (azimuth, elevation, bank), in radians
	void toEuler(T * e) const { toEuler(e[0],e[1],e[2]); }

	/// Convert to Euler angle vector (azimuth, elevation, bank), in radians
	void toEuler(Vec<3,T> & v) const { toEuler(v[0],v[1],v[2]); }

	/// Get local x unit vector (1,0,0) in absolute coordinates
	void toVectorX(T& ax, T& ay, T& az) const;
	void toVectorX(Vec<3,T>& v) const { toVectorX(v[0],v[1],v[2]); }
	Vec<3,T> toVectorX() const { Vec<3,T> v; toVectorX(v); return v; }

	/// Get local y unit vector (0,1,0) in absolute coordinates
	void toVectorY(T& ax, T& ay, T& az) const;
	void toVectorY(Vec<3,T>& v) const { toVectorY(v[0],v[1],v[2]); }
	Vec<3,T> toVectorY() const { Vec<3,T> v; toVectorY(v); return v; }

	/// Get local z unit vector (0,0,1) in absolute coordinates
	void toVectorZ(T& ax, T& ay, T& az) const;
	void toVectorZ(Vec<3,T>& v) const { toVectorZ(v[0],v[1],v[2]); }
	Vec<3,T> toVectorZ() const { Vec<3,T> v; toVectorZ(v); return v; }

	/// Rotate vector
	/// NOTE: quaternion should be normalized for accurate results.
	Vec<3,T> rotate(const Vec<3,T>& v) const;
	
	/// This is rotation by the quaternion's conjugate
	Vec<3,T> rotateTransposed(const Vec<3,T>& v) const;

	/// Spherical interpolation
	Quat slerp(const Quat& target, T amt) const { return slerp(*this, target, amt); }

	/// In-place spherical interpolation
	void slerpTo(const Quat& target, T amt) { set(slerp(*this, target, amt)); }

	/// Get the quaternion from a given point and quaterion toward another point
	void towardPoint(const Vec<3,T>& pos, const Quat<T>& q, const Vec<3,T>& v, float amt);
		
	/// utility for debug printing:
	void print(FILE * out = stdout) const { fprintf(out, "Quat(%f, %f, %f, %f)\n", w, x, y, z); }


	static T accuracyMax(){ return 1.000001; }
	static T accuracyMin(){ return 0.999999; }
	static T eps(){ return 0.0000001; }

private:
	static T abs(T v){ return v>T(0) ? v : -v; }
};

template<class T> Quat<T> operator + (T r, const Quat<T>& q){ return  q+r; }
template<class T> Quat<T> operator - (T r, const Quat<T>& q){ return -q+r; }
template<class T> Quat<T> operator * (T r, const Quat<T>& q){ return  q*r; }
template<class T> Quat<T> operator / (T r, const Quat<T>& q){ return  q.conjugate()*(r/q.magSqr()); }





/// Implementation

template<typename T>
inline Quat<T>& Quat<T> :: normalize() {
	T unit = magSqr();
	if(unit*unit < eps()){
		// unit too close to epsilon, set to default transform
		setIdentity();
	}
	else if(unit > accuracyMax() || unit < accuracyMin()){
		(*this) *= 1./sqrt(unit);
	}
	return *this;
}

// assumes both are already normalized!
template<typename T>
inline Quat<T> Quat<T> :: multiply(const Quat<T>& q) const {
	return Quat(
		w*q.w - x*q.x - y*q.y - z*q.z,
		w*q.x + x*q.w + y*q.z - z*q.y,
		w*q.y + y*q.w + z*q.x - x*q.z,
		w*q.z + z*q.w + x*q.y - y*q.x
	);
}

template<typename T>
inline Quat<T> Quat<T> :: reverseMultiply(const Quat<T> & q) const {
	return q * (*this);
}

template<typename T>
inline Quat<T>& Quat<T> :: fromAxisAngle(const T& angle, const Vec<3,T>& axis) {
	return fromAxisAngle(angle, axis[0],axis[1],axis[2]);
}

template<typename T>
inline Quat<T>& Quat<T> :: fromAxisAngle(const T& angle, const T& ux, const T& uy, const T& uz) {
	T t2 = angle * T(0.5);
	T sinft2 = sin(t2);	
	w = cos(t2);
	x = ux * sinft2;
	y = uy * sinft2;
	z = uz * sinft2;
	return *this;
}

template<typename T>
inline Quat<T>& Quat<T>::fromEuler(const T& az, const T& el, const T& ba){
	//http://vered.rose.utoronto.ca/people/david_dir/GEMS/GEMS.html
	//Converting from Euler angles to a quaternion is slightly more tricky, as the order of operations
	//must be correct. Since you can convert the Euler angles to three independent quaternions by
	//setting the arbitrary axis to the coordinate axes, you can then multiply the three quaternions
	//together to obtain the final quaternion.

	//So if you have three Euler angles (a, b, c), then you can form three independent quaternions
	//Qx = [ cos(a/2), (sin(a/2), 0, 0)]
    //Qy = [ cos(b/2), (0, sin(b/2), 0)]
    //Qz = [ cos(c/2), (0, 0, sin(c/2))]
	//And the final quaternion is obtained by Qx * Qy * Qz.

	T c1 = cos(az * T(0.5));
	T c2 = cos(el * T(0.5));
	T c3 = cos(ba * T(0.5));
	T s1 = sin(az * T(0.5));
	T s2 = sin(el * T(0.5));
	T s3 = sin(ba * T(0.5));

	// equiv Q1 = Qy * Qx; // since many terms are zero
	T tw = c1*c2;
	T tx = c1*s2;
	T ty = s1*c2;
	T tz =-s1*s2;

	// equiv Q2 = Q1 * Qz; // since many terms are zero
	w = tw*c3 - tz*s3;
	x = tx*c3 + ty*s3;
	y = ty*c3 - tx*s3;
	z = tw*s3 + tz*c3;
	//return normalize();
	return *this;
}

template<typename T>
inline Quat<T>& Quat<T> :: fromMatrix(const T * m) {
	T trace = m[0]+m[5]+m[10];
	w = sqrt(1. + trace)*0.5;

	if(trace > 0.) {
		x = (m[9] - m[6])/(4.*w);
		y = (m[2] - m[8])/(4.*w);
		z = (m[4] - m[1])/(4.*w);
	}
	else {
		if(m[0] > m[5] && m[0] > m[10]) {
			// m[0] is greatest
			x = sqrt(1. + m[0]-m[5]-m[10])*0.5;
			w = (m[9] - m[6])/(4.*x);
			y = (m[4] + m[1])/(4.*x);
			z = (m[8] + m[2])/(4.*x);
		}
		else if(m[5] > m[0] && m[5] > m[10]) {
			// m[1] is greatest
			y = sqrt(1. + m[5]-m[0]-m[10])*0.5;
			w = (m[2] - m[8])/(4.*y);
			x = (m[4] + m[1])/(4.*y);
			z = (m[9] + m[6])/(4.*y);
		}
		else { //if(m[10] > m[0] && m[10] > m[5]) {
			// m[2] is greatest
			z = sqrt(1. + m[10]-m[0]-m[5])*0.5;
			w = (m[4] - m[1])/(4.*z);
			x = (m[8] + m[2])/(4.*z);
			y = (m[9] + m[6])/(4.*z);
		}
	}
	return *this;
}

template<typename T>
inline Quat<T>& Quat<T> :: fromMatrixTransposed(const T * m) {
	T trace = m[0]+m[5]+m[10];
	w = sqrt(1. + trace)*0.5;

	if(trace > 0.) {
		x = (m[6] - m[9])/(4.*w);
		y = (m[8] - m[2])/(4.*w);
		z = (m[1] - m[4])/(4.*w);
	}
	else {
		if(m[0] > m[5] && m[0] > m[10]) {
			// m[0] is greatest
			x = sqrt(1. + m[0]-m[5]-m[10])*0.5;
			w = (m[6] - m[9])/(4.*x);
			y = (m[1] + m[4])/(4.*x);
			z = (m[2] + m[8])/(4.*x);
		}
		else if(m[5] > m[0] && m[5] > m[10]) {
			// m[1] is greatest
			y = sqrt(1. + m[5]-m[0]-m[10])*0.5;
			w = (m[8] - m[2])/(4.*y);
			x = (m[1] + m[4])/(4.*y);
			z = (m[6] + m[9])/(4.*y);
		}
		else { //if(m[10] > m[0] && m[10] > m[5]) {
			// m[2] is greatest
			z = sqrt(1. + m[10]-m[0]-m[5])*0.5;
			w = (m[1] - m[4])/(4.*z);
			x = (m[2] + m[8])/(4.*z);
			y = (m[6] + m[9])/(4.*z);
		}
	}
	return *this;
}

template<typename T>
inline void Quat<T> :: toAxisAngle(T& aa, T& ax, T& ay, T& az) const {
	T unit = w*w;
	if(unit < accuracyMin()){ // |cos x| must always be less than or equal to 1!
		T invSinAngle = 1./sqrt(1. - unit); // = 1/sqrt(1 - cos^2(theta/2))

		aa = ::acos(w) * T(2);
		ax = x * invSinAngle;
		ay = y * invSinAngle;
		az = z * invSinAngle;
	} else {
		aa = 0;
		if(x == T(0) && y == T(0) && z == T(0)){
			// axes are 0,0,0, change to a default:
			ax = T(1);
			ay = T(0);
			az = T(0);
		}
		else{
			// for small angles, axis is roughly equal to i,j,k components
			// axes are close to zero, should be normalized:
			Vec<3,T> v(x, y, z);
			v.normalize();
			ax = v[0];
			ay = v[1];
			az = v[2];
		}
	}
}

template<typename T>
inline void Quat<T> :: toEuler(T& az, T& el, T& ba) const {
	// http://www.mathworks.com/access/helpdesk/help/toolbox/aeroblks/quaternionstoeulerangles.html
	T sqw = w*w;
	T sqx = x*x;
	T sqy = y*y;
	T sqz = z*z;
	az = asin (-2.0 * (x*z - w*y));
	el = atan2( 2.0 * (y*z + w*x), (sqw - sqx - sqy + sqz));
	ba = atan2( 2.0 * (x*y + w*z), (sqw + sqx - sqy - sqz));
}

template<typename T>
inline void Quat<T>::toCoordinateFrame(Vec<3,T>& ux, Vec<3,T>& uy, Vec<3,T>& uz) const {
	static const T _2 = T(2);
	static const T _1 = T(1);
	T	_2w=_2*w, _2x=_2*x, _2y=_2*y;
	T	wx=_2w*x, wy=_2w*y, wz=_2w*z, xx=_2x*x, xy=_2x*y,
		xz=_2x*z, yy=_2y*y, yz=_2y*z, zz=_2*z*z;

	ux[0] =-zz - yy + _1;
	ux[1] = wz + xy;
	ux[2] = xz - wy;

	uy[0] = xy - wz;
	uy[1] =-zz - xx + _1;
	uy[2] = wx + yz;

	uz[0] = wy + xz;
	uz[1] = yz - wx;
	uz[2] =-yy - xx + _1;
}

/*
Quat to matrix:
RHCS
	[ 1 - 2y - 2z    2xy + 2wz      2xz - 2wy	] 
	[											] 
	[ 2xy - 2wz      1 - 2x - 2z    2yz + 2wx	] 
	[											] 
	[ 2xz + 2wy      2yz - 2wx      1 - 2x - 2y	]

LHCS              
	[ 1 - 2y - 2z    2xy - 2wz      2xz + 2wy	] 
	[											] 
	[ 2xy + 2wz      1 - 2x - 2z    2yz - 2wx	] 
	[											] 
	[ 2xz - 2wy      2yz + 2wx      1 - 2x - 2y	]
*/

template<typename T>
inline void Quat<T> :: toMatrix(T * m) const {
	Vec<3,T> ux,uy,uz;
	toCoordinateFrame(ux,uy,uz);

	m[ 0] = ux[0];	m[ 4] = uy[0];	m[ 8] = uz[0];	m[12] = 0;
	m[ 1] = ux[1];	m[ 5] = uy[1];	m[ 9] = uz[1];	m[13] = 0;
	m[ 2] = ux[2];	m[ 6] = uy[2];	m[10] = uz[2];	m[14] = 0;
	m[ 3] = 0;		m[ 7] = 0;		m[11] = 0;		m[15] = 1;
}

// Note: same as toMatrix, but with matrix indices transposed
template<typename T>
inline void Quat<T> :: toMatrixTransposed(T * m) const {
	Vec<3,T> ux,uy,uz;
	toCoordinateFrame(ux,uy,uz);

	m[ 0] = ux[0];	m[ 1] = uy[0];	m[ 2] = uz[0];	m[ 3] = 0;
	m[ 4] = ux[1];	m[ 5] = uy[1];	m[ 6] = uz[1];	m[ 7] = 0;
	m[ 8] = ux[2];	m[ 9] = uy[2];	m[10] = uz[2];	m[11] = 0;
	m[12] = 0;		m[13] = 0;		m[14] = 0;		m[15] = 1;
}

template<typename T>
inline void Quat<T> :: toVectorX(T& ax, T& ay, T& az) const {
	ax = 1.0 - 2.0*y*y - 2.0*z*z;
	ay = 2.0*x*y + 2.0*z*w;
	az = 2.0*x*z - 2.0*y*w;
}

template<typename T>
inline void Quat<T> :: toVectorY(T& ax, T& ay, T& az) const {
	ax = 2.0*x*y - 2.0*z*w;
	ay = 1.0 - 2.0*x*x - 2.0*z*z;
	az = 2.0*y*z + 2.0*x*w;
}

template<typename T>
inline void Quat<T> :: toVectorZ(T& ax, T& ay, T& az) const {
	ax = 2.0*x*z + 2.0*y*w;
	ay = 2.0*y*z - 2.0*x*w;
	az = 1.0 - 2.0*x*x - 2.0*y*y;
}

/*
	Rotating a vector is simple:	
	v1 = q * qv * q^-1
	Where v is a 'pure quaternion' derived from the vector, i.e. w = 0. 	
*/
template<typename T>
inline Vec<3,T> Quat<T> :: rotate(const Vec<3,T>& v) const {
	// dst = ((q * quat(v)) * q^-1)
	// faster & simpler:
	// we know quat(v).w == 0
	Quat p(
		-x*v.x - y*v.y - z*v.z,
		 w*v.x + y*v.z - z*v.y,
		 w*v.y - x*v.z + z*v.x,
		 w*v.z + x*v.y - y*v.x
	);
	// faster & simpler:
	// we don't care about the w component
	// and we know that conj() is simply (w, -x, -y, -z):
	return Vec<3,T>(
		p.x*w - p.w*x + p.z*y - p.y*z,
		p.y*w - p.w*y + p.x*z - p.z*x,
		p.z*w - p.w*z + p.y*x - p.x*y
	);
//	p *= conj();	// p * q^-1
//	return Vec<3,T>(p.x, p.y, p.z);
}

template<typename T>
inline Vec<3,T> Quat<T> :: rotateTransposed(const Vec<3,T>& v) const {
	return Quat(*this).conj().rotate(v);
}

template<typename T>
Quat<T> Quat<T> :: slerp(const Quat& input, const Quat& target, T amt){
	Quat<T> result;
	
	if (amt==T(0)) {
		return input;
	} else if (amt==T(1)) {
		return target;
	}
	
	int bflip = 0;
	T dot_prod = input.dot(target);
	T a, b;

	//clamp
	dot_prod = (dot_prod < -1) ? -1 : ((dot_prod > 1) ? 1 : dot_prod);

	// if B is on opposite hemisphere from A, use -B instead
	if (dot_prod < 0.0) {
		dot_prod = -dot_prod;
		bflip = 1;
	}

	T cos_angle = acos(dot_prod);
	if(Quat::abs(cos_angle) > eps()) {
		T sine = sin(cos_angle);
		T inv_sine = 1./sine;

		a = sin(cos_angle*(1.-amt)) * inv_sine;
		b = sin(cos_angle*amt) * inv_sine;

		if (bflip) { b = -b; }
	} else {
		// nearly the same;
		// approximate without trigonometry
		a = amt;
		b = 1.-amt;
	}

	result.w = a*input.w + b*target.w;
	result.x = a*input.x + b*target.x;
	result.y = a*input.y + b*target.y;
	result.z = a*input.z + b*target.z;

	result.normalize();
	return result;
}

template<typename T>
void Quat<T> :: slerpBuffer(const Quat& input, const Quat& target, Quat<T> * buffer, int numFrames){


	/// Sinusoidal generator based on recursive formula x0 = c x1 - x2
	struct RSin {

		/// Constructor
		RSin(const T& frq=T(0), const T& phs=T(0), const T& amp=T(1))
		:	val2(0), mul(0){
			T f=frq, p=phs;
			mul  = (T)2 * (T)cos(f);
			val2 = (T)sin(p - f * T(2))*amp;
			val  = (T)sin(p - f       )*amp;
		}

		/// Generate next value.
		T operator()() const {
			T v0 = mul * val - val2;
			val2 = val;
			return val = v0;
		}

		mutable T val;
		mutable T val2;
		T mul;			///< Multiplication factor. [-2, 2] range gives stable sinusoids.
	};


	int bflip = 1;
	T dot_prod = input.dot(target);

	//clamp
	dot_prod = (dot_prod < -1) ? -1 : ((dot_prod > 1) ? 1 : dot_prod);

	// if B is on opposite hemisphere from A, use -B instead
	if (dot_prod < 0.0) {
		dot_prod = -dot_prod;
		bflip = -1;
	}

	const T cos_angle = acos(dot_prod);
	const T inv_frames = 1./((T)numFrames);

	if(Quat::abs(cos_angle) > eps())
	{
		const T sine = sin(cos_angle);
		const T inv_sine = 1./sine;
		RSin sinA(-cos_angle*inv_frames, cos_angle, inv_sine);
		RSin sinB(cos_angle*inv_frames, 0, inv_sine * bflip);

		for (int i=0; i<numFrames; i++) {
			//T amt = i*inv_frames;
			T a = sinA();
			T b = sinB();

			buffer[i].w = a*input.w + b*target.w;
			buffer[i].x = a*input.x + b*target.x;
			buffer[i].y = a*input.y + b*target.y;
			buffer[i].z = a*input.z + b*target.z;
			buffer[i].normalize();
		}
	} else {
		for (int i=0; i<numFrames; i++) {
			T a = i*inv_frames;
			T b = 1.-a;

			buffer[i].w = a*input.w + b*target.w;
			buffer[i].x = a*input.x + b*target.x;
			buffer[i].y = a*input.y + b*target.y;
			buffer[i].z = a*input.z + b*target.z;
			buffer[i].normalize();
		}
	}
}

template<typename T> 
Quat<T> Quat<T> :: getRotationTo(const Vec<3, T>& src, const Vec<3, T>& dst) {
	Quat<T> q;
	Vec<3, T> v0(src);
	Vec<3, T> v1(dst);
	
	T d = v0.dot(v1);
	if (d >= 1.) {
		// vectors are the same
		return q;
	}
	if (d < -0.999999999) {
		// vectors are nearly opposing
		// pick an axis to rotate around
		Vec<3, T> axis = cross(Vec<3, T>(0, 1, 0), src);
		// if colinear, pick another:
		if (axis.magSqr() < 0.00000000001) {
			axis = cross(Vec<3, T>(0, 0, 1), src);
		}
		//axis.normalize();
		q.fromAxisAngle(M_PI, axis);
	} else {
		T s = sqrt((d+1.)*2);
		T invs = 1./s;
		Vec<3, T> c = cross(v0, v1);
		q.x = c[0] * invs;
		q.y = c[1] * invs;
		q.z = c[2] * invs;
		q.w = s * 0.5;
	}
	return q.normalize();
}

/*!
	Get the quaternion from a given point and quaterion toward another point
*/
template<typename T>
void Quat<T> :: towardPoint(const Vec<3,T>& pos, const Quat<T>& q, const Vec<3,T>& v, float amt) {
	Vec<3,T> diff, axis;
	diff = v-pos;
	diff.normalize();
	
	if(amt < 0) {
		diff = diff*-1.;
	}

	Vec<3,T> zaxis;
	q.toVectorZ(zaxis);
	//axis = zaxis.cross(diff);
	cross(axis, zaxis, diff);
	//Vec<3,T>::cross(axis, zaxis, diff);
	axis.normalize();

	float axis_mag_sqr = axis.dot(axis);
	float along = zaxis.dot(diff); //Vec<3,T>::dot(zaxis, diff);

	if(axis_mag_sqr < 0.001 && along < 0) {
		//Vec<3,T>::cross(axis, zaxis, Vec<3,T>(0., 0., 1.));
		cross(axis, zaxis, Vec<3,T>(0, 0, 1));
		axis.normalize();

		if(axis_mag_sqr < 0.001) {
			//Vec<3,T>::cross(axis, zaxis, Vec<3,T>(0., 1., 0.));
			cross(axis, zaxis, Vec<3,T>(0, 1, 0));
			axis.normalize();
		}

		axis_mag_sqr = axis.dot(axis); //Vec<3,T>::dot(axis, axis);
	}

	if(along < 0.9995 && axis_mag_sqr > 0.001) {
		float theta = Quat::abs(amt)*acos(along);
//			printf("theta: %f  amt: %f\n", theta, amt);
		fromAxisAngle(theta, axis[0], axis[1], axis[2]);
	}
	else {
		setIdentity();
	}
}

// v1 and v2 must be normalized
// alternatively expressed as Q = (1+gp(v1, v2))/sqrt(2*(1+dot(b, a)))
template<typename T>
Quat<T> Quat<T> :: rotor(const Vec<3,T>& v1, const Vec<3,T>& v2) {
	// get the normal to the plane (i.e. the unit bivector containing the v1 and v2)
	Vec<3,T> n;
	cross(n, v1, v2);
	n.normalize();	// normalize because the cross product can get slightly denormalized

	// calculate half the angle between v1 and v2
	T dotmag = v1.dot(v2);
	T theta = acos(dotmag)*0.5;

	// calculate the scaled actual bivector generaed by v1 and v2
	Vec<3,T> bivec = n*sin(theta);
	Quat<T> q(cos(theta), bivec[0], bivec[1], bivec[2]);

	return q;
}

} // namespace

#endif /* include guard */
