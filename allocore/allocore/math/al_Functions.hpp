#ifndef INC_AL_MATH_FUNCTIONS_HPP
#define INC_AL_MATH_FUNCTIONS_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Various commonly used mathematical functions that are not included in the
	standard C/C++ math libraries

	Author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include <cmath>
#include <utility> // forward
#include "allocore/system/al_Config.h"
#include "allocore/math/al_Constants.hpp"

// Undefine these macros if found (in windows.h) in favor of proper functions
// defined in this file.
#ifdef sinc
#undef sinc
#endif

//#ifdef __MINGW32__
	#ifndef AL_USE_STD_ROUND
		#define AL_USE_STD_ROUND
	#endif
//#endif

namespace al {

/// \addtogroup allocore
/// @{

// Define base functions first that other functions depend on

/// Returns absolute value
template<class T> T abs(const T& v){ return v>T(0) ? v : -v; }
template<> inline float abs<float>(const float& v){ return std::abs(v); }
template<> inline double abs<double>(const double& v){ return std::abs(v); }

/// Returns maximum of two values
template<class T> T max(const T& a, const T& b){ return a<b?b:a; }

/// Returns maximum of three values
template<class T> T max(const T& a, const T& b, const T& c){ return al::max(al::max(a,b),c); }

/// Returns minimum of two values
template<class T> T min(const T& a, const T& b){ return a<b?a:b; }

/// Returns minimum of three values
template<class T> T min(const T& a, const T& b, const T& c){ return al::min(al::min(a,b),c); }

/// Returns the value r such that r = x - n*y
template<class T> T remainder(const T& x, const T& y){ return x - long(x/y) * y; }

/// Returns value rounded to nearest integer towards zero
template<class T> T round(const T& v);

/// Returns value rounded to nearest integer multiple of 'step' towards zero
template<class T> T round(const T& v, const T& step);

/// Returns value rounded to nearest integer multiple of 'step' towards zero. Faster version to avoid 1/step divide.
template<class T> T round(const T& v, const T& step, const T& recStep);

/// Returns value rounded to nearest integer away from zero
template<class T> T roundAway(const T& v);

/// Returns value rounded to nearest to nearest integer multiple of 'step' away from zero
template<class T> T roundAway(const T& v, const T& step);

/// Signum function for real numbers
template<class T> T sgn(const T& v, const T& norm=T(1)){
	return v==T(0) ? T(0) : v<T(0) ? -norm : norm;
}

/// Returns whether or not an integer value is odd
template<class T> bool odd(const T& v){ return v & T(1); }

/// Returns whether or not an integer value is even
template<class T> bool even(const T& v){ return 0 == al::odd(v); }


//---- General Functions

/// Returns value clipped ouside of range [-eps, eps]
template<class T> T atLeast(const T& v, const T& eps){
	return v >= T(0) ? max(v, eps) : min(v, -eps);
}

/// Returns floating point value rounded to next highest integer
template<class T> T ceil(const T& v);
template<class T> T ceil(const T& v, const T& step);
template<class T> T ceil(const T& v, const T& step, const T& recStep);

/// Returns even number ceiling
inline uint32_t ceilEven(uint32_t v){ return v + (v&1); }

/// Returns power of two ceiling of value

/// This uses an algorithm devised by Sean Anderson, Sep. 2001.
/// From "Bit Twiddling Hacks", http://graphics.stanford.edu/~seander/bithacks.html.
inline uint32_t ceilPow2(uint32_t v){
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >>16;
	return v+1;
}

/// Returns value clipped to [lo, hi]
template<class T> T clip(const T& v, const T& hi=T(1), const T& lo=T(0)){
	     if(v < lo) return lo;
	else if(v > hi)	return hi;
	return v;
}

/// Returns value clipped to [lo, hi] and signifies clipping behavior

/// clipFlag signifies if and where clipping occured.  0 means no clipping
/// occured, -1 means clipping occured at the lower bound, and 1 means
/// clipping at the upper bound.
template<class T> T clip(const T& v, int& clipFlag, const T& hi, const T& lo){
	clipFlag = 0;
	     if(v < lo){ clipFlag = -1; return lo; }
	else if(v > hi){ clipFlag =  1; return hi; }
	return v;
}

/// Returns value clipped to [-hi, hi]
template<class T> T clipS(const T& v, const T& hi=T(1)){
	return al::clip(v, hi, -hi);
}

/// Returns floor of floating point value
template<class T> T floor(const T& v);
template<class T> T floor(const T& v, const T& step);
template<class T> T floor(const T& v, const T& step, const T& recStep);

/// Returns power of two floor of value

/// This uses an algorithm devised by Sean Anderson, Sep. 2001.
/// From "Bit Twiddling Hacks", http://graphics.stanford.edu/~seander/bithacks.html.
inline uint32_t floorPow2(uint32_t v){
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return (v >> 1) + 1;
}

/// Returns value folded into [lo, hi]

/// For out-of-range values, the boundaries act like mirrors reflecting
/// the value into the range. For an even number of periods out of the range
/// this is identical to a wrap().
template<class T> T fold(const T& v, const T& hi=T(1), const T& lo=T(0));

/// Returns value folded into [lo, hi] one time
template<class T> T foldOnce(const T& v, const T& hi=T(1), const T& lo=T(0));

/// Returns base 2 logarithm of value

/// If the value is not an exact power of two, the logarithm of the next
/// highest power of two will taken.
/// This uses an algorithm devised by Eric Cole, Jan. 2006.
/// From "Bit Twiddling Hacks", http://graphics.stanford.edu/~seander/bithacks.html.
uint32_t log2(uint32_t v);

template <typename T> void divide(T divisor, T& x){ x/=divisor; }

template <typename T, typename... Rest>
void divide(T divisor, T& x, Rest&... rest){
	divide(divisor,x);
	divide(divisor,rest...);
}

template<class T> T mapRange(T v, T ilo, T ihi, T olo, T ohi){
	auto f = (v - ilo) / (ihi - ilo); // to fraction in [0,1]
	return (ohi - olo)*f + olo; // lerp to output range
}

#define TEM template<class T>
TEM T pow2(const T& v){ return v*v; }				///< Returns value to the 2nd power
TEM T pow2S(const T& v){ return v*al::abs(v); }		///< Returns value to the 2nd power preserving sign
TEM T pow3 (const T& v){ return v*v*v; }			///< Returns value to the 3rd power
TEM T pow3Abs(const T& v){ return al::abs(pow3(v)); } ///< Returns absolute value to the 3rd power
TEM T pow4 (const T& v){ return pow2(pow2(v)); }	///< Returns value to the 4th power
TEM T pow5 (const T& v){ return v * pow4(v); }		///< Returns value to the 5th power
TEM T pow6 (const T& v){ return pow3(pow2(v)); }	///< Returns value to the 6th power
TEM T pow7 (const T& v){ return v * pow6(v); }		///< Returns value to the 7th power
TEM T pow8 (const T& v){ return pow4(pow2(v)); }	///< Returns value to the 8th power
TEM T pow9 (const T& v){ return v * pow8(v); }		///< Returns value to the 9th power
TEM T pow16(const T& v){ return pow4(pow4(v)); }	///< Returns value to the 16th power
TEM T pow17(const T& v){ return pow16(v)*v; }		///< Returns value to the 17th power
TEM T pow32(const T& v){ return pow4(pow8(v)); }	///< Returns value to the 32nd power
TEM T pow33(const T& v){ return pow32(v)*v; }		///< Returns value to the 33rd power
TEM T pow64(const T& v){ return pow8(pow8(v)); }	///< Returns value to the 64th power
TEM T pow65(const T& v){ return pow64(v)*v; }		///< Returns value to the 65th power
#undef TEM

/// Returns value to a positive integer power

/// \param[in] base		the base value to exponentiate
/// \param[in] power	the power to exponentiate by
template<class T>
T powN(T base, unsigned power){
	switch(power){
		case  0: return T(1);
		case  1: return base;
		case  2: return pow2(base);
		case  3: return pow3(base);
		case  4: return pow4(base);
		case  5: return pow5(base);
		case  6: return pow6(base);
		case  7: return pow7(base);
		case  8: return pow8(base);
		case  9: return pow9(base);
		case 16: return pow16(base);
		case 32: return pow32(base);
		case 64: return pow64(base);
		default:{
			T r = pow8(base)*pow2(base);
			for(unsigned i=10; i<power; ++i) r *= base;
			return r;
		}
	}
}

/// Sum of integers squared from 1 to n
template<class T> T sumOfSquares(T n){
	// 0^2 + 1^2 + 2^2 + ... + n^2 = n(n+1)(2n+1)/6
	static constexpr double c1_6 = 1./6.;
	static constexpr double c2_6 = 2./6.;
	return n*(n+T(1))*(c2_6*n+c1_6);
}

/// Truncates floating point value at decimal
template<class T> T trunc(const T& v);

/// Truncates floating point value to step
template<class T> T trunc(const T& v, const T& step);

/// Truncates floating point value to step. Faster version to avoid 1/step divide.
template<class T> T trunc(const T& v, const T& step, const T& recStep);

/// Returns value wrapped in [lo, hi)
template<class T> T wrap(const T& v, const T& hi=T(1), const T& lo=T(0));

/// Returns value wrapped in [lo, hi)

/// 'numWraps' reports how many wrappings occured where the sign, + or -,
/// signifies above 'hi' or below 'lo', respectively.
template<class T> T wrap(const T& v, long& numWraps, const T& hi=T(1), const T& lo=T(0));

/// Like wrap(), but only adds or subtracts 'hi' once from value
template<class T> T wrapOnce(const T& v, const T& hi=T(1));

template<class T> T wrapOnce(const T& v, const T& hi, const T& lo);

/// Returns value wrapped in [-pi, pi)
template<class T> T wrapPhase(const T& radians);

/// Like wrapPhase(), but only wraps once
template<class T> T wrapPhaseOnce(const T& radians);


//---- Intervals and Comparison

template <class UInt, class Int, class Float>
bool aeqBase(Float a, Float b, int maxULP){
	static_assert(sizeof(Float)==sizeof(Int), "Float/int size mismatch");
	static_assert(sizeof(UInt) ==sizeof(Int), "Int size mismatch");
	// Make sure maxULP is non-negative and small enough that the
	// default NAN won't compare as equal to anything.
	//assert(maxULP > 0 && maxULP < 4 * 1024 * 1024);
	union{ Float f; Int i; };
	f=a; auto ai = i;
	f=b; auto bi = i;
	static constexpr UInt msb = UInt(1)<<(sizeof(UInt)*8-1);
	// Make ai and bi lexicographically ordered as a twos-complement int
	if(ai < 0) ai = msb - ai;
	if(bi < 0) bi = msb - bi;
	return al::abs(ai - bi) <= maxULP;
}

/// Return whether two floats are almost equal

/// \param[in] a		first operand
/// \param[in] b		second operand
/// \param[in] maxULP	maximum "units in the last place"
///
/// Algorithm from Dawson, B. "Comparing floating point numbers",
/// http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
inline bool aeq(float a, float b, int maxULP=10){
	return aeqBase<uint32_t,int32_t>(a,b,maxULP);
}
inline bool aeq(double a, double b, int maxULP=10){
	return aeqBase<uint64_t,int64_t>(a,b,maxULP);
}

template <class T>
bool inSet(const T& val, const T& x){ return val==x; }

/// Returns whether val is equal to any of the successive arguments

/// Example: inSet(1, 4,3,1,47) returns true
/// Example: inSet(4.2, 3.1,5.7,10.9) returns false
/// Suitable for any type with == operator defined (including std::string).
template <class T, class... Ts>
bool inSet(const T& val, const T& x, Ts&&... xs){
	return inSet(val, x) || inSet<T>(val, std::forward<Ts>(xs)...);
}

/// Returns whether the absolute value is less than an epsilon
template<class T> bool lessAbs(const T& v, const T& eps=T(0.000001)){
	return al::abs(v) < eps;
}

/// Sort values so that a <= b
template<class T> void sort(T& a, T& b){
	if(a>b){ T t=a; a=b; b=t; }
}

/// Returns whether value is in interval [lo, hi]
template<class T> bool within(const T& v, const T& lo, const T& hi){
	return !((v < lo) || (v > hi));
}

/// Returns whether three values are in interval [lo, hi]
template<class T> bool within3(const T& a, const T& b, const T& c, const T& lo, const T& hi){
	return al::within(a,lo,hi) && al::within(b,lo,hi) && al::within(c,lo,hi);
}

/// Returns whether value is in interval [lo, hi)
template<class T> bool withinIE(const T& v, const T& lo, const T& hi){
	return (!(v < lo)) && (v < hi);
}


//---- Number Theory

/// Returns factorial. Argument must be less than or equal to 12.
inline uint32_t factorial(uint32_t n0to12){
	static const uint32_t factorial12u[13] = {
		1, 1, 2, 6, 24, 120, 720, 5040, 40320,
		362880, 3628800, 39916800, 479001600
	};
	return factorial12u[n0to12];
}

/// Returns square root of factorial
inline double factorialSqrt(int v){
	if(v<=1) return 1;
	double r=1;
	for(int i=2; i<=v; ++i) r *= std::sqrt(i);
	return r;
}

/// Return greatest common divisor of two arguments
template<class T> T gcd(const T& x, const T& y){
	if(y==T(0)) return al::abs(x);
	return al::gcd(y, al::remainder(x,y));
}

template <typename T, typename... Ts>
T gcd(const T& x, const Ts&... rest){ return gcd(x, gcd(rest...)); }

/// Returns least common multiple
template<class T> T lcm(const T& x, const T& y){ return (x*y)/al::gcd(x,y); }

/// Returns nearest integer division of one value to another
template<class T> T nearestDiv(T of, T to){ return to / al::round(to/of); }

/// Returns next largest value that is a multiple of 'm'
template<class T> T nextMultiple(T v, T m){
	auto q = (uint32_t)(v / m);
	return T(q + 1) * m;
}

/// Returns the number of digits in the integer portion
template<class T> T numInt(const T& v){ return al::floor(std::log10(v)) + 1; }

/// Returns (n+1)th prime number up to n=53
inline uint8_t prime(uint8_t n){
	static const uint8_t primes54[54] = {
	/*	  0    1    2    3    4    5    6    7    8    9   */
		  2,   3,   5,   7,  11,  13,  17,  19,  23,  29, // 0
		 31,  37,  41,  43,  47,  53,  59,  61,	 67,  71, // 1
		 73,  79,  83,  89,  97, 101, 103, 107, 109, 113, // 2
		127, 131, 137, 139, 149, 151, 157, 163, 167, 173, // 3
		179, 181, 191, 193, 197, 199, 211, 223, 227, 229, // 4
		233, 239, 241, 251								  // 5
	};
	return primes54[n];
}

/// Returns whether values have no common multiple
template <typename... Ts>
bool coprime(const Ts&... vals){ return gcd(vals...) == 1; }

/// Returns true if integer is prime
inline bool isPrime(unsigned n){
	if(n<=1) return false;
	else if(n<=3) return true;
	else if(n%2 == 0 || n%3 == 0) return false;
	uint64_t i = 5; // 64-bit to prevent overflow on squaring
	while(i*i <= n){
		if(n%i == 0 || n%(i+2) == 0) return false;
		i += 6;
	}
	return true;
}

/// Makes all arguments coprime (relatively prime)

/// \returns greatest common divisor (gcd) of output arguments with 1 meaning
/// the input arguments were already coprime.
template <typename... Args>
auto makeCoprime(Args&... args) -> decltype(al::gcd(args...)){
	auto gcdiv = gcd(args...);
	if(gcdiv>decltype(gcdiv)(1)) divide(gcdiv, args...);
	return gcdiv;
}


//---- Special Functions

/// Fast approximation to atan2()

// Author: Jim Shima, http://www.dspguru.com/comp.dsp/tricks/alg/fxdatan2.htm.
// |error| < 0.01 rad
template<class T> T atan2Fast(const T& y, const T& x){
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

/// The Gauss error function or probability integral
/// \see http://en.wikipedia.org/wiki/Error_function
template<class T> T erf(const T& x){
	const T xx = x*x;
	const T axx = T(0.147) * xx;
	return al::sgn(x)*std::sqrt(T(1) - std::exp(xx*(T(-4./M_PI) - axx)/(T(1)+axx)));
}

/// Returns e^(-v*v)
template<class T> T gaussian(const T& v){ return std::exp(-v*v); }

/// Gudermannian function

/// Relates circular and hyperbolic functions without using complex numbers.
/// \see http://en.wikipedia.org/wiki/Gudermannian_function
template<class T> T gudermannian(const T& x){
	return T(2) * std::atan(std::exp(x)) - T(M_PI_2);
}

/// Generalized Laguerre polynomial L{n,k}

/// \param[in] n	degree, a non-negative integer
/// \param[in] k	order
/// \param[in] x	position
/// http://en.wikipedia.org/wiki/Laguerre_polynomials
template<class T> T laguerreL(int n, int k, T x);

/// Associated Legendre polynomial
///
/// P_l^m(cos(t)) = (-1)^{l+m} / (2^l l!) sin^m(t) (d/d cos(t))^{l+m} sin^{2l}(t)
///
/// \param[in]	l	degree where l >= 0
/// \param[in]	m	order  where 0 <= m <= l
/// \param[in]	t	angle in [0, pi]
///
/// http://comp.cs.ehime-u.ac.jp/~ogata/nac/index.html
template<class T> T legendreP(int l, int m, T t);
template<class T> T legendreP(int l, int m, T ct, T st);

/// Unnormalized sinc function

/// \param[in] r	Value in radians
/// \param[in] eps	Small value to avoid division by zero
template<class T> T sinc(const T& r, const T& eps=T(0.0001)){
	return (al::abs(r) > eps) ? std::sin(r)/r : std::cos(r);
}


//---- Bit Operations

/// Returns number of bits set to 1

/// From "Bit Twiddling Hacks",
/// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
inline uint32_t bitsSet(uint32_t v){
	v = v - ((v >> 1) & 0x55555555);                    // reuse input as temporary
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333);     // temp
	return (((v + (v >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24; // count
}

/// Returns the next representable floating-point or integer value following x in the direction of y
template<class T> T nextAfter(const T& x, const T& y);

/// Returns number of trailing zeros in 32-bit int

/// This implements an algorithm from the paper
/// "Using de Bruijn Sequences to Index 1 in a Computer Word"
/// by Charles E. Leiserson, Harald Prokof, and Keith H. Randall.
uint32_t trailingZeroes(uint32_t v);



/// @} // end allocore group



// Implementation
//------------------------------------------------------------------------------

#define TEM template<class T>

namespace{
	template<class T> const T roundEps();
	template<> inline const float  roundEps<float >(){ return 0.499999925f; }
	template<> inline const double roundEps<double>(){ return 0.499999985; }

	inline uint8_t deBruijn(uint32_t v){
		static const uint8_t bitPos[32] = {
			 0, 1,28, 2,29,14,24, 3,30,22,20,15,25,17, 4, 8,
			31,27,13,23,21,19,16, 7,26,12,18, 6,11, 5,10, 9
		};
		return bitPos[(uint32_t(v * 0x077CB531UL)) >> 27];
	}
}

TEM inline T ceil(const T& v){ return al::round(v + roundEps<T>()); }
TEM inline T ceil(const T& v, const T& s){ return al::ceil(v/s)*s; }
TEM inline T ceil(const T& v, const T& s, const T& r){ return al::ceil(v*r)*s; }

TEM inline T floor(const T& v){ return al::round(v - roundEps<T>()); }
TEM inline T floor(const T& v, const T& s){ return al::floor(v/s)*s; }
TEM inline T floor(const T& v, const T& s, const T& r){ return al::floor(v*r)*s; }

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

inline uint32_t log2(uint32_t v){ return deBruijn(al::ceilPow2(v)); }

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

TEM inline T trunc(const T& v){ return al::round( (v > (T)0) ? v-roundEps<T>() : v+roundEps<T>() ); }
TEM inline T trunc(const T& v, const T& s){ return al::trunc(v/s)*s; }
TEM inline T trunc(const T& v, const T& s, const T& r){ return al::trunc(v*r)*s; }

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
		// Try one wrap first before resorting to more expensive operations
		R += diff;

		// If value was very slightly less than 'lo', then less significant
		// digits might get truncated by adding a larger number.
		if(R==hi) return al::nextAfter(R, lo);

		if(R < lo) R += diff * uint32_t(((lo - R)/diff) + 1);
		if(R==hi) return lo;
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

inline uint32_t trailingZeroes(uint32_t v){ return deBruijn(v & -v); }

#undef TEM
} // al::
#endif
