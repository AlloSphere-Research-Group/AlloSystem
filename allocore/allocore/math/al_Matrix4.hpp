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

typedef Matrix4<double>	Matrix4d;	///< Double-precision 4-by-4 matrix
typedef Matrix4<float>	Matrix4f;	///< Single-precision 4-by-4 matrix


/// 4x4 Matrix (Homogenous Transform)
///
/// @ingroup allocore
template<typename T=double>
class Matrix4 : public Mat<4, T> {
public:
	typedef Mat<4, T> Base;

	/// Default constructor creates an identity matrix
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

	/// @param[in] src		C-array to copy values from
	Matrix4(const T * src)
	:	Base(src)
	{}

	/// @param[in] src		matrix to copy values from
	Matrix4(const Mat<4,T>& src)
	:	Base(src)
	{}


	/// Get a quaternion representation
	Quat<T> toQuat() const {
		return Quat<T>().fromMatrix(Base::elems());
	}

	/// Set from quaternion
	Matrix4& fromQuat(Quat<T>& q) { q.toMatrix(Base::elems()); return *this; }
	Matrix4& fromQuatTransposed(Quat<T>& q) { q.toMatrixTransposed(Base::elems()); return *this; }


	static Matrix4 rotate(float angle, float x, float y, float z) {
		return Matrix4::rotate(angle, Vec3d(x, y, z));
	}

	static Matrix4 rotate(float angle, const Vec<3, T> &v) {
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

	/// Get a shear transformation matrix on the xy plane
	static Matrix4 shearXY(T x, T y) {
		return Matrix4(	1,	0,	x,	0,
						0,	1,	y,	0,
						0,	0,	1,	0,
						0,	0,	0,	1	);
	}

	/// Get a shear transformation matrix on the yz plane
	static Matrix4 shearYZ(T y, T z) {
		return Matrix4(	1,	0,	0,	0,
						y,	1,	0,	0,
						z,	0,	1,	0,
						0,	0,	0,	1	);
	}

	/// Get a shear transformation matrix on the zx plane
	static Matrix4 shearZX(T z, T x) {
		return Matrix4(	1,	x,	0,	0,
						0,	1,	0,	0,
						0,	z,	1,	0,
						0,	0,	0,	1	);
	}


	/// Get a perspective projection matrix

	/// @param[in] l	distance from center of near plane to left edge
	/// @param[in] r	distance from center of near plane to right edge
	/// @param[in] b	distance from center of near plane to bottom edge
	/// @param[in] t	distance from center of near plane to top edge
	/// @param[in] n	distance from eye to near plane
	/// @param[in] f	distance from eye to far plane
	static Matrix4 perspective(T l, T r, T b, T t, T n, T f) {
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		T n2 = n*2;
		T fn2 = f*n2;
		return Matrix4(	n2/W,	0,		W2/W,		0,
						0,		n2/H,	H2/H,		0,
						0,		0,		-D2/D,		-fn2/D,
						0,		0,		-1,			0 );
	}

	/// Get a perspective projection matrix

	/// @param[in] fovy		field of view angle, in degrees, in the y direction
	/// @param[in] aspect	aspect ratio
	/// @param[in] near		distance from eye to near plane
	/// @param[in] far		distance from eye to far plane
	static Matrix4 perspective(T fovy, T aspect, T near, T far) {
		double f = 1./tan(fovy*M_DEG2RAD/2.);
		T D = far-near;	T D2 = far+near;
		T fn2 = far*near*2;
		return Matrix4(	f/aspect,	0,	0,			0,
						0,			f,	0,			0,
						0,			0,	-D2/D,		-fn2/D,
						0,			0,	-1,			0
		);
	}

	/// Calculate perspective projection from near plane and eye coordinates

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
	static Matrix4 perspective(
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
		vu = (nearTL-nearBL).normalize();	// up vector
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

	/// Get a left-eye perspective projection matrix (for stereographics)
	static Matrix4 perspectiveLeft(T fovy, T aspect, T near, T far, T eyeSep, T focal) {
		return perspectiveOffAxis(fovy, aspect, near, far,-0.5*eyeSep, focal);
	}

	/// Get a right-eye perspective projection matrix (for stereographics)
	static Matrix4 perspectiveRight(T fovy, T aspect, T near, T far, T eyeSep, T focal) {
		return perspectiveOffAxis(fovy, aspect, near, far, 0.5*eyeSep, focal);
	}

	/// Get an off-axis perspective projection matrix (for stereographics)
	static Matrix4 perspectiveOffAxis(T fovy, T aspect, T near, T far, T xShift, T focal) {
		T top = near * tan(fovy*M_DEG2RAD*0.5);	// height of view at distance = near
		T bottom = -top;
		T shift = -xShift*near/focal;
		T left = -aspect*top + shift;
		T right = aspect*top + shift;
		return perspective(left, right, bottom, top, near, far);
	}

	/// Get an off-axis perspective projection matrix (for stereographics)

	/// @param[in] fovy		field of view angle, in degrees, in the y direction
	/// @param[in] aspect	aspect ratio
	/// @param[in] near		near clipping plane coordinate
	/// @param[in] far		far clipping plane coordinate
	/// @param[in] xShift	amount to shift off x-axis
	/// @param[in] yShift	amount to shift off y-axis
	/// @param[in] focal	focal length
	static Matrix4 perspectiveOffAxis(T fovy, T aspect, T near, T far, T xShift, T yShift, T focal) {
		double tanfovy = tan(fovy*M_DEG2RAD/2.);
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

	static Matrix4 unPerspective(T l, T r, T b, T t, T n, T f) {
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		T n2 = n*2;
		T fn2 = f*n2;
		return Matrix4(	W/n2,	0,		0,		W2/n2,
						0,		H/n2,	0,		H2/n2,
						0,		0,		0,		-1,
						0,		0,		-D/fn2,	D2/fn2	);
	}

	/// Get an orthographic projection matrix

	/// @param[in] l	coordinate of left clipping plane
	/// @param[in] r	coordinate of right clipping plane
	/// @param[in] b	coordinate of bottom clipping plane
	/// @param[in] t	coordinate of top clipping plane
	/// @param[in] n	coordinate of near clipping plane
	/// @param[in] f	coordinate of far clipping plane
	static Matrix4 ortho(T l, T r, T b, T t, T n, T f) {
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		return Matrix4(	2/W,	0,		0,		-W2/W,
						0,		2/H,	0,		-H2/H,
						0,		0,		-2/D,	-D2/D,
						0,		0,		0,		1	);
	}

	static Matrix4 unOrtho(T l, T r, T b, T t, T n, T f) {
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		return Matrix4(	W/2,	0,		0,		W2/2,
						0,		H/2,	0,		H2/2,
						0,		0,		D/-2,	D2/2,
						0,		0,		0,		1	);
	}

	/// Get a two-dimensional orthographic projection matrix

	/// This is identical to ortho with -1, 1 for the near, far distances.
	/// @param[in] l	coordinate of left clipping plane
	/// @param[in] r	coordinate of right clipping plane
	/// @param[in] b	coordinate of bottom clipping plane
	/// @param[in] t	coordinate of top clipping plane
	static Matrix4 ortho2D(T l, T r, T b, T t) {
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		return Matrix4(	2/W,	0,		0,		-W2/W,
						0,		2/H,	0,		-H2/H,
						0,		0,		-1,		0,
						0,		0,		0,		1	);
	}

	/// Get a viewing matrix based on an eye reference frame

	/// @param[in] ur		eye right unit direction vector
	/// @param[in] uu		eye up unit direction vector
	/// @param[in] uf		eye forward unit direction vector
	/// @param[in] eyePos	eye position
	static Matrix4 lookAt(const Vec<3,T>& ur, const Vec<3,T>& uu, const Vec<3,T>& uf, const Vec<3,T>& eyePos) {
		return Matrix4(
			 ur[0], ur[1], ur[2], -(ur.dot(eyePos)),
			 uu[0], uu[1], uu[2], -(uu.dot(eyePos)),
			 uf[0], uf[1], uf[2], -(uf.dot(eyePos)),
			0, 0, 0, 1
		);
	}

	/// Get a viewing matrix based on look-at parameters

	/// @param[in] eyePos	eye position
	/// @param[in] at		point being looked at
	/// @param[in] up		up vector
	static Matrix4 lookAt(const Vec<3,T>& eyePos, const Vec<3,T>& at, const Vec<3,T>& up) {
		Vec<3,T> z = (at - eyePos).normalize();
		Vec<3,T> y = up; y.normalize();
		Vec<3,T> x = cross(z, up).normalize();
		return lookAt(x, y, -z, eyePos);
	}

//	static Matrix4 lookAtRH(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos) {
//		return Matrix4(
//					   ux[0], ux[1], ux[2], -(ux.dot(pos)),
//					   uy[0], uy[1], uy[2], -(uy.dot(pos)),
//					   uz[0],uz[1],uz[2],  -(uz.dot(pos)),
//					   0, 0, 0, 1
//					   );
//	}
//
//	static Matrix4 lookAtRH(const Vec<3,T>& eye, const Vec<3,T>& at, const Vec<3,T>& up) {
//		Vec<3,T> z = (at - eye).normalize();
//		Vec<3,T> x = cross(up, z);
//		Vec<3,T> y = cross(z, x);
//		return lookAt(x, y, z, eye);
//	}

	/// Get a left-eye viewing matrix
	static Matrix4 lookAtLeft(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos, double eyeSep) {
		return lookAtOffAxis(ux,uy,uz, pos,-0.5*eyeSep);
	}

	/// Get a right-eye viewing matrix
	static Matrix4 lookAtRight(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos, double eyeSep) {
		return lookAtOffAxis(ux,uy,uz, pos, 0.5*eyeSep);
	}

	/// Get an off-axis viewing matrix
	static Matrix4 lookAtOffAxis(const Vec<3,T>& ux, const Vec<3,T>& uy, const Vec<3,T>& uz, const Vec<3,T>& pos, double eyeShift){
		return lookAt(ux, uy, uz, pos + (ux * -eyeShift));
	}

	/// Computes product of matrix multiplied by column vector, r = m * vCol

	/// This is typically what is required to project a vertex through a transform.
	/// For a better explanation, @see http://xkcd.com/184/ -g
	Vec<4,T> transform(const Vec<4,T>& vCol) const {
		return *this * vCol;
	}

	/// Computes product of matrix multiplied by column vector, r = m * vCol
	Vec<4,T> transform(const Vec<3,T>& vCol) const {
		return transform(Vec<4,T>(vCol, T(1)));
	}


	/// Get the inverse of a matrix
	static Matrix4 inverse(const Mat<4,T>& m) {
		Matrix4 res(m);
		invert(res);
		return res;
	}


	// \deprecated Use Mat::translation
	static Matrix4 translate(T x, T y, T z) {return translation(Vec<3,T>(x,y,z));}
	// \deprecated Use Mat::translation
	template<typename V>
	static Matrix4 translate(const Vec<3,V>& v){return translation(v);}
	// \deprecated Use Mat::scaling
	static Matrix4 scale(T x, T y, T z) { return scaling(x,y,z); }
	// \deprecated Use Mat::scaling
	template<typename V>
	static Matrix4 scale(const Vec<3,V>& v){return scaling(v);}
	// \deprecated Use Mat::scaling
	template<typename V>
	static Matrix4 scale(const V& v) { return scaling(v); }
	// \deprecated Use Mat::rotation
	static Matrix4 rotateXY(T theta) { return rotation(theta,0,1); }
	static Matrix4 rotateYZ(T theta) { return rotation(theta,1,2); }
	static Matrix4 rotateZX(T theta) { return rotation(theta,2,0); }
};



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
