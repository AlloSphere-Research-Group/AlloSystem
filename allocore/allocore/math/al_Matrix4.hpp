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

#include <cmath> // tan
#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Mat.hpp"
#include "allocore/math/al_Vec.hpp"

namespace al {

template<class T> class Matrix4;

typedef Matrix4<double>	Matrix4d;	///< Double-precision 4-by-4 matrix
typedef Matrix4<float>	Matrix4f;	///< Single-precision 4-by-4 matrix


/// 4x4 Matrix (Homogenous Transform)

/// All functions operate according to a right-handed coordinate system.
/// Elements are stored in column-major format.
/// Mathematical reference: http://www.glprogramming.com/red/appendixf.html
/// @ingroup allocore
template<typename T=double>
class Matrix4 : public Mat<4, T> {
public:
	typedef Mat<4, T> Base;

	using Base::Base; // inherit constructors
	using Base::operator=;

	/// Default constructor creates an identity matrix
	Matrix4(const T& diag = T(1))
	:	Base(diag)
	{}

	template <int M, class U>
	Matrix4(const Mat<M,U>& src)
	:	Base(src)
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


	/// Get a perspective projection matrix

	/// @param[in] l	distance from center of near plane to left edge
	/// @param[in] r	distance from center of near plane to right edge
	/// @param[in] b	distance from center of near plane to bottom edge
	/// @param[in] t	distance from center of near plane to top edge
	/// @param[in] n	distance from eye to near plane
	/// @param[in] f	distance from eye to far plane
	static Matrix4 perspective(T l, T r, T b, T t, T n, T f){
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		T n2 = n*2;
		T fn2 = f*n2;
		return {
			n2/W,	0,		W2/W,		0,
			0,		n2/H,	H2/H,		0,
			0,		0,		-D2/D,		-fn2/D,
			0,		0,		-1,			0
		};
	}

	/// Get a perspective projection matrix

	/// @param[in] fovy		field of view angle, in degrees, in the y direction
	/// @param[in] aspect	aspect ratio
	/// @param[in] near		distance from eye to near plane
	/// @param[in] far		distance from eye to far plane
	static Matrix4 perspective(T fovy, T aspect, T near, T far){
		T f = std::tan((90.-fovy*0.5)*M_DEG2RAD); // tan(pi/2-x) = 1/tan(x)
		T D = far-near;	T D2 = far+near;
		T fn2 = far*near*2;
		return {
			f/aspect,	0,	0,			0,
			0,			f,	0,			0,
			0,			0,	-D2/D,		-fn2/D,
			0,			0,	-1,			0
		};
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
		T near,	T far
	){
		// compute orthonormal basis for the screen
		auto vr = (nearBR - nearBL).normalize();	// right vector
		auto vu = (nearTL - nearBL).normalize();	// up vector
		auto vn = cross(vr, vu);					// forward vector (out from screen)

		// compute vectors from eye to screen corners:
		auto va = nearBL - eye;
		auto vb = nearBR - eye;
		auto vc = nearTL - eye;

		// distance from eye to screen-plane
		// = component of va along vector vn (normal to screen)
		auto d = -va.dot(vn);

		// find extent of perpendicular projection
		T nbyd = near/d;
		auto l = vr.dot(va) * nbyd;
		auto r = vr.dot(vb) * nbyd;
		auto b = vu.dot(va) * nbyd;	// not vd?
		auto t = vu.dot(vc) * nbyd;

		return perspective(l, r, b, t, near, far);
	}

	/// Get an off-axis perspective projection matrix (for stereoscopy)
	static Matrix4 perspectiveOffAxis(T fovy, T aspect, T near, T far, T xShift, T focal){
		return perspectiveOffAxis(fovy, aspect, near,far, xShift,T(0), focal);
	}

	/// Get an off-axis perspective projection matrix (for stereoscopy)

	/// @param[in] fovy		field of view angle, in degrees, in the y direction
	/// @param[in] aspect	aspect ratio
	/// @param[in] near		near clipping plane coordinate
	/// @param[in] far		far clipping plane coordinate
	/// @param[in] xShift	amount to shift off x-axis
	/// @param[in] yShift	amount to shift off y-axis
	/// @param[in] focal	focal length
	static Matrix4 perspectiveOffAxis(T fovy, T aspect, T near, T far, T xShift, T yShift, T focal){
		T tanfovy = std::tan(fovy*M_DEG2RAD*0.5);
		T t = near * tanfovy; // height of view at distance = near
		T b = -t;
		T l = -aspect*t;
		T r =  aspect*t;
		T dx = -xShift*near/focal;
		T dy = -yShift*near/focal;
		return perspective(l+dx, r+dx, b+dy, t+dy, near, far);
	}

	static Matrix4 unPerspective(T l, T r, T b, T t, T n, T f){
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		T n2 = n*2;
		T fn2 = f*n2;
		return {
			W/n2,	0,		0,		W2/n2,
			0,		H/n2,	0,		H2/n2,
			0,		0,		0,		-1,
			0,		0,		-D/fn2,	D2/fn2
		};
	}

	/// Get an orthographic projection matrix

	/// @param[in] l	coordinate of left clipping plane
	/// @param[in] r	coordinate of right clipping plane
	/// @param[in] b	coordinate of bottom clipping plane
	/// @param[in] t	coordinate of top clipping plane
	/// @param[in] n	coordinate of near clipping plane
	/// @param[in] f	coordinate of far clipping plane
	static Matrix4 ortho(T l, T r, T b, T t, T n, T f){
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		return {
			2/W,	0,		0,		-W2/W,
			0,		2/H,	0,		-H2/H,
			0,		0,		-2/D,	-D2/D,
			0,		0,		0,		1
		};
	}

	static Matrix4 unOrtho(T l, T r, T b, T t, T n, T f){
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		T D = f-n;	T D2 = f+n;
		return {
			W/2,	0,		0,		W2/2,
			0,		H/2,	0,		H2/2,
			0,		0,		D/-2,	D2/2,
			0,		0,		0,		1
		};
	}

	/// Get a two-dimensional orthographic projection matrix

	/// This is identical to ortho with -1, 1 for the near, far distances.
	/// @param[in] l	coordinate of left clipping plane
	/// @param[in] r	coordinate of right clipping plane
	/// @param[in] b	coordinate of bottom clipping plane
	/// @param[in] t	coordinate of top clipping plane
	static Matrix4 ortho2D(T l, T r, T b, T t){
		T W = r-l;	T W2 = r+l;
		T H = t-b;	T H2 = t+b;
		return {
			2/W,	0,		0,		-W2/W,
			0,		2/H,	0,		-H2/H,
			0,		0,		-1,		0,
			0,		0,		0,		1
		};
	}

	/// Get a view matrix based on an eye reference frame

	/// @param[in] ur		eye right direction unit vector
	/// @param[in] uu		eye up direction unit vector
	/// @param[in] ub		eye backward direction unit vector
	/// @param[in] eyePos	eye position
	static Matrix4 lookAt(const Vec<3,T>& ur, const Vec<3,T>& uu, const Vec<3,T>& ub, const Vec<3,T>& eyePos){
		auto m = Matrix4(ur,uu,ub, eyePos);
		invertRigid(m);
		return m;
	}

	/// Get a view matrix based on look-at parameters

	/// @param[in] eyePos	eye position
	/// @param[in] at		point being looked at
	/// @param[in] up		up vector
	static Matrix4 lookAt(const Vec<3,T>& eyePos, const Vec<3,T>& at, const Vec<3,T>& up){
		auto ub = (eyePos - at).normalize();
		auto ur = cross(up, ub).normalize();
		auto uu = cross(ub, ur);
		return lookAt(ur, uu, ub, eyePos);
	}
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
