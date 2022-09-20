#ifndef INCLUDE_AL_SPHERICAL_HPP
#define INCLUDE_AL_SPHERICAL_HPP

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
	A collection of classes related to spherical geometry

	File author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/


#include "allocore/math/al_Complex.hpp"
#include "allocore/math/al_Functions.hpp"
#include "allocore/math/al_Vec.hpp"

namespace al{

template <class T> class SphereCoord;

/// @addtogroup allocore
/// @{

typedef SphereCoord<float> SphereCoordf;	///< float SphereCoord
typedef SphereCoord<double> SphereCoordd;	///< double SphereCoord



/// Convert spherical to Cartesian coordinates in-place

/// @param[in,out] r2x		radius to x coordinate
/// @param[in,out] t2y		theta (angle on xy plane), in [-pi, pi], to z coordinate
/// @param[in,out] p2z		phi (angle from z axis), in [0, pi], to y coordinate
template<class T> void sphericalToCart(T& r2x, T& t2y, T& p2z);

/// Convert spherical to Cartesian coordinates in-place
template<class T> void sphericalToCart(T * vec3);

/// Convert Cartesian to spherical coordinates in-place

/// @param[in,out] x2r		x coordinate to radius
/// @param[in,out] y2t		y coordinate to theta (angle on xy plane), in [-pi, pi]
/// @param[in,out] z2p		z coordinate to phi (angle from z axis), in [0, pi]
template<class T> void cartToSpherical(T& x2r, T& y2t, T& z2p);

/// Convert Cartesian to spherical coordinates in-place
template<class T> void cartToSpherical(T * vec3);

/// Stereographic projection from an n-sphere to an n-1 dimensional hyperplane

/// \tparam N		dimensions of sphere
/// \tparam T		element type
/// @param[in] v	unit n-vector describing point on n-sphere
/// \returns		vector describing projected coordinate on n-1 hyperplane
template <int N, class T>
Vec<N-1,T> sterProj(const Vec<N,T>& v);



/// Spherical coordinate in terms of two complex numbers

/// The first component, theta, is the angle on the x-y plane and the second
/// component, phi, is the angle from the +z axis. The magnitude of theta
/// should always be 1, while the magnitude of phi is the radius.
template <class T>
class SphereCoord {
public:
	typedef Complex<T> C;

	C t;	///< Theta component, longitudinal angle (angle from +x towards +y)
	C p;	///< Phi component, latitudinal angle (angle from +z axis)

	///
	SphereCoord(const C& theta =C(1,0), const C& phi =C(1,0))
	:	t(theta), p(phi){}

	/// @param[in] v	Cartesian position
	template <class U>
	SphereCoord(const Vec<3,U>& v){ fromCart(v); }


	/// Get negation in Cartesian space
	SphereCoord  operator - () const { return SphereCoord(t, -p); }
	SphereCoord& operator *=(T v){ p*=v; return *this; }
	SphereCoord  operator * (T v) const { return SphereCoord(t, p*v); }

	/// Get radius
	T radius() const { return p.mag(); }

	/// Returns Cartesian coordinate
	Vec<3,T> toCart() const{
		return Vec<3,T>(t.r*p.i, t.i*p.i, p.r);
	}

	/// Set from two angles, in radians, and radius

	/// @param[in] theta	longitudinal angle (angle from +x towards +y)
	/// @param[in] phi		latitudinal angle (angle from +z axis)
	/// @param[in] radius	radius
	SphereCoord& fromAngle(const T& theta, const T& phi, const T& radius =T(1)){
		t.fromPolar(theta);
		p.fromPolar(radius, phi);
		return *this;
	}

	/// Set from Cartesian coordinate
	template <class U>
	SphereCoord& fromCart(const Vec<3,U>& v){
		t.set(v[0], v[1]);
		T tmag = t.mag();
		p.set(v[2], tmag);
		tmag != 0 ? t*=(1./tmag) : t.set(1,0);
		return *this;
	}
};



/// Spherical harmonic evaluator using cached coefficients

/// Spherical harmonics are solutions to Laplace's differential equation on the
/// surface of a 2-sphere. The solutions are complex functions parameterized by
/// two integers, l and m, and two angles defining an orientation in space.
/// The l number determines the number of nodal lines (circles with zero
/// magnitude) and m determines the number of latitudinal nodal lines
/// (geodesics intersecting the z axis). When |m| = l, the harmonics are "beach
/// ball"-like (sectoral) and when m = 0, the harmonics are "target"-like
/// (zonal). Other values of m produce a checkerboard pattern (tesseral).
/// Th Condon-Shortley phase factor of (-1)^m is included.
template <int L_MAX=16>
class SphericalHarmonic{
public:

	SphericalHarmonic(){
		createLUT();
	}

	/// Evaluate spherical harmonic

	/// @param[in] l		number of nodal lines
	/// @param[in] m		number of latitudinal nodal lines, |m| <= l
	/// @param[in] ctheta	unit magnitude complex number describing longitudinal angle in [0, 2pi]
	/// @param[in] cphi		unit magnitude complex number describing latitudinal angle in [0, pi]
	template <class T>
	Complex<T> operator()(int l, int m, const Complex<T>& ctheta, const Complex<T>& cphi) const {
		return coef(l,m) * al::legendreP(l, al::abs(m), cphi.r, cphi.i) * expim(m, ctheta);
	}

	template <class T>
	static Complex<T> expim(int m, const Complex<T>& ctheta){
		Complex<T> res = al::powN(ctheta, al::abs(m));
		if(m < 0) res.i = -res.i;
		return res;
	}

	/// Get normalization coefficient
	static double coef(int l, int m){ return l<=L_MAX ? coefTab(l,m) : coefCalc(l,m); }

	/// Get normalization coefficient (tabulated)
	static const double& coefTab(int l, int m){ return LUT(l,m); }

	/// Get normalization coefficient (calculated)
	static double coefCalc(int l, int m){
		int M = al::abs(m);
		double res = ::sqrt((2*l + 1) / M_4PI) * al::factorialSqrt(l-M) / al::factorialSqrt(l+M);
		return (m<0 && al::odd(M)) ? -res : res;
	}

private:
	// this holds precomputed coefficients for each basis
	static double& LUT(int l, int m){
		static double t[L_MAX+1][L_MAX*2+1];
		return t[l][m+L_MAX];
	}

	static void createLUT(){
		static bool make=true;
		if(make){
			make=false;
			for(int l=0; l<=L_MAX; ++l){
				for(int m=-L_MAX; m<=L_MAX; ++m){
					double c=0;
					 // m must be in [-l,l]
					if(al::abs(m) <= l)	c = coefCalc(l,m);
					LUT(l, m) = c;
				}
			}
		}
	}
};


/// Spherical harmonic function
static SphericalHarmonic<> spharm;


/// @} // end allocore group


// Implementation

template <class T>
void sphericalToCart(T& r, T& t, T& p){
	T rsinp = r * sin(p);
	T rcosp = r * cos(p);
	r = rsinp * cos(t);
	t = rsinp * sin(t);
	p = rcosp;
}

template <class T>
inline void sphericalToCart(T * vec3){ sphericalToCart(vec3[0], vec3[1], vec3[2]); }

template <class T>
void cartToSpherical(T& x, T& y, T& z){
	T r = sqrt(x*x + y*y + z*z);
	T t = atan2(y, x);
	z = acos(z/r);
	y = t;
	x = r;
}

template <class T>
inline void cartToSpherical(T * vec3){ cartToSpherical(vec3[0], vec3[1], vec3[2]); }

template <int N, class T>
inline Vec<N-1,T> sterProj(const Vec<N,T>& v){
	return sub<N-1>(v) * (T(1)/v[N-1]);
}

} // ::al
#endif
