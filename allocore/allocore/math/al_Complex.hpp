#ifndef INCLUDE_AL_COMPLEX_HPP
#define INCLUDE_AL_COMPLEX_HPP

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
	Complex number class

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include <cmath>
#include <utility> // swap
#include "allocore/math/al_Constants.hpp"

namespace al {

template <class T> class Complex;
template <class T> class Polar;

typedef Polar<float>	Polarf;
typedef Polar<double>	Polard;
typedef Complex<float>	Complexf;
typedef Complex<double>	Complexd;



/// Polar number
///
/// @ingroup allocore
template <class T>
class Polar{
public:

	union{
		struct{
			T m;		///< Magnitude
			T p;		///< Phase, in radians
		};
		T elems[2];
	};

	/// @param[in] phs		phase, in radians
	Polar(const T& phs=T(0)): m(T(1)), p(phs){}

	/// @param[in] mag		magnitude
	/// @param[in] phs		phase, in radians
	Polar(const T& mag, const T& phs): m(mag), p(phs){}

	/// @param[in] v		rectangular complex number to convert from
	Polar(const Complex<T>& v){ *this = v; }

	Polar& operator = (const Complex<T>& v){ m=v.norm(); p=v.arg(); return *this; }
};


/// Complex number
///
/// @ingroup allocore
template <class T>
class Complex{
public:

	typedef Complex<T> C;
	typedef T value_type;

	union{
		struct{
			T r;		///< Real component
			T i;		///< Imaginary component
		};
		T elems[2];
	};

	Complex(const Complex& v): r(v.r), i(v.i){}
	Complex(const Polar<T>& v){ *this = v; }
	Complex(const T& r=T(0), const T& i=T(0)): r(r), i(i){}
	Complex(const T& m, const T& p, int fromPolar){ (*this) = Polar<T>(m,p); }


	C& arg(T v){ return fromPolar(norm(), v); }					///< Set argument leaving norm the same
	C& norm(T v){ return fromPolar(v, arg()); }					///< Set norm leaving argument the same

	C& fromPolar(T phase){ r=::cos(phase); i=::sin(phase); return *this; }	///< Set phase and normalize
	C& fromPolar(T m, T p){ return set(Polar<T>(m,p)); }		///< Set magnitude and phase

	/// Set real and imaginary components
	C& set(T vr, T vi){ r=vr; i=vi; return *this; }
	C& set(const Polar<T>& p){ return *this = p; }

	T& operator[](int i){ return elems[i];}
	const T& operator[](int i) const { return elems[i]; }

	// Accessors compatible with std::complex
	T& real(){return r;}
	const T& real() const {return r;}
	T& imag(){return i;}
	const T& imag() const {return i;}

	bool operator ==(const C& v) const { return (r==v.r) && (i==v.i); }		///< Returns true if all components are equal
	bool operator ==(const T& v) const { return (r==v  ) && (i==T(0));}		///< Returns true if real and equals value
	bool operator !=(const C& v) const { return (r!=v.r) || (i!=v.i); }		///< Returns true if any components are not equal
	bool operator > (const C& v) const { return normSqr() > v.normSqr(); }	///< Returns true if norm is greater than argument's norm
	bool operator < (const C& c) const { return normSqr() < c.normSqr(); }	///< Returns true if norm is less than argument's norm

	C& operator = (const Polar<T>& v){ r=v.m*::cos(v.p); i=v.m*::sin(v.p); return *this; }
	C& operator = (const C& v){ r=v.r; i=v.i; return *this; }
	C& operator = (T v){ r=v;   i=T(0); return *this; }
	C& operator -=(const C& v){ r-=v.r; i-=v.i; return *this; }
	C& operator -=(T v){ r-=v; return *this; }
	C& operator +=(const C& v){ r+=v.r; i+=v.i; return *this; }
	C& operator +=(T v){ r+=v; return *this; }
	C& operator *=(const C& v){ return set(r*v.r - i*v.i, i*v.r + r*v.i); }
	C& operator *=(T v){ r*=v; i*=v; return *this; }
	C& operator /=(const C& v){ return (*this) *= v.recip(); }
	C& operator /=(T v){ r/=v; i/=v; return *this; }

	C& muli(){ std::swap(r,i); i=-i; return *this; } ///< Multiply by i (rotate 90 deg counter-clockwise)
	C& divi(){ std::swap(r,i); r=-r; return *this; } ///< Divide by i (multiply by -i)

	C operator - () const { return C(-r, -i); }
	C operator - (const C& v) const { return C(*this) -= v; }
	C operator - (T v) const { return C(*this) -= v; }
	C operator + (const C& v) const { return C(*this) += v; }
	C operator + (T v) const { return C(*this) += v; }
	C operator * (const C& v) const { return C(*this) *= v; }
	C operator * (T v) const { return C(*this) *= v; }
	C operator / (const C& v) const { return C(*this) /= v; }
	C operator / (T v) const { return C(*this) /= v; }

	T arg() const { return atan2(i, r); }					///< Returns argument in [-pi, pi]
	T argUnit() const { T r=arg()/(2*M_PI); return r>0 ? r : r+1; }	///< Return argument in unit interval [0, 1)
	C conj() const { return C(r,-i); }						///< Returns conjugate, z*
	T dot(const C& v) const { return r*v.r + i*v.i; }		///< Returns vector dot product
	C exp() const { return Polar<T>(::exp(r), i); }			///< Returns e^z
	C log() const { return Complex<T>(T(0.5)*::log(normSqr()), arg()); } ///< Returns log(z)
	T norm() const { return ::sqrt(normSqr()); }			///< Returns norm (radius), |z|
	T normSqr() const { return dot(*this); }				///< Returns square of norm, |z|^2
	C& normalize(T m=T(1)){ return *this *= (m/norm()); }	///< Sets magnitude to 1, |z|=1
	C pow(const C& v) const { return ((*this).log()*v).exp(); }	///< Returns z^v
	C pow(T v) const { return ((*this).log()*v).exp(); }	///< Returns z^v
	C recip() const { return conj()/normSqr(); }			///< Return multiplicative inverse, 1/z
	C sgn(T m=T(1)) const { return C(*this).normalize(m); }	///< Returns signum, z/|z|, the closest point on unit circle
	C sqr() const { return C(r*r-i*i, T(2)*r*i); }			///< Returns square

	/// Returns square root
	C sqrt() const {
		static const T c = T(1)/::sqrt(T(2));
		T n = norm();
		T a = ::sqrt(n+r) * c;
		T b = ::sqrt(n-r) * (i<T(0) ? -c : c);
		return C(a,b);
	}

	C cos()  const { return C(::cos(r)*::cosh(i),-::sin(r)*::sinh(i)); } ///< Returns cos(z)
	C sin()  const { return C(::sin(r)*::cosh(i), ::cos(r)*::sinh(i)); } ///< Returns sin(z)
	C cosh() const { return C(::cos(i)*::sinh(r), ::sin(i)*::cosh(r)); } ///< Returns cosh(z)
	C sinh() const { return C(::cos(i)*::cosh(r), ::sin(i)*::sinh(r)); } ///< Returns sinh(z)

	T abs() const { return norm(); }						///< Returns norm (radius), |z|
	T mag() const { return norm(); }						///< Returns norm (radius), |z|
	T magSqr() const { return normSqr(); }					///< Returns square of norm, |z|^2
	T phase() const { return arg(); }						///< Returns argument (angle)
};

#define TEM template <class T> inline
TEM T abs(const Complex<T>& c){ return c.mag(); }
TEM Complex<T> exp(const Complex<T>& c){ return c.exp(); }
TEM Complex<T> log(const Complex<T>& c){ return c.log(); }
TEM Complex<T> pow(const Complex<T>& b, const Complex<T>& e){ return b.pow(e); }
TEM Complex<T> pow(const Complex<T>& b, const T& e){ return b.pow(e); }
//TEM Complex<T> sqrt(const Complex<T>& v){ return v.sqrt(); } // TODO: these ambiguate other functions
//TEM Complex<T> cos(const Complex<T>& v){ return v.cos(); }
//TEM Complex<T> sin(const Complex<T>& v){ return v.sin(); }
#undef TEM

template <class T>
inline Complex<T> operator + (T r, const Complex<T>& c){ return  c+r; }

template <class T>
inline Complex<T> operator - (T r, const Complex<T>& c){ return -c+r; }

template <class T>
inline Complex<T> operator * (T r, const Complex<T>& c){ return  c*r; }

template <class T>
inline Complex<T> operator / (T r, const Complex<T>& c){ return  c.conj()*(r/c.normSqr()); }


template <class VecN, class T>
VecN rotate(const VecN& v, const VecN& p, const Complex<T>& a){
	return v*a.r + p*a.i;
}

/// Rotates two vectors by angle in plane formed from bivector v1 ^ v2
///
/// @ingroup allocore
template <class VecN, class T>
void rotatePlane(VecN& v1, VecN& v2, const Complex<T>& a){
	VecN t = al::rotate(v1, v2, a);
	v2 = al::rotate(v2, VecN(-v1), a);
	v1 = t;
}


/// Stereographically project complex number onto Riemann sphere
///
/// @ingroup allocore
template <class Vec3, class T>
Vec3 sterProj(const al::Complex<T>& c){
	T magSqr = c.magSqr();
	T mul = T(2)/(magSqr + T(1));
	return Vec3(
		c.r*mul,
		c.i*mul,
		(magSqr - T(1))*mul*T(0.5)
	);
}

} // al::

#endif
