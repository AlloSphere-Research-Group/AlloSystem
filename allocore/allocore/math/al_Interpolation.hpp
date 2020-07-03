#ifndef INCLUDE_AL_MATH_INTERPOLATION_HPP
#define INCLUDE_AL_MATH_INTERPOLATION_HPP

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
	A collection of generic interpolation functions

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include <initializer_list>

namespace al {

/// Utilities for interpolation
namespace ipl{

/// @addtogroup allocore
/// @{

/// Bezier curve, 3-point quadratic

/// @param[in] frac 	fraction between x2 and x0
///
template <class Tf, class Tv>
Tv bezier(Tf frac, const Tv& x2, const Tv& x1, const Tv& x0);

/// Bezier curve, 4-point cubic

/// @param[in] frac 	fraction between x3 and x0
///
template <class Tf, class Tv>
Tv bezier(Tf frac, const Tv& x3, const Tv& x2, const Tv& x1, const Tv& x0);

///	de Casteljau algorithm for four point interpolation

///	@param frac		Interpolation factor [0, 1]
///	@param a		First point
///	@param b		Second point
///	@param c		Third point
///	@param d		Fourth point
template <class Tf, class Tv>
Tv casteljau(const Tf& frac, const Tv& a, const Tv& b, const Tv& c, const Tv& d);

/// Hermite interpolation
template <class Tp, class Tv>
Tv hermite(Tp f, const Tv& w, const Tv& x, const Tv& y, const Tv& z, Tp tension, Tp bias);


/// Computes FIR coefficients for Waring-Lagrange interpolation

///		'h' are the FIR coefficients and should be of size ('order' + 1). \n
///		'delay' is a fractional delay in samples. \n
///		As order increases, this converges to sinc interpolation.
template <class T> void lagrange(T * h, T delay, int order);

/// Optimized lagrange() for first order.
template <class T> void lagrange1(T * h, T delay);

/// Optimized lagrange() for second order
template <class T> void lagrange2(T * h, T delay);

/// Optimized lagrange() for third order
template <class T> void lagrange3(T * h, T delay);


// Various functions to perform Waring-Lagrange interpolation.
//		These are much faster than using a general purpose FIR filter since
//		the coefs are computed directly and nested multiplication is used
//		rather than directly evaluating the polynomial (FIR).

/// Compute weights for cubic cardinal spline

/// @param[out] w	four output weights
/// @param[ in] x	four input domain values; spline in [x[1], x[2]]
/// @param[ in] f	fraction in [0,1]
/// @param[ in] b	smoothness parameter in [-1,1]; 1 = Catmull-Rom
template <class Tf, class Tv>
void cardinalSpline(Tv * w, const Tv * x, const Tf& f, double b);

/// Cubic interpolation (cardinal spline with tension 0, Catmull-Rom spline)
template <class Tf, class Tv>
Tv cubic(Tf frac, const Tv& w, const Tv& x, const Tv& y, const Tv& z);

/// Cubic interpolation (cardinal spline with a tension -1)
template <class Tf, class Tv>
Tv cubic2(Tf frac, const Tv& w, const Tv& x, const Tv& y, const Tv& z);

/// Linear interpolation.  Identical to first order Lagrange.
template <class Tf, class Tv>
Tv linear(Tf frac, const Tv& x, const Tv& y);

/// Linear interpolation between three elements
template <class Tf, class Tv>
Tv linear(Tf frac, const Tv& x, const Tv& y, const Tv& z);

/// Cyclic linear interpolation between three elements
template <class Tf, class Tv>
Tv linearCyclic(Tf frac, const Tv& x, const Tv& y, const Tv& z);

/// Element-wise linear interpolation between two arrays of values
template <class Tf, class Tv>
void linear(Tv * dst, const Tv * xs, const Tv * xp1s, int len, const Tf& frac);

/// Nearest neighbor interpolation
template <class Tf, class Tv>
Tv nearest(Tf frac, const Tv& x, const Tv& y);

/// Truncating interpolation into a list
template <class Tf, class Tv>
const Tv& trunc(Tf frac, const std::initializer_list<Tv>& src);

/// Bilinear interpolation between values on corners of quadrilateral
template <class Tf, class Tv>
inline Tv bilinear(
	const Tf& fracX, const Tf& fracY,
	const Tv& xy, const Tv& Xy,
	const Tv& xY, const Tv& XY
){
	return linear(fracY,
		linear(fracX, xy,Xy),
		linear(fracX, xY,XY)
	);
}

/// Bilinear interpolation between values on corners of quadrilateral
template <class Tf2, class Tv>
inline Tv bilinear(
	const Tf2& f,
	const Tv& xy, const Tv& Xy,
	const Tv& xY, const Tv& XY
){
	return bilinear(f[0],f[1],xy,Xy,xY,XY);
}

/// Trilinear interpolation between values on corners of a hexahedron
template <class Tf, class Tv>
inline Tv trilinear(
	const Tf& fracX, const Tf& fracY, const Tf& fracZ,
	const Tv& xyz, const Tv& Xyz,
	const Tv& xYz, const Tv& XYz,
	const Tv& xyZ, const Tv& XyZ,
	const Tv& xYZ, const Tv& XYZ
){
	return linear(fracZ,
		bilinear(fracX,fracY, xyz,Xyz,xYz,XYz),
		bilinear(fracX,fracY, xyZ,XyZ,xYZ,XYZ)
	);
}

/// Trilinear interpolation between values on corners of a hexahedron

/// @param[in] f		3 element array of fractions along x, y, and z
///
template <class Tf3, class Tv>
inline Tv trilinear(
	const Tf3& f,
	const Tv& xyz, const Tv& Xyz,
	const Tv& xYz, const Tv& XYz,
	const Tv& xyZ, const Tv& XyZ,
	const Tv& xYZ, const Tv& XYZ
){
	return trilinear(f[0],f[1],f[2],xyz,Xyz,xYz,XYz,xyZ,XyZ,xYZ,XYZ);
}

/// @} // end allocore group



//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------

template <class Tf, class Tv>
inline Tv bezier(Tf d, const Tv& x2, const Tv& x1, const Tv& x0){
	Tf d2 = d * d;
	Tf dm1 = Tf(1) - d;
	Tf dm12 = dm1 * dm1;
	return dm12 * x2 + Tf(2)*dm1*d * x1 + d2 * x0;

//	x2 (1-d)(1-d) + 2 x1 (1-d) d + x0 d d
//	x2 - d 2 x2 + d d x2 + d 2 x1 - d d 2 x1 + d d x0
//	x2 - d (2 x2 + d x2 + 2 x1 - d 2 x1 + d x0)
//	x2 - d (2 (x2 + x1) + d (x2 - 2 x1 + x0))

//	float c2 = x2 - 2.f * x1 + x0;
//	float c1 = 2.f * (x2 + x1);
//	return x2 - (d * c2 + c1) * d;
}

template <class Tf, class Tv>
inline Tv bezier(Tf d, const Tv& x3, const Tv& x2, const Tv& x1, const Tv& x0){
	Tv c1 = Tf(3) * (x2 - x3);
	Tv c2 = Tf(3) * (x1 - x2) - c1;
	Tv c3 = x0 - x3 - c1 - c2;
	return ((c3 * d + c2) * d + c1) * d + x3;
}

template <class Tf, class Tv>
Tv casteljau(const Tf& f, const Tv& a, const Tv& b, const Tv& c, const Tv& d){
	Tv ab = linear(f,a,b);
	Tv bc = linear(f,b,c);
	Tv cd = linear(f,c,d);
	return linear(
		f,
		linear(f, ab, bc),
		linear(f, bc, cd)
	);
}

template <class Tf, class Tv>
inline void cardinalSpline(Tv * w, const Tv * x, const Tf& f, double b){

	//c = (1-b);		// tension
	b *= (x[2]-x[1]);	// make domain [t[1], t[2]]

	// evaluate the Hermite basis functions
	Tf h00 = (1 + 2*f)*(f-1)*(f-1);
	Tf h10 = f*(f-1)*(f-1);
	Tf h01 = f*f*(3-2*f);
	Tf h11 = f*f*(f-1);

	/*
	The general cubic Hermite spline is
		p(f) = h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1

	For a cardinal spline, the tangent at point k is
		m[k] = (1-c) * (p[k+1] - p[k-1]) / (x[k+1] - x[k-1])

	To get the weights for each point, we plug the tangents into the general
	spline equation and factor out the p[k].
	*/

	w[0] = (    - b*h10/(x[2]-x[0]));
	w[1] = (h00 - b*h11/(x[3]-x[1]));
	w[2] = (h01 + b*h10/(x[2]-x[0]));
	w[3] = (    + b*h11/(x[3]-x[1]));
}

template <class Tf, class Tv>
inline Tv cubic(Tf f, const Tv& w, const Tv& x, const Tv& y, const Tv& z){
//	Tv c3 = (x - y)*(Tf)1.5 + (z - w)*(Tf)0.5;
//	Tv c2 = w - x*(Tf)2.5 + y*(Tf)2. - z*(Tf)0.5;
//	Tv c1 = (y - w)*(Tf)0.5;
//	return ((c3 * f + c2) * f + c1) * f + x;

	// -w + 3x - 3y + z
	// 2w - 5x + 4y - z
	// c2 = w - 2x + y - c3

//	Tv c3 = (x - y)*(Tf)3 + z - w;
//	Tv c2 = w - x*(Tf)2 + y - c3;
//	Tv c1 = y - w;
//	return (((c3 * f + c2) * f + c1)) * f * (Tf)0.5 + x;

//	Tv c3 = (x - y)*(Tf)1.5 + (z - w)*(Tf)0.5;
//	Tv c2 = (y + w)*(Tf)0.5 - x - c3;
//	Tv c1 = (y - w)*(Tf)0.5;
//	return ((c3 * f + c2) * f + c1) * f + x;

	Tv c1 = (y - w)*Tf(0.5);
	Tv c3 = (x - y)*Tf(1.5) + (z - w)*Tf(0.5);
	Tv c2 = c1 + w - x - c3;
	return ((c3 * f + c2) * f + c1) * f + x;
}

template <class T>
void cubic(T * dst, const T * xm1s, const T * xs, const T * xp1s, const T * xp2s, int len, T f){
	for(int i=0; i<len; ++i) dst[i] = cubic(f, xm1s[i], xs[i], xp1s[i], xp2s[i]);
}

// From http://astronomy.swin.edu.au/~pbourke/other/interpolation/ (Paul Bourke)
template <class Tf, class Tv>
inline Tv cubic2(Tf f, const Tv& w, const Tv& x, const Tv& y, const Tv& z){
	Tv c3 = z - y - w + x;
	Tv c2 = w - x - c3;
	Tv c1 = y - w;
	return ((c3 * f + c2) * f + c1) * f + x;
}

// From http://astronomy.swin.edu.au/~pbourke/other/interpolation/ (Paul Bourke)
/*
   Tension: 1 is high, 0 normal, -1 is low
   Bias: 0 is even,
         positive is towards first segment,
         negative towards the other
*/
template <class Tp, class Tv>
inline Tv hermite(Tp f,
	const Tv& w, const Tv& x, const Tv& y, const Tv& z,
	Tp tension, Tp bias)
{
	tension = (Tp(1) - tension)*Tp(0.5);

	// compute endpoint tangents
	//Tv m0 = ((x-w)*(1+bias) + (y-x)*(1-bias))*tension;
	//Tv m1 = ((y-x)*(1+bias) + (z-y)*(1-bias))*tension;
	Tv m0 = ((x*Tv(2) - w - y)*bias + y - w)*tension;
	Tv m1 = ((y*Tv(2) - x - z)*bias + z - x)*tension;

//	x - w + x b - w b + y - x - y b + x b
//	-w + 2x b - w b + y - y b
//	b(2x - w - y) + y - w
//
//	y - x + y b - x b + z - y - z b + y b
//	-x + 2y b - x b + z - z b
//	b(2y - x - z) + z - x

	Tp f2 = f  * f;
	Tp f3 = f2 * f;

	// compute hermite basis functions
	Tp a3 = Tp(-2)*f3 + Tp(3)*f2;
	Tp a0 = Tp(1) - a3;
	Tp a2 = f3 - f2;
	Tp a1 = f3 - Tp(2)*f2 + f;

	return x*a0 + m0*a1 + m1*a2 + y*a3;
}

template <class T> void lagrange(T * a, T delay, int order){
	for(int i=0; i<=order; ++i){
		T coef = T(1);
		T i_f = T(i);
		for(int j=0; j<=order; ++j){
			if(j != i){
				T j_f = (T)j;
				coef *= (delay - j_f) / (i_f - j_f);
			}
		}
		*a++ = coef;
	}
}

template <class T> inline void lagrange1(T * h, T d){
	h[0] = T(1) - d;
	h[1] = d;
}

template <class T> inline void lagrange2(T * h, T d){
	h[0] =      (d - T(1)) * (d - T(2)) * T(0.5);
	h[1] = -d              * (d - T(2))         ;
	h[2] =  d * (d - T(1))              * T(0.5);
}

template <class T> inline void lagrange3(T * h, T d){
	T d1 = d - T(1);
	T d2 = d - T(2);
	T d3 = d - T(3);
	h[0] =     -d1 * d2 * d3 * T(1./6.);
	h[1] =  d      * d2 * d3 * T(0.5);
	h[2] = -d * d1      * d3 * T(0.5);
	h[3] =  d * d1 * d2      * T(1./6.);
}

/*
x1 (1 - d) + x0 d
x1 - x1 d + x0 d
x1 + (x0 - x1) d

x2 (d - 1) (d - 2) /2 - x1 d (d - 2) + x0 d (d - 1) /2
d d /2 x2 - d 3/2 x2 + x2 - d d x1 + d 2 x1 + d d /2 x0 - d /2 x0
d d /2 x2 - d d x1 + d d /2 x0 - d 3/2 x2 + d 2 x1 - d /2 x0 + x2
d (d (/2 x2 - x1 + /2 x0) - 3/2 x2 + 2 x1 - /2 x0) + x2
*/

template <class Tf, class Tv>
inline Tv linear(Tf f, const Tv& x, const Tv& y){
	return (y - x) * f + x;
}

template <class Tf, class Tv>
inline Tv linear(Tf frac, const Tv& x, const Tv& y, const Tv& z){
	frac *= Tf(2);
	if(frac<Tf(1)) return ipl::linear(frac, x,y);
	return ipl::linear(frac-Tf(1), y,z);
}

template <class Tf, class Tv>
void linear(Tv * dst, const Tv * xs, const Tv * xp1s, int len, const Tf& f){
	for(int i=0; i<len; ++i) dst[i] = linear(f, xs[i], xp1s[i]);
}

template <class Tf, class Tv>
inline Tv linearCyclic(Tf frac, const Tv& x, const Tv& y, const Tv& z){
	frac *= Tf(3);
	if(frac <= Tf(1))		return ipl::linear(frac, x,y);
	else if(frac >= Tf(2))	return ipl::linear(frac-Tf(2), z,x);
							return ipl::linear(frac-Tf(1), y,z);
}

template <class Tf, class Tv>
inline Tv nearest(Tf frac, const Tv& x, const Tv& y){
	return (frac < Tf(0.5)) ? x : y;
}

template <class Tf, class Tv>
inline const Tv& trunc(Tf frac, const std::initializer_list<Tv>& src){
	int i = src.size()*frac;
	return *(src.begin()+i);
}

} // al::ipl
} // al::

#endif
