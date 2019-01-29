#ifndef INCLUDE_AL_MATH_FUNCTIONS_HPP
#define INCLUDE_AL_MATH_FUNCTIONS_HPP

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
	This includes various commonly used mathematical functions that are not
	included in the standard C/C++ math libraries.

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include <cmath>
#include "allocore/system/al_Config.h"
#include "allocore/math/al_Constants.hpp"


// Undefine these macros if found (in windows.h) in favor of proper functions
// defined in this file.
#ifdef sinc
#undef sinc
#endif

#ifdef __MINGW32__
	#ifndef AL_USE_STD_ROUND
		#define AL_USE_STD_ROUND
	#endif
#endif

namespace al {

/// @addtogroup allocore
/// @{

/// Returns absolute value
template<class T> T abs(const T& v);

/// Return whether two floats are almost equal

/// @param[in] a		first operand
/// @param[in] b		second operand
/// @param[in] maxULP	maximum "units in the last place"
///
/// Algorithm from Dawson, B. "Comparing floating point numbers",
/// http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
bool aeq(float a, float b, int maxULP=10);
bool aeq(double a, double b, int maxULP=10);

/// Convert amplitude to decibels
template <class T>
inline T ampTodB(const T& amp){ return 20*std::log(amp); }

/// Returns value clipped ouside of range [-eps, eps]
template<class T> T atLeast(const T& v, const T& eps);

/// Fast approximation to atan2().

// Author: Jim Shima, http://www.dspguru.com/comp.dsp/tricks/alg/fxdatan2.htm.
// |error| < 0.01 rad
template<class T> T atan2Fast(const T& y, const T& x);

/// Returns number of bits set to 1.

/// From "Bit Twiddling Hacks",
/// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
uint32_t bitsSet(uint32_t v);

/// Returns floating point value rounded to next highest integer.
template<class T> T ceil(const T& val);
template<class T> T ceil(const T& val, const T& step);
template<class T> T ceil(const T& val, const T& step, const T& recStep);

/// Returns even number ceiling
inline uint32_t ceilEven(uint32_t v){ return v += v & 1UL; }

/// Returns power of two ceiling of value

/// This uses an algorithm devised by Sean Anderson, Sep. 2001.
/// From "Bit Twiddling Hacks", http://graphics.stanford.edu/~seander/bithacks.html.
uint32_t ceilPow2(uint32_t value);

/// Returns value clipped to [lo, hi]
template<class T> T clip(const T& value, const T& hi=T(1), const T& lo=T(0));

/// Returns value clipped to [lo, hi] and signifies clipping behavior

/// clipFlag signifies if and where clipping occured.  0 means no clipping
/// occured, -1 means clipping occured at the lower bound, and 1 means
/// clipping at the upper bound.
template<class T> T clip(const T& v, int& clipFlag, const T& hi, const T& lo);

/// Returns value clipped to [-hi, hi].
template<class T> T clipS(const T& value, const T& hi=T(1));

/// Convert decibels to amplitude
template <class T>
inline T dBToAmp(const T& db){ return ::pow(10, db/20.); }

/// Returns whether or not an integer value is even.
template<class T> bool even(const T& v);

/// The Gauss error function or probability integral
/// @see http://en.wikipedia.org/wiki/Error_function
template<class T> T erf(const T& v);

/// Returns factorial. Argument must be less than or equal to 12.
uint32_t factorial(uint32_t n0to12);

/// Returns square root of factorial
double factorialSqrt(int v);

/// Returns floor of floating point value.
template<class T> T floor(const T& val);
template<class T> T floor(const T& val, const T& step);
template<class T> T floor(const T& val, const T& step, const T& recStep);

/// Returns power of two floor of value

/// This uses an algorithm devised by Sean Anderson, Sep. 2001.
/// From "Bit Twiddling Hacks", http://graphics.stanford.edu/~seander/bithacks.html.
uint32_t floorPow2(uint32_t value);

/// Returns value folded into [lo, hi]

/// For out-of-range values, the boundaries act like mirrors reflecting
/// the value into the range. For an even number of periods out of the range
/// this is identical to a wrap().
template<class T> T fold(const T& value, const T& hi=T(1), const T& lo=T(0));

/// Returns value folded into [lo, hi] one time.
template<class T> T foldOnce(const T& value, const T& hi=T(1), const T& lo=T(0));

/// Returns e^(-v*v)
template<class T> T gaussian(const T& v);

/// Return greatest common divisor of two arguments
template<class T> T gcd(const T& x, const T& y);

template <typename T, typename... Ts>
T gcd(const T& x, const Ts&... rest){ return gcd(x, gcd(rest...)); }

template <typename... Ts>
bool coprime(const Ts&... vals){ return gcd(vals...) == 1; }

/// The Gudermannian function

/// Relates circular and hyperbolic functions without using complex numbers.
/// @see http://en.wikipedia.org/wiki/Gudermannian_function
template<class T> T gudermannian(const T& x);

/// Returns true if integer is prime
bool isPrime(unsigned n);

/// Generalized Laguerre polynomial L{n,k}

/// @param[in] n	degree, a non-negative integer
/// @param[in] k	order
/// @param[in] x	position
/// http://en.wikipedia.org/wiki/Laguerre_polynomials
template<class T> T laguerreL(int n, int k, T x);

/// Returns least common multiple
template<class T> T lcm(const T& x, const T& y);

/// Associated Legendre polynomial
///
/// P_l^m(cos(t)) = (-1)^{l+m} / (2^l l!) sin^m(t) (d/d cos(t))^{l+m} sin^{2l}(t)
///
/// @param[in]	l	degree where l >= 0
/// @param[in]	m	order  where 0 <= m <= l
/// @param[in]	t	angle in [0, pi]
///
/// http://comp.cs.ehime-u.ac.jp/~ogata/nac/index.html
template<class T> T legendreP(int l, int m, T t);
template<class T> T legendreP(int l, int m, T ct, T st);

/// Returns whether the absolute value is less than an epsilon.
template<class T> bool lessAbs(const T& v, const T& eps=T(0.000001));

/// Returns base 2 logarithm of value.

/// If the value is not an exact power of two, the logarithm of the next
/// highest power of two will taken.
/// This uses an algorithm devised by Eric Cole, Jan. 2006.
/// From "Bit Twiddling Hacks", http://graphics.stanford.edu/~seander/bithacks.html.
uint32_t log2(uint32_t v);

/// Returns maximum of two values
template<class T> T max(const T& v1, const T& v2);

/// Returns maximum of three values
template<class T> T max(const T& v1, const T& v2, const T& v3);

/// Returns mean of two values
template<class T> T mean(const T& v1, const T& v2);

/// Returns minimum of two values
template<class T> T min(const T& v1, const T& v2);

/// Returns minimum of three values
template<class T> T min(const T& v1, const T& v2, const T& v3);

/// Returns nearest integer division of one value to another
template<class T> inline T nearestDiv(T of, T to);

/// Returns the next representable floating-point or integer value following x in the direction of y
template<class T> T nextAfter(const T& x, const T& y);

/// Returns next largest value of 'val' that is a multiple of 'multiple'.
template<class T> T nextMultiple(T val, T multiple);

/// Returns the number of digits in the integer portion
template<class T> T numInt(const T& v);

/// Returns whether or not an integer value is odd.
template<class T> bool odd(const T& v);

/// Evaluates polynomial a0 + a1 x + a2 x^2
template<class T> T poly(const T& x, const T& a0, const T& a1, const T& a2);

/// Evaluates polynomial a0 + a1 x + a2 x^2 + a3 x^3
template<class T> T poly(const T& x, const T& a0, const T& a1, const T& a2, const T& a3);

template<class T> T pow2(const T& v);		///< Returns value to the 2nd power.
template<class T> T pow2S(const T& v);		///< Returns value to the 2nd power preserving sign.
template<class T> T pow3(const T& v);		///< Returns value to the 3rd power.
template<class T> T pow3Abs(const T& v);	///< Returns absolute value to the 3rd power.
template<class T> T pow4(const T& v);		///< Returns value to the 4th power.
template<class T> T pow5(const T& v);		///< Returns value to the 5th power.
template<class T> T pow6(const T& v);		///< Returns value to the 6th power.
template<class T> T pow8(const T& v);		///< Returns value to the 8th power.
template<class T> T pow16(const T& v);		///< Returns value to the 16th power.
template<class T> T pow64(const T& v);		///< Returns value to the 64th power.

/// Returns value to a positive integer power

/// @param[in] base		the base value to exponentiate
/// @param[in] power	the power to exponentiate by
template<class T>
T powN(T base, unsigned power);

/// Returns (n+1)th prime number up to n=53.
unsigned char prime(uint32_t n);

/// Returns the value r such that r = x - n*y.
template<class T> T remainder(const T& x, const T& y);

/// Returns value rounded to nearest integer towards zero.
template<class T> T round(const T& v);

/// Returns value rounded to nearest integer multiple of 'step' towards zero.
template<class T> T round(const T& v, const T& step);

/// Returns value rounded to nearest integer multiple of 'step' towards zero. Faster version to avoid 1/step divide.
template<class T> T round(const T& v, const T& step, const T& recStep);

/// Returns value rounded to nearest integer away from zero.
template<class T> T roundAway(const T& v);

/// Returns value rounded to nearest to nearest integer multiple of 'step' away from zero.
template<class T> T roundAway(const T& v, const T& step);

/// Signum function for real numbers
template<class T> T sgn(const T& v, const T& norm=T(1));

/// Unnormalized sinc function
template<class T> T sinc(const T& radians, const T& eps=T(0.0001));

/// Returns slope of line passing through two points.
template<class T> T slope(const T& x1, const T& y1, const T& x2, const T& y2);

/// Sort values so that value1 <= value2.
template<class T> void sort(T& value1, T& value2);

/// Sum of integers squared from 1 to n.
template<class T> T sumOfSquares(T n);

/// Returns number of trailing zeros in 32-bit int

/// This implements an algorithm from the paper
/// "Using de Bruijn Sequences to Index 1 in a Computer Word"
/// by Charles E. Leiserson, Harald Prokof, and Keith H. Randall.
uint32_t trailingZeroes(uint32_t v);

/// Truncates floating point value at decimal.
template<class T> T trunc(const T& v);

/// Truncates floating point value to step.
template<class T> T trunc(const T& v, const T& step);

/// Truncates floating point value to step. Faster version to avoid 1/step divide.
template<class T> T trunc(const T& v, const T& step, const T& recStep);

/// Returns whether value is in interval [lo, hi].
template<class T> bool within(const T& v, const T& lo, const T& hi);

/// Returns whether 3 values are in interval [lo, hi].
template<class T> bool within3(const T& v1, const T& v2, const T& v3, const T& lo, const T& hi);

/// Returns whether value is in interval [lo, hi).
template<class T> bool withinIE(const T& v, const T& lo, const T& hi);

/// Returns value wrapped in [lo, hi).
template<class T> T wrap(const T& value, const T& hi=T(1), const T& lo=T(0));

/// Returns value wrapped in [lo, hi).

/// 'numWraps' reports how many wrappings occured where the sign, + or -,
/// signifies above 'hi' or below 'lo', respectively.
template<class T> T wrap(const T& value, long& numWraps, const T& hi=T(1), const T& lo=T(0));

/// Returns value incremented by 1 and wrapped into interval [0, max).
template<class T> T wrapAdd1(const T& v, const T& max){ ++v; return v == max ? 0 : v; }

/// Like wrap(), but only adds or subtracts 'hi' once from value.
template<class T> T wrapOnce(const T& value, const T& hi=T(1));

template<class T> T wrapOnce(const T& value, const T& hi, const T& lo);

/// Returns value wrapped in [-pi, pi)
template<class T> T wrapPhase(const T& radians);

/// Like wrapPhase(), but only wraps once
template<class T> T wrapPhaseOnce(const T& radians);


/// @} // end allocore group



// Implementation
//------------------------------------------------------------------------------

#define TEM template<class T>

namespace{
	template<class T> const T roundEps();
	template<> inline const float  roundEps<float >(){ return 0.499999925f; }
	template<> inline const double roundEps<double>(){ return 0.499999985; }

	inline uint32_t deBruijn(uint32_t v){
		static const uint32_t deBruijnBitPosition[32] = {
			 0, 1,28, 2,29,14,24, 3,30,22,20,15,25,17, 4, 8,
			31,27,13,23,21,19,16, 7,26,12,18, 6,11, 5,10, 9
		};
		return deBruijnBitPosition[(uint32_t(v * 0x077CB531UL)) >> 27];
	}

	const uint32_t mFactorial12u[13] = {
		1, 1, 2, 6, 24, 120, 720, 5040, 40320,
		362880, 3628800, 39916800, 479001600
	};

	const uint8_t mPrimes54[54] = {
	/*	  0    1    2    3    4    5    6    7    8    9   */
		  2,   3,   5,   7,  11,  13,  17,  19,  23,  29, // 0
		 31,  37,  41,  43,  47,  53,  59,  61,	 67,  71, // 1
		 73,  79,  83,  89,  97, 101, 103, 107, 109, 113, // 2
		127, 131, 137, 139, 149, 151, 157, 163, 167, 173, // 3
		179, 181, 191, 193, 197, 199, 211, 223, 227, 229, // 4
		233, 239, 241, 251								  // 5
	};
}

template<> inline float abs<float>(const float& v){ return std::abs(v); }
template<> inline double abs<double>(const double& v){ return std::abs(v); }
TEM inline T abs(const T& v){ return v>T(0) ? v : -v; }

inline bool aeq(float a, float b, int maxULP){
	// Make sure maxULP is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	//assert(maxULP > 0 && maxULP < 4 * 1024 * 1024);
	union{ float f; int32_t i; } u;
	u.f=a; int32_t ai = u.i;
	u.f=b; int32_t bi = u.i;
	// Make ai and bi lexicographically ordered as a twos-complement int
	if(ai < 0) ai = 0x80000000 - ai;
	if(bi < 0) bi = 0x80000000 - bi;
	return al::abs(ai - bi) <= maxULP;
}

inline bool aeq(double a, double b, int maxULP){
	// Make sure maxULP is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	//assert(maxULP > 0 && maxULP < 4 * 1024 * 1024);
	union{ double f; int64_t i; } u;
	u.f=a; int64_t ai = u.i;
	u.f=b; int64_t bi = u.i;
	// Make ai and bi lexicographically ordered as a twos-complement int
	if(ai < 0) ai = 0x8000000000000000ULL - ai;
	if(bi < 0) bi = 0x8000000000000000ULL - bi;
	return al::abs(ai - bi) <= maxULP;
}

TEM inline T atLeast(const T& v, const T& e){ return (v >= T(0)) ? max(v, e) : min(v, -e); }

TEM T atan2Fast(const T& y, const T& x){

	T r, angle;
	T ay = al::abs(y) + T(1e-10);      // kludge to prevent 0/0 condition

	if(x < T(0)){
		r = (x + ay) / (ay - x);
		angle = T(M_3PI_4);
	}
	else{
		r = (x - ay) / (x + ay);
		angle = T(M_PI_4);
	}

	angle += (T(0.1963)*r*r - T(0.9817))*r;
	return y < T(0) ? -angle : angle;
}

inline uint32_t bitsSet(uint32_t v){
	v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
	return ((v + ((v >> 4) & 0xF0F0F0F)) * 0x1010101) >> 24; // count
}

TEM inline T ceil(const T& v){ return al::round(v + roundEps<T>()); }
TEM inline T ceil(const T& v, const T& s){ return al::ceil(v/s)*s; }
TEM inline T ceil(const T& v, const T& s, const T& r){ return al::ceil(v*r)*s; }

inline uint32_t ceilPow2(uint32_t v){
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >>16;
	return v+1;
}

TEM inline T clip(const T& v, const T& hi, const T& lo){
	     if(v < lo) return lo;
	else if(v > hi)	return hi;
	return v;
}

TEM inline T clip(const T& v, int & clipFlag, const T& hi, const T& lo){
	clipFlag = 0;
	     if(v < lo){ clipFlag = -1; return lo; }
	else if(v > hi){ clipFlag =  1; return hi; }
	return v;
}

TEM inline T clipS(const T& v, const T& hi){ return al::clip(v, hi, -hi); }

TEM inline bool even(const T& v){ return 0 == al::odd(v); }

/// @see http://en.wikipedia.org/wiki/Error_function
TEM inline T erf(const T& x) {
	static T a = 0.147;
	const T x2 = x*x;
	const T ax2 = a * x2;
	return al::sgn(x)*std::sqrt(T(1) - std::exp(-x2*(T(4./M_PI) + ax2)/(T(1)+ax2)));
}

inline uint32_t factorial(uint32_t v){ return mFactorial12u[v]; }

inline double factorialSqrt(int v){
	if(v<=1) return 1;
	double r=1;
	for(int i=2; i<=v; ++i) r *= std::sqrt(i);
	return r;
}

TEM inline T floor(const T& v){ return al::round(v - roundEps<T>()); }
TEM inline T floor(const T& v, const T& s){ return al::floor(v/s)*s; }
TEM inline T floor(const T& v, const T& s, const T& r){ return al::floor(v*r)*s; }

inline uint32_t floorPow2(uint32_t v){
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return (v >> 1) + 1;
}

TEM inline T fold(const T& v, const T& hi, const T& lo){
	long numWraps;
	T R = al::wrap(v, numWraps, hi, lo);
	if(numWraps & 1) R = hi + lo - R;
	return R;
}

TEM inline T foldOnce(const T& v, const T& hi, const T& lo){
	if(v > hi) return hi + (hi - v);
	if(v < lo) return lo + (lo - v);
	return v;
}

TEM inline T gaussian(const T& v){ return std::exp(-v*v); }

TEM T gcd(const T& x, const T& y){
	if(y==T(0)) return al::abs(x);
	return al::gcd(y, al::remainder(x,y));
}

/// @see http://en.wikipedia.org/wiki/Gudermannian_function
TEM T gudermannian(const T& x) {
	return T(2) * std::atan(exp(x)) - T(M_PI_2);
}

inline bool isPrime(unsigned n){
	if(n<=1) return false;
	else if(n<=3) return true;
	else if(n%2 == 0 || n%3 == 0) return false;
	unsigned i = 5;
	while(i*i <= n){
		if(n%i == 0 || n%(i+2) == 0) return false;
		i += 6;
	}
	return true;
}

TEM T laguerreL(int n, int k, T x){
//	T res = 1, bin = 1;
//
//	for(int i=n; i>=1; --i){
//		bin = bin * (k+i) / (n + 1 - i);
//		res = bin - x * res / i;
//	}
//	return res;

	if(n <0) return T(0);

	T L1= 0, R = 1;
	for(int i=1; i<=n; ++i){
		T L0 = L1;
		L1 = R;
		R = ((2*i + k-1 - x)*L1 - (i + k-1)*L0)/i;
	}
	return R;
}


TEM inline T lcm(const T& x, const T& y){ return (x*y)/al::gcd(x,y); }

TEM T legendreP(int l, int m, T ct, T st){

	switch(l){
		case 0: return 1.;

		case 1:
			switch(m){
				case 0: return ct;
				case 1: return -st;
				default:return 0.;
			}

		case 2:
			switch(m){
				case 0: return -0.5 + 1.5*ct*ct;
				case 1: return -3.0*ct*st;
				case 2: return  3.0*st*st;
				default:return 0.;
			}

		case 3:
			switch(m){
				case 0: return ct*(-1.5 + 2.5*ct*ct);
				case 1: return (1.5 - 7.5*ct*ct)*st;
				case 2: return  15.*ct*st*st;
				case 3: return -15.*st*st*st;
				default:return 0.;
			}

		case 4:
			switch(m){
				case 0: ct*=ct; return 0.375 + ct*(-3.75 + 4.375*ct);
				case 1: return ct*(7.5 - 17.5*ct*ct)*st;
				case 2: return (-7.5 + 52.5*ct*ct)*st*st;
				case 3: return -105.*ct*st*st*st;
				case 4: st*=st; return 105.*st*st;
				default:return 0.;
			}

		default:;
	}

//	if(l<0){ /*printf("l=%d. l must be non-negative.\n");*/ return 0; }
//	if(m<-l || m>l){ /*printf("m=%d. m must be -l <= m <= l.\n");*/ return 0; }

	// First compute answer for |m|

	// compute P_l^m(x) by the recurrence relation
	//		(l-m)P_l^m(x) = x(2l-1)P_{l-1}^m(x) - (l+m-1)P_{l-2}^m(x)
	// with
	//		P_m^m(x) = (-1)^m (2m-1)!! (1-x)^{m/2},
	//		P_{m+1}^m(x) = x(2m+1) P_m^m(x).

	T P = 0;				// the result
	int M = al::abs(m);		// M = |m|
	T y1 = 1.;				// recursion state variable

	for(int i=1; i<=M; ++i)
		y1 *= -((i<<1) - 1) * st;

	if(l==M) P = y1;

	else{
		T y = ((M<<1) + 1) * ct * y1;
		if(l==(M+1)) P = y;

		else{
			T c = (M<<1) - 1;
			for(int k=M+2; k<=l; ++k){
				T y2 = y1;
				y1 = y;
				T d = c / (k - M);
				y = (2. + d) * ct * y1 - (1. + d) * y2;
			}
			P = y;
		}
	}

//	// In the case that m<0,
//	// compute P_n^{-|m|}(x) by the formula
//	//		P_l^{-|m|}(x) = (-1)^{|m|}((l-|m|)!/(l+|m|)!)^{1/2} P_l^{|m|}(x).
//	// NOTE: when l and |m| are large, we risk numerical underflow...
//	if(m<0){
//		for(int i=l-M+1; i<=l+M; ++i) P *= 1. / i;
//		if(al::odd(M)) P = -P;
//	}

	return P;
}

TEM T legendreP(int l, int m, T t){
	return al::legendreP(l,m, std::cos(t), std::sin(t));
}

TEM inline bool lessAbs(const T& v, const T& eps){ return al::abs(v) < eps; }

inline uint32_t log2(uint32_t v){ return deBruijn(al::ceilPow2(v)); }

TEM inline T max(const T& v1, const T& v2){ return v1<v2?v2:v1; }
TEM inline T max(const T& v1, const T& v2, const T& v3){ return al::max(al::max(v1,v2),v3); }
TEM inline T mean(const T& v1, const T& v2){ return (v1 + v2) * T(0.5); }
TEM inline T min(const T& v1, const T& v2){ return v1<v2?v1:v2; }
TEM inline T min(const T& v1, const T& v2, const T& v3){ return al::min(al::min(v1,v2),v3); }

TEM inline T nearestDiv(T of, T to){ return to / al::round(to/of); }

#ifdef __MSYS__
// MSYS2 does not support C++11 std::nextafter like it should
template<class T>
inline T nextAfter(const T& x, const T& y){ return x<y ? x+1 : x-1; }
template<>
inline float nextAfter(const float& x, const float& y){ return nextafterf(x,y); }
template<>
inline double nextAfter(const double& x, const double& y){ return nextafter(x,y); }
#else
TEM inline T nextAfter(const T& x, const T& y){ return std::nextafter(x,y); }
#endif

TEM inline T nextMultiple(T v, T m){
	uint32_t div = (uint32_t)(v / m);
	return T(div + 1) * m;
}

TEM inline T numInt(const T& v){ return al::floor(std::log10(v)) + 1; }

TEM inline bool odd(const T& v){ return v & T(1); }

TEM inline T poly(const T& v, const T& a0, const T& a1, const T& a2){ return a0 + v*(a1 + v*a2); }
TEM inline T poly(const T& v, const T& a0, const T& a1, const T& a2, T a3){ return a0 + v*(a1 + v*(a2 + v*a3)); }

TEM inline T pow2 (const T& v){ return v*v; }
TEM inline T pow2S(const T& v){ return v*al::abs(v); }
TEM inline T pow3 (const T& v){ return v*v*v; }
TEM inline T pow3Abs(const T& v){ return al::abs(pow3(v)); }
TEM inline T pow4 (const T& v){ return pow2(pow2(v)); }
TEM inline T pow5 (const T& v){ return v * pow4(v); }
TEM inline T pow6 (const T& v){ return pow3(pow2(v)); }
TEM inline T pow8 (const T& v){ return pow4(pow2(v)); }
TEM inline T pow16(const T& v){ return pow4(pow4(v)); }
TEM inline T pow64(const T& v){ return pow8(pow8(v)); }

TEM inline T powN(T base, unsigned power){
	switch(power){
		case 0: return T(1);
		case 1: return base;
		case 2: return pow2(base);
		case 3: return pow3(base);
		case 4: return pow4(base);
		case 5: return pow5(base);
		case 6: return pow6(base);
		case 7: return pow6(base)*base;
		case 8: return pow8(base);
		case 9: return pow8(base)*base;
		default:{
			T r = pow8(base)*pow2(base);
			for(unsigned i=10; i<power; ++i) r *= base;
			return r;
		}
	}
}

inline uint8_t prime(uint32_t n){ return mPrimes54[n]; }

TEM inline T remainder(const T& x, const T& y){ return x - long(x/y) * y; }

TEM inline T round(const T& v){
#ifdef AL_USE_STD_ROUND
	return std::round(v);
#else
	static const double roundMagic = 6755399441055744.; // 2^52 * 1.5
	double r=v;
	return (r + roundMagic) - roundMagic;
#endif
}
TEM inline T round(const T& v, const T& s){ return al::round<double>(v/s) * s; }
TEM inline T round(const T& v, const T& s, const T& r){ return al::round<T>(v * r) * s; }
TEM inline T roundAway(const T& v){ return v<T(0) ? al::floor(v) : al::ceil(v); }
TEM inline T roundAway(const T& v, const T& s){ return v<T(0) ? al::floor(v,s) : al::ceil(v,s); }

TEM inline T sgn(const T& v, const T& norm){ return v==T(0) ? T(0) : v<T(0) ? -norm : norm; }

TEM inline T sinc(const T& r, const T& eps){ return (al::abs(r) > eps) ? std::sin(r)/r : std::cos(r); }

TEM inline T slope(const T& x1, const T& y1, const T& x2, const T& y2){ return (y2-y1)/(x2-x1); }

TEM inline void sort(T& v1, T& v2){ if(v1>v2){ T t=v1; v1=v2; v2=t; } }

TEM inline T sumOfSquares(T n){
	static const T c1_6 = 1/T(6);
	static const T c2_6 = c1_6*T(2);
	return n*(n+1)*(c2_6*n+c1_6);
}

inline uint32_t trailingZeroes(uint32_t v){ return deBruijn(v & -v); }

TEM inline T trunc(const T& v){ return al::round( (v > (T)0) ? v-roundEps<T>() : v+roundEps<T>() ); }
TEM inline T trunc(const T& v, const T& s){ return al::trunc(v/s)*s; }
TEM inline T trunc(const T& v, const T& s, const T& r){ return al::trunc(v*r)*s; }

TEM inline bool within  (const T& v, const T& lo, const T& hi){ return !((v < lo) || (v > hi)); }
TEM inline bool withinIE(const T& v, const T& lo, const T& hi){ return (!(v < lo)) && (v < hi); }

TEM inline bool within3(const T& v1, const T& v2, const T& v3, const T& lo, const T& hi){
	return al::within(v1,lo,hi) && al::within(v2,lo,hi) && al::within(v3,lo,hi);
}

// TODO: fuse the following two functions
TEM inline T wrap(const T& v, const T& hi, const T& lo){
	if(lo == hi) return lo;

	T R = v;
	T diff = hi - lo;

	if(R >= hi){
		R -= diff;
		if(R >= hi) R -= diff * uint32_t((R - lo)/diff);
	}
	else if(R < lo){
		R += diff;

		// If value is very slightly less than 'lo', then less significant
		// digits might get truncated by adding a larger number.
		if(R==diff) return al::nextAfter(R, lo);

		if(R < lo) R += diff * uint32_t(((lo - R)/diff) + 1);
		if(R==diff) return lo;
	}
	return R;
}

TEM inline T wrap(const T& v, long& numWraps, const T& hi, const T& lo){
	if(lo == hi){ numWraps = 0xFFFFFFFF; return lo; }

	T R = v;
	T diff = hi - lo;
	numWraps = 0;

	if(R >= hi){
		R -= diff;
		if(R >= hi){
			numWraps = long((R - lo)/diff);
			R -= diff * numWraps;
		}
		++numWraps;
	}
	else if(R < lo){
		R += diff;
		if(R < lo){
			numWraps = long((R - lo)/diff) - 1;
			R -= diff * numWraps;
		}
		--numWraps;
	}
	return R;
}

TEM inline T wrapOnce(const T& v, const T& hi){
	     if(v >= hi ) return v - hi;
	else if(v < T(0)) return v + hi;
	return v;
}

TEM inline T wrapOnce(const T& v, const T& hi, const T& lo){
	     if(v >= hi) return v - hi + lo;
	else if(v <  lo) return v + hi - lo;
	return v;
}

TEM inline T wrapPhase(const T& r_){
	// The result is		[r+pi - 2pi floor([r+pi] / 2pi)] - pi
	// which simplified is	r - 2pi floor([r+pi] / 2pi) .
	T r = r_;
	if(r >= T(M_PI)){
		r -= T(M_2PI);
		if(r < T(M_PI)) return r;
		return r - T(long((r+M_PI)*M_1_2PI)  )*M_2PI;
	}
	else if (r < T(-M_PI)){
		r += T(M_2PI);
		if(r >= T(-M_PI)) return r;
		return r - T(long((r+M_PI)*M_1_2PI)-1)*M_2PI;
	}
	else return r;
}

TEM inline T wrapPhaseOnce(const T& r){
	if(r >= T(M_PI))		return r - T(M_2PI);
	else if(r < T(-M_PI))	return r + T(M_2PI);
	return r;
}

TEM inline T mapRange(T value, T inlow, T inhigh, T outlow, T outhigh){
  float tmp = (value - inlow) / (inhigh-inlow);
  return tmp*(outhigh-outlow) + outlow;
}

TEM inline T lerp(T src, T dest, T amt){
	return src*(T(1)-amt) + dest*amt;
}

#undef TEM
} // al::
#endif
