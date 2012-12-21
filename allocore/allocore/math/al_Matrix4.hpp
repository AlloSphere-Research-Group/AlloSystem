#ifndef INCLUDE_AL_MATRIX4_HPP
#define INCLUDE_AL_MATRIX4_HPP

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
	This is a more specialized 4x4 matrix including transformations commonly 
	used in computer graphics.

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Quat.hpp"
#include "allocore/math/al_Mat.hpp"
#include "allocore/math/al_Vec.hpp"

namespace al {

template<class T> class Matrix4;


/// 4x4 Matrix (Homogenous Transform)
template<typename T=double>
class Matrix4 : public Mat<4, T> {	
public:
	typedef Mat<4, T> Base;	

	Matrix4()
	: Base(
		1, 0, 0, 0, 
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	)
	{}

	/*
		The constructor will map into memory locations as follows:

		Matrix4(arg1, arg2, arg3, ...)
			
		arg1 ->m[0]	arg2 ->m[4]	arg3 ->m[8]		arg4 ->m[12]
		arg5 ->m[1]	arg6 ->m[5]	arg7 ->m[9]		arg8 ->m[13]
		arg9 ->m[2]	arg10->m[6]	arg11->m[10]	arg12->m[14]
		arg13->m[3]	arg14->m[7]	arg15->m[11]	arg16->m[15]	
		
		Matrix4(r1c1, r1c2, r1c3, r1c4, 
				r2c1, r2c2, r2c3, r2c4, 
				r3c1, r3c2, r3c3, r3c4, 
				r4c1, r4c2, r4c3, r4c4)
	*/
	Matrix4(
		const T& r1c1, const T& r1c2, const T& r1c3, const T& r1c4,
		const T& r2c1, const T& r2c2, const T& r2c3, const T& r2c4,
		const T& r3c1, const T& r3c2, const T& r3c3, const T& r3c4,
		const T& r4c1, const T& r4c2, const T& r4c3, const T& r4c4
	)
	:	Base(
			r1c1, r1c2, r1c3, r1c4,
			r2c1, r2c2, r2c3, r2c4,
			r3c1, r3c2, r3c3, r3c4,
			r4c1, r4c2, r4c3, r4c4
		)
	{}
	
	Matrix4(
		const Vec<3,T>& xaxis,
		const Vec<3,T>& yaxis,
		const Vec<3,T>& zaxis,
		const Vec<3,T>& position
	)
	:	Base(
			xaxis[0], yaxis[0], zaxis[0], position[0],
			xaxis[1], yaxis[1], zaxis[1], position[1],
			xaxis[2], yaxis[2], zaxis[2], position[2],
			0, 0, 0, 1
		)
	{}
	
	Matrix4(const T * src)
	:	Base(src)
	{}
	
	Matrix4(const Base & src)
	:	Base(src)
	{}
	
	Matrix4& set(const Base & src) { Base::set(src.elems()); return *this; }
	
	Quat<T>& toQuat() {
		Quat<T> q;
		q.fromMatrix(Base::elems);
		return q;
	}
	Matrix4& fromQuat(Quat<T>& q) { q.toMatrix(Base::elems()); return *this; }
	Matrix4& fromQuatTransposed(Quat<T>& q) { q.toMatrixTransposed(Base::elems()); return *this; }
		
	static const Matrix4 identity() {
		return Matrix4(
			1,	0,	0,	0, 
			0,	1,	0,	0, 
			0,	0,	1,	0, 
			0,	0,	0,	1
		);
	}
	
	static const Matrix4 translate(T x, T y, T z) {
		return Matrix4(
			1, 0, 0, x,
			0, 1, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1
		);
	}	
	template<typename T1>
	static const Matrix4 translate(const Vec<3, T1>& v) { return translate(v.x, v.y, v.z); }
	
	static const Matrix4 scale(T x, T y, T z) {
		return Matrix4(
			x,	0,	0,	0,
			0,	y,	0,	0, 
			0,	0,	z,	0, 
			0,	0,	0,	1
		);
	}
	template<typename T1>
	static const Matrix4 scale(const Vec<3, T1>& v) { return scale(v.x, v.y, v.z); }
	template<typename T1>
	static const Matrix4 scale(const T1 v) { return scale(v, v, v); }
	
	static const Matrix4 rotateYZ(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		return Matrix4(	1, 0, 0, 0, 
						0, C,-S, 0, 
						0, S, C, 0, 
						0, 0, 0, 1);
	}
	static const Matrix4 rotateZX(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		return Matrix4(	C, 0, S, 0, 
						0, 1, 0, 0, 
						-S,0, C, 0, 
						0, 0, 0, 1);
	}
	static const Matrix4 rotateXY(T theta) {
		const T C = cos(theta); 
		const T S = sin(theta);
		return Matrix4(	C,-S, 0, 0, 
						S, C, 0, 0, 
						0, 0, 1, 0, 
						0, 0, 0, 1);
	}

	static const Matrix4 rotate(float angle, float x, float y, float z) {
		return Matrix4::rotate(angle, Vec3d(x, y, z));
	}
	
	static const Matrix4 rotate(float angle, const Vec<3, T> &v) {
		Vec<3, T> axis(v);
		axis.normalize();
		
		float c = cos(angle);
		float s = sin(angle);
			
		Matrix4 m(
			axis[0]*axis[0]*(1-c)+c,
			axis[1]*axis[0]*(1-c)+axis[2]*s,
			axis[0]*axis[2]*(1-c)-axis[1]*s,
			0,
			
			axis[0]*axis[1]*(1-c)-axis[2]*s,
			axis[1]*axis[1]*(1-c)+c,
			axis[1]*axis[2]*(1-c)+axis[0]*s,
			0,

			axis[0]*axis[2]*(1-c)+axis[1]*s,
			axis[1]*axis[2]*(1-c)-axis[0]*s,
			axis[2]*axis[2]*(1-c)+c,
			0,
			
			0, 0, 0, 1
		);
		
		return m;
	}
	
	static const Matrix4 shearYZ(T y, T z) {
		return Matrix4(	1,	0,	0,	0,
						y,	1,	0,	0,
						z,	0,	1,	0,
						0,	0,	0,	1	);
	}
	static const Matrix4 shearZX(T z, T x) {
		return Matrix4(	1,	x,	0,	0,
						0,	1,	0,	0,
						0,	z,	1,	0,
						0,	0,	0,	1	);
	}
	static const Matrix4 shearXY(T x, T y) {
		return Matrix4(	1,	0,	x,	0,
						0,	1,	y,	0,
						0,	0,	1,	0,
						0,	0,	0,	1	);
	}


	/// @param[in] l	distance from center of near plane to left edge
	/// @param[in] r	distance from center of near plane to right edge
	/// @param[in] b	distance from center of near plane to bottom edge
	/// @param[in] t	distance from center of near plane to top edge
	/// @param[in] n	distance from eye to near plane
	/// @param[in] f	distance from eye to far plane
	static const Matrix4 perspective(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		const T n2 = n*2;
		const T fn2 = f*n2;
		return Matrix4(	n2/W,	0,		W2/W,		0, 
						0,		n2/H,	H2/H,		0, 
						0,		0,		-D2/D,		-fn2/D,
						0,		0,		-1,			0 );
	}
	
	static const Matrix4 perspective(T fovy, T aspect, T near, T far) {
		float f = 1/tan(fovy*M_DEG2RAD/2.);
		const T D = far-near;	const T D2 = far+near;
		const T fn2 = far*near*2;
		return Matrix4(	f/aspect,	0,	0,			0,
						0,			f,	0,			0,
						0,			0,	-D2/D,		-fn2/D,
						0,			0,	-1,			0
		);
	}
	
	/// Calculate perspective projection for near plane and eye coordinates
	
	/// (nearBL, nearBR, nearTL, eye) all share the same coordinate system
	/// (nearBR,nearBL) and (nearTL,nearBL) should form a right angle
	/// (eye) can be set freely, allowing diverse off-axis projections
	/// @see Generalized Perspective Projection, Robert Kooima, 2009, EVL
	/// @param[in] nearBL	bottom-left near-plane coordinate (world-space)
	/// @param[in] nearBR	bottom-right near-plane coordinate (world-space)
	/// @param[in] nearTL	top-left near-plane coordinate (world-space)
	/// @param[in] eye		eye coordinate (world-space)
	/// @param[in] near		near plane distance from eye
	/// @param[in] far		far plane distance from eye
	static const Matrix4 perspective(
								const Vec<3,T>& nearBL,	
								const Vec<3,T>& nearBR,	
								const Vec<3,T>& nearTL,	
								const Vec<3,T>& eye,	
								T near,	T far)			
	{
		Vec<3,T> va, vb, vc;
		Vec<3,T> vr, vu, vn;
		T l, r, b, t, d;

		// compute orthonormal basis for the screen
		vr = (nearBR-nearBL).normalize();	// right vector
		vu = (nearTL-nearBL).normalize();	// upvector
		cross(vn, vr, vu);	// normal(forward) vector (out from screen)
		vn.normalize();

		// compute vectors from eye to screen corners:
		va = nearBL-eye;	
		vb = nearBR-eye;	
		vc = nearTL-eye;	

		// distance from eye to screen-plane
		// = component of va along vector vn (normal to screen)
		d = -va.dot(vn);

		// find extent of perpendicular projection
		T nbyd = near/d;
		l = vr.dot(va) * nbyd;
		r = vr.dot(vb) * nbyd;
		b = vu.dot(va) * nbyd;	// not vd?
		t = vu.dot(vc) * nbyd;
		
		return perspective(l, r, b, t, near, far);
	}
	
	// for stereographics:
	static const Matrix4 perspectiveLeft(T fovy, T aspect, T near, T far, T eyeSep, T focal) {
		return perspectiveOffAxis(fovy, aspect, near, far,-0.5*eyeSep, focal);
	}
	static const Matrix4 perspectiveRight(T fovy, T aspect, T near, T far, T eyeSep, T focal) {
		return perspectiveOffAxis(fovy, aspect, near, far, 0.5*eyeSep, focal);
	}
	static const Matrix4 perspectiveOffAxis(T fovy, T aspect, T near, T far, T xShift, T focal) {
		T top = near * tan(fovy*M_DEG2RAD*0.5);	// height of view at distance = near
		T bottom = -top;
		T shift = -xShift*near/focal;
		T left = -aspect*top + shift;
		T right = aspect*top + shift;
		return perspective(left, right, bottom, top, near, far);
	}
	static const Matrix4 perspectiveOffAxis(T fovy, T aspect, T near, T far, T xShift, T yShift, T focal) {
		float tanfovy = tan(fovy*M_DEG2RAD/2.);
		T t = near * tanfovy;	// height of view at distance = near
		T b = -t;
		T l = -aspect*t;
		T r = aspect*t;
		
		T shift = -xShift*near/focal;
		l += shift;
		r += shift;
		shift = -yShift*near/focal;
		t += shift;
		b += shift;
		
		return perspective(l, r, b, t, near, far);
	}
	
	static const Matrix4 unPerspective(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		const T n2 = n*2;
		const T fn2 = f*n2;
		return Matrix4(	W/n2,	0,		0,		W2/n2,
						0,		H/n2,	0,		H2/n2,
						0,		0,		0,		-1,
						0,		0,		-D/fn2,	D2/fn2	);
	}
	
	static const Matrix4 ortho(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		return Matrix4(	2/W,	0,		0,		-W2/W,
						0,		2/H,	0,		-H2/H,
						0,		0,		-2/D,	-D2/D,
						0,		0,		0,		1	);
	}
	
	static const Matrix4 unOrtho(T l, T r, T b, T t, T n, T f) {
		const T W = r-l;	const T W2 = r+l;
		const T H = t-b;	const T H2 = t+b;
		const T D = f-n;	const T D2 = f+n;
		return Matrix4(	W/2,	0,		0,		W2/2,
						0,		H/2,	0,		H2/2,
						0,		0,		D/-2,	D2/2,
						0,		0,		0,		1	);
	}


	/*
	How does lookAt work?
	http://pyopengl.sourceforge.net/documentation/manual/gluLookAt.3G.html
	http://www.opengl.org/wiki/GluLookAt_code
	*/
	
	/// @param[in] ux	
	///
	///
	static const Matrix4 lookAt(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& eyePos) {
		return Matrix4(
			 ux[0], ux[1], ux[2], -(ux.dot(eyePos)),
			 uy[0], uy[1], uy[2], -(uy.dot(eyePos)),
			 uz[0], uz[1], uz[2], -(uz.dot(eyePos)),
			0, 0, 0, 1
		);
	}
	
	static const Matrix4 lookAt(const Vec<3,T>& eyePos, const Vec<3,T>& at, const Vec<3,T>& up) {
		Vec<3,T> z = (at - eyePos).normalize();	
		Vec<3,T> y = up; y.normalize();
		Vec<3,T> x = cross(z, up).normalize();
		return lookAt(x, y, -z, eyePos);
	}

//	static const Matrix4 lookAtRH(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos) {
//		return Matrix4(
//					   ux[0], ux[1], ux[2], -(ux.dot(pos)),
//					   uy[0], uy[1], uy[2], -(uy.dot(pos)),
//					   uz[0],uz[1],uz[2],  -(uz.dot(pos)),
//					   0, 0, 0, 1
//					   );
//	}
//	
//	static const Matrix4 lookAtRH(const Vec<3,T>& eye, const Vec<3,T>& at, const Vec<3,T>& up) {
//		Vec<3,T> z = (at - eye).normalize();
//		Vec<3,T> x = cross(up, z);
//		Vec<3,T> y = cross(z, x);
//		return lookAt(x, y, z, eye);
//	}	
	
	// for stereographics:
	static const Matrix4 lookAtLeft(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos, double eyeSep) {
		return lookAtOffAxis(ux,uy,uz, pos,-0.5*eyeSep);
	}
	static const Matrix4 lookAtRight(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos, double eyeSep) {
		return lookAtOffAxis(ux,uy,uz, pos, 0.5*eyeSep);
	}
	static const Matrix4 lookAtOffAxis(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos, double eyeShift){
		return lookAt(ux, uy, uz, pos + (ux * -eyeShift));
	}
	
	/// Computes product of matrix multiplied by column vector, r = m * vCol
	// This is typically what is required to project a vertex through a transform
	// For a better explanation, @see http://xkcd.com/184/ -g
	Vec<4,T> transform(const Vec<3,T>& vCol) const {
		Vec<4,T> r, v(vCol[0], vCol[1], vCol[2], 1.);
		Mat<4,T>::multiply(r, *this, v);
		return r;
	}
	Vec<4,T> transform(const Vec<4,T>& v) const {
		Vec<4,T> r;
		Mat<4,T>::multiply(r, *this, v);
		return r;
	}
	
	void print(FILE * out) {
		Mat<4,T>& m = *this;
		fprintf(out, "{ %f, %f, %f, %f,\n",	m[0], m[4], m[8], m[12]);
		fprintf(out, "%f, %f, %f, %f,\n",	m[1], m[5], m[9], m[13]);
		fprintf(out, "%f, %f, %f, %f,\n",	m[2], m[6], m[10], m[14]);
		fprintf(out, "%f, %f, %f, %f }",	m[3], m[7], m[11], m[15]);
	}
	
	static const Matrix4 inverse(const Base& m) {
		double determinant =
			m[12]*m[9]*m[6]*m[3] - m[8]*m[13]*m[6]*m[3] - m[12]*m[5]*m[10]*m[3] + m[4]*m[13]*m[10]*m[3]+
			m[8]*m[5]*m[14]*m[3] - m[4]*m[9]*m[14]*m[3] - m[12]*m[9]*m[2]*m[7] + m[8]*m[13]*m[2]*m[7]+
			m[12]*m[1]*m[10]*m[7] - m[0]*m[13]*m[10]*m[7] - m[8]*m[1]*m[14]*m[7] + m[0]*m[9]*m[14]*m[7]+
			m[12]*m[5]*m[2]*m[11] - m[4]*m[13]*m[2]*m[11] - m[12]*m[1]*m[6]*m[11] + m[0]*m[13]*m[6]*m[11]+
			m[4]*m[1]*m[14]*m[11] - m[0]*m[5]*m[14]*m[11] - m[8]*m[5]*m[2]*m[15] + m[4]*m[9]*m[2]*m[15]+
			m[8]*m[1]*m[6]*m[15] - m[0]*m[9]*m[6]*m[15] - m[4]*m[1]*m[10]*m[15] + m[0]*m[5]*m[10]*m[15];
	
		return Matrix4(
			m[9]*m[14]*m[7] - m[13]*m[10]*m[7] + m[13]*m[6]*m[11] - m[5]*m[14]*m[11] - m[9]*m[6]*m[15] + m[5]*m[10]*m[15],
			m[12]*m[10]*m[7] - m[8]*m[14]*m[7] - m[12]*m[6]*m[11] + m[4]*m[14]*m[11] + m[8]*m[6]*m[15] - m[4]*m[10]*m[15],
			m[8]*m[13]*m[7] - m[12]*m[9]*m[7] + m[12]*m[5]*m[11] - m[4]*m[13]*m[11] - m[8]*m[5]*m[15] + m[4]*m[9]*m[15],
			m[12]*m[9]*m[6] - m[8]*m[13]*m[6] - m[12]*m[5]*m[10] + m[4]*m[13]*m[10] + m[8]*m[5]*m[14] - m[4]*m[9]*m[14],
			m[13]*m[10]*m[3] - m[9]*m[14]*m[3] - m[13]*m[2]*m[11] + m[1]*m[14]*m[11] + m[9]*m[2]*m[15] - m[1]*m[10]*m[15],
			m[8]*m[14]*m[3] - m[12]*m[10]*m[3] + m[12]*m[2]*m[11] - m[0]*m[14]*m[11] - m[8]*m[2]*m[15] + m[0]*m[10]*m[15],
			m[12]*m[9]*m[3] - m[8]*m[13]*m[3] - m[12]*m[1]*m[11] + m[0]*m[13]*m[11] + m[8]*m[1]*m[15] - m[0]*m[9]*m[15],
			m[8]*m[13]*m[2] - m[12]*m[9]*m[2] + m[12]*m[1]*m[10] - m[0]*m[13]*m[10] - m[8]*m[1]*m[14] + m[0]*m[9]*m[14],
			m[5]*m[14]*m[3] - m[13]*m[6]*m[3] + m[13]*m[2]*m[7] - m[1]*m[14]*m[7] - m[5]*m[2]*m[15] + m[1]*m[6]*m[15],
			m[12]*m[6]*m[3] - m[4]*m[14]*m[3] - m[12]*m[2]*m[7] + m[0]*m[14]*m[7] + m[4]*m[2]*m[15] - m[0]*m[6]*m[15],
			m[4]*m[13]*m[3] - m[12]*m[5]*m[3] + m[12]*m[1]*m[7] - m[0]*m[13]*m[7] - m[4]*m[1]*m[15] + m[0]*m[5]*m[15],
			m[12]*m[5]*m[2] - m[4]*m[13]*m[2] - m[12]*m[1]*m[6] + m[0]*m[13]*m[6] + m[4]*m[1]*m[14] - m[0]*m[5]*m[14],
			m[9]*m[6]*m[3] - m[5]*m[10]*m[3] - m[9]*m[2]*m[7] + m[1]*m[10]*m[7] + m[5]*m[2]*m[11] - m[1]*m[6]*m[11],
			m[4]*m[10]*m[3] - m[8]*m[6]*m[3] + m[8]*m[2]*m[7] - m[0]*m[10]*m[7] - m[4]*m[2]*m[11] + m[0]*m[6]*m[11],
			m[8]*m[5]*m[3] - m[4]*m[9]*m[3] - m[8]*m[1]*m[7] + m[0]*m[9]*m[7] + m[4]*m[1]*m[11] - m[0]*m[5]*m[11],
			m[4]*m[9]*m[2] - m[8]*m[5]*m[2] + m[8]*m[1]*m[6] - m[0]*m[9]*m[6] - m[4]*m[1]*m[10] + m[0]*m[5]*m[10]
		) * (1./determinant);
	}
};

typedef Matrix4<double>	Matrix4d;	///< Double-precision 4-by-4 matrix
typedef Matrix4<float>	Matrix4f;	///< Single-precision 4-by-4 matrix



/// Get frustum far plane distance from a projection matrix
template <class T>
inline T frustumFar(const Mat<4,T>& proj){
	return proj[14] / (proj[10] + T(1)); }

/// Get frustum near plane distance from a projection matrix
template <class T>
inline T frustumNear(const Mat<4,T>& proj){
	return proj[14] / (proj[10] - T(1)); }

/// Get frustum depth from a projection matrix
template <class T>
inline T frustumDepth(const Mat<4,T>& proj){
	return (T(-2)*proj[14]) / (proj[10]*proj[10] - T(1)); }

/// Get frustum aspect ratio from a projection matrix
template <class T>
inline T frustumAspect(const Mat<4,T>& proj){
	return proj[5] / proj[0]; }

} // al::

#endif /* include guard */
