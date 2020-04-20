#ifndef INCLUDE_AL_MATH_RANDOM_HPP
#define INCLUDE_AL_MATH_RANDOM_HPP

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
	Various flavors of pseudo-random number generators and distributions

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

#include "allocore/system/al_Config.h" // uint32_t
#include <time.h> // time()
#include <cmath>

namespace al {

/// Random number generation utilities
namespace rnd{

class LinCon;
class MulLinCon;
class Tausworthe;
template<class RNG> class Random;


/// Get a random seed in interval [0, 4294967296)
inline static uint32_t seed(){
	static uint32_t val = time(NULL);
	return val = val*1664525UL + 1013904223UL;
}


/// Random distribution generator
///
/// @ingroup allocore
template <class RNG=al::rnd::Tausworthe>
class Random{
public:

	/// Default constructor uses a randomly generated seed
	Random(){}

	/// @param[in] seed		Initial seed value
	Random(uint32_t seed): mRNG(seed){}


	/// Set seed
	Random& seed(uint32_t v){ mRNG.seed(v); return *this; }

	/// Get underlying random number generator
	RNG& rng(){ return mRNG; }


	/// Returns uniform random in [0, 1)
	float uniform(){
		union{ uint32_t u; float f; };
		u = mRNG() >> 9 | 0x3F800000;
		return f - 1.f;
	}

	/// Returns uniform random in [0, hi)
	template <class T>
	T uniform(const T& hi){ return hi*uniform(); }

	/// Returns uniform random in [lo, hi)
	template <class T>
	T uniform(const T& hi, const T& lo){ return T((hi-lo)*uniform()) + lo; }

	/// Returns uniform random in [-1, 1)
	float uniformS(){
		union{ uint32_t u; float f; };
		u = mRNG() >> 9 | 0x40000000;
		return f - 3.f;
	}

	/// Returns uniform random in [-lim, lim)
	template <class T>
	T uniformS(const T& lim){ return lim*uniformS(); }

	/// Returns point within a unit ball

	/// To get a random point on a sphere, simply normalize the result.
	/// \tparam		N		dimensions of ball
	/// @param[in]	point	an array of size N
	template <int N, class T>
	void ball(T * point);

	/// Returns point within a unit ball
	template <template<int,class> class VecType, int N, class T>
	void ball(VecType<N,T>& point){ ball<N>(&point[0]); }

	/// Returns point within a unit ball
	template <class VecType>
	VecType ball(){ VecType v; ball(v); return v; }

	/// Returns point within a unit n-cube

	/// \tparam		N		dimensions of cube
	/// @param[in]	point	an array of size N
	template <int N, class T>
	void cube(T * point);

	/// Returns point within a unit n-cube
	template <template<int,class> class VecType, int N, class T>
	void cube(VecType<N,T>& point){ cube<N>(&point[0]); }

	/// Returns point within a unit n-cube
	template <class VecType>
	VecType cube(){ VecType v; cube(v); return v; }

	/// Returns standard normal variate
	float normal(){ float r; normal(r,r); return r; }

	/// Returns two standard normal variates (for the price of one)
	template <class T>
	void normal(T& y1, T& y2);

	/// Returns triangle distribution variate, in (-1,1)
	float triangle();

	/// Returns triangle distribution variate, in (-hi,hi)
	template <class T>
	T triangle(const T& hi){ return hi*triangle(); }

	/// Returns argument with sign randomly flipped
	float sign(float x=1.f);

	/// Returns true with a probability of 0.5
	bool prob(){ return mRNG()&0x80000000; }

	/// Returns true with a probability of p
	bool prob(float p){ return uniform() < p; }

	/// Randomly shuffles elements in array
	template <class T>
	void shuffle(T * arr, uint32_t len);


	// DEPRECATED:
	float gaussian(){ return normal(); }
	template <class T> void gaussian(T& y1, T& y2){ normal(y1,y2); }
protected:
	RNG mRNG;
};



/// Linear congruential uniform pseudo-random number generator.

///	This generator is very fast requiring only a single integer multiply and add
/// per iteration.  However, the least significant bits of the numbers are less
/// random; the most extreme case being the LSB which at best flips between
/// 0 and 1. This generator also exhibits poor dimensional distribution,
/// therefore it is best to have a different generator for each dimension,
/// rather than sharing one.
///
/// @ingroup allocore
class LinCon {
public:
	/// Default constructor uses a randomly generated seed
	LinCon(){
		seed(al::rnd::seed());
		type(0);
	}

	/// @param[in] seed		Initial seed value
	LinCon(uint32_t seed)
	:	mVal(seed)
	{	type(0); }


	/// Generate next uniform random integer in [0, 2^32)
	uint32_t operator()(){
		return mVal = mVal*mMul + mAdd;
	}

	/// Set seed
	void seed(uint32_t v){ mVal=v; }

	/// Change the type of equation used.

	/// 0 - Knuth, Numerical Recipes in C\n
	/// 1 - BCPL
	void type(int v){
		switch(v){
		default:
		case 0: mMul =    1664525; mAdd = 1013904223; break;
		case 1:	mMul = 2147001325; mAdd =  715136305; break;
		}
	}

private:
	uint32_t mVal;
	uint32_t mMul, mAdd;
};



/// Multiplicative linear congruential uniform pseudo-random number generator.

///	This generator is faster than LinCon requiring only a single integer
/// multiply per iteration. However, the downside is that it produces lower
/// quality (less "random") results than LinCon. Because of this, it is really
/// not appropriate for simulations, but due to its speed it is very useful for
/// synthesizing noise for audio and graphics.
///
/// @ingroup allocore
class MulLinCon{
public:
	/// Default constructor uses a randomly generated seed
	MulLinCon(){
		seed(al::rnd::seed());
		type(0);
	}

	/// @param[in] seed	Initial seed value
	MulLinCon(uint32_t seed)
	:	mVal(seed)
	{	type(0); }


	/// Generate next uniform random integer in [0, 2^32)
	uint32_t operator()(){
		return mVal *= mMul;
	}

	/// Set seed
	void seed(uint32_t v){ mVal=v; }

	/// Change the type of equation used.

	/// 0 - L'Ecuyer M8  (optimal generator for <=  8 dimensions)\n
	/// 1 - L'Ecuyer M16 (optimal generator for <= 16 dimensions)\n
	/// 2 - L'Ecuyer M32 (optimal generator for <= 32 dimensions)\n
	/// 3 - Marsaglia, Super-Duper\n
	void type(int v){
		switch(v){
		default:mMul = 2891336453UL; break; // L'Ecuyer M8
		case 1: mMul =   29943829UL; break; // L'Ecuyer M16
		case 2: mMul =   32310901UL; break; // L'Ecuyer M32
		case 3: mMul =      69069UL; break; // Super-duper
		}
	}

private:
	uint32_t mVal;
	uint32_t mMul;
};



/// Combined Tausworthe uniform pseudo-random number generator.

/// This generator produces highly random numbers, but is more expensive than
/// than a linear congruential RNG.
/// It is based on the paper
/// P. L'Ecuyer, "Maximally Equidistributed Combined Tausworthe Generators",
/// Mathematics of Computation, 65, 213 (1996), 203--213.
/// http://www.iro.umontreal.ca/~lecuyer/papers.html
///
/// @ingroup allocore
class Tausworthe{
public:

	/// Default constructor uses a randomly generated seed
	Tausworthe();

	/// @param[in] seed		Initial seed value
	Tausworthe(uint32_t seed);


	/// Generate next uniform random integer in [0, 2^32)
	uint32_t operator()();

	/// Set seed
	void seed(uint32_t v);

	/// Set seed
	void seed(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4);

private:
	uint32_t s1, s2, s3, s4;
	void iterate();
};


/// Get global random number generator
inline Random<>& global(){ static Random<> r; return r; }

/// Returns point within a unit ball

/// To get a random point on a sphere, simply normalize the result.
/// \tparam		N		dimensions of ball
/// @param[in]	point	an array of size N
template <int N, class T>
inline void ball(T * point){ global().ball<N>(point); }

/// Returns point within a unit ball
template <template<int,class> class VecType, int N, class T>
inline void ball(VecType<N,T>& point){ global().ball(point); }

/// Returns point within a unit ball
template <class VecType>
inline VecType ball(){ return global().ball<VecType>(); }

/// Returns point within a unit n-cube

/// \tparam		N		dimensions of cube
/// @param[in]	point	an array of size N
template <int N, class T>
inline void cube(T * point){ global().cube<N>(point); }

/// Returns point within a unit n-cube
template <template<int,class> class VecType, int N, class T>
inline void cube(VecType<N,T>& point){ global().cube(point); }

/// Returns point within a unit n-cube
template <class VecType>
inline VecType cube(){ return global().cube<VecType>(); }

/// Returns standard normal variate
inline float normal(){ return global().normal(); }
inline float gaussian(){ return normal(); }

/// Returns triangle distribution variate, in (-1,1)
inline float triangle(){ return global().triangle(); }

/// Returns true with probability 0.5
inline bool prob(){ return global().prob(); }

/// Returns true with probability p
inline bool prob(float p){ return global().prob(p); }

/// Returns argument with sign randomly flipped
inline float sign(float x=1.f){ return global().sign(x); }

/// Returns uniform random in [0, 1)
inline float uniform(){ return global().uniform(); }

/// Returns uniform random in [0, hi)
template <class T>
inline T uniform(const T& hi){ return global().uniform(hi); }

/// Returns uniform random in [lo, hi)
template <class T>
inline T uniform(const T& hi, const T& lo){ return global().uniform(hi,lo); }

/// Returns signed uniform random in (-1, 1)
inline float uniformS(){ return global().uniformS(); }

/// Returns signed uniform random in (-lim, lim)
template <class T>
inline T uniformS(const T& lim){ return global().uniformS(lim); }



// Implementation_______________________________________________________________

inline Tausworthe::Tausworthe(){ seed(al::rnd::seed()); }
inline Tausworthe::Tausworthe(uint32_t sd){ seed(sd); }

inline uint32_t Tausworthe::operator()(){
	iterate();
	return s1 ^ s2 ^ s3 ^ s4;
}

inline void Tausworthe::seed(uint32_t v){
	al::rnd::LinCon g(v);
	g();
	seed(g(), g(), g(), g());
}

inline void Tausworthe::seed(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4){
	//printf("%u %u %u %u\n", v1, v2, v3, v4);
	v1 & 0xffffffe ? s1 = v1 : s1 = ~v1;
	v2 & 0xffffff8 ? s2 = v2 : s2 = ~v2;
	v3 & 0xffffff0 ? s3 = v3 : s3 = ~v3;
	v4 & 0xfffff80 ? s4 = v4 : s4 = ~v4;
}

inline void Tausworthe::iterate(){
	s1 = ((s1 & 0xfffffffe) << 18) ^ (((s1 <<  6) ^ s1) >> 13);
	s2 = ((s2 & 0xfffffff8) <<  2) ^ (((s2 <<  2) ^ s2) >> 27);
	s3 = ((s3 & 0xfffffff0) <<  7) ^ (((s3 << 13) ^ s3) >> 21);
	s4 = ((s4 & 0xffffff80) << 13) ^ (((s4 <<  3) ^ s4) >> 12);
}


template <class RNG>
template <int N, class T>
void Random<RNG>::ball(T * point){
	T w;
	do{
		w = T(0);
		for(int i=0; i<N; ++i){
			float v = uniformS();
			point[i] = v;
			w += v*v;
		}
	} while(w >= T(1)); // if on or outside unit ball, try again
}

template <class RNG>
template <int N, class T>
inline void Random<RNG>::cube(T * point){
	for(int i=0; i<N; ++i) point[i] = uniform();
}

// Box-Muller transform
//		Box, G. and Muller, M. A note on the generation of normal deviates.
//		Ann. Math. Slat. 28, (1958).
//
// http://en.wikipedia.org/wiki/Box–Muller_transform
template <class RNG>
template <class T> void Random<RNG>::normal(T& y1, T& y2){
	float x1, x2, w;

	// Search for point within unit circle using sample-reject.
	// This will pass with probability π/4 = ~0.785.
	do{
		x1 = uniformS();
		x2 = uniformS();
		w = x1 * x1 + x2 * x2;
	} while(w >= 1.f);

	// perform inverse Gaussian function mapping
	w = std::sqrt((-2.f * std::log(w)) / w);
	y1 = T(x1 * w);
	y2 = T(x2 * w);
}

template <class RNG>
inline float Random<RNG>::triangle(){
	union {float f; uint32_t i;} u,v;
	u.i = 0x3f800000 | (mRNG()>>9); // float in [1,2)
	v.i = 0x3f800000 | (mRNG()>>9); // float in [1,2)
	return u.f - v.f;
}

template <class RNG>
inline float Random<RNG>::sign(float x){
	union {float f; uint32_t i;} u = {x};
	u.i |= mRNG() & 0x80000000;
	return u.f;
}

// Fisher-Yates shuffle
template <class RNG>
template <class T>
void Random<RNG>::shuffle(T * arr, uint32_t len){
	for(uint32_t i=len-1; i>0; --i){
		uint32_t j = uniform(i+1);
		T t = arr[i];
		arr[i] = arr[j];
		arr[j] = t;
	}
}

} // al::rnd::
} // al::
#endif

