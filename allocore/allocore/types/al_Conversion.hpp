#ifndef INCLUDE_AL_CONVERSION_HPP
#define INCLUDE_AL_CONVERSION_HPP

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
	This is a grab bag of routines for converting between built-in types

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


#include <stdio.h>
#include <string.h>
#include <iostream>
#include <limits.h>
#include <sstream>		/* string conversion */
#include "allocore/system/al_Config.h"

namespace al{

#ifndef UINT32_C
#define UINT32_C(v) v ## UL
#endif

#ifndef UINT64_C
#define UINT64_C(v) v ## ULL
#endif

#define CONST_(N, vf, vd)\
	template <class T> struct N;\
	template<> struct N< float>{ operator uint32_t() const { return UINT32_C(vf); } };\
	template<> struct N<double>{ operator uint64_t() const { return UINT64_C(vd); } };

	CONST_(MaskExpo, 0x7F800000, 0x7FF0000000000000) // IEEE-754 floating-point exponent bit mask
	CONST_(MaskFrac, 0x007FFFFF, 0x000FFFFFFFFFFFFF) // IEEE-754 floating-point fraction bit mask
	CONST_(MaskSign, 0x80000000, 0x8000000000000000) // IEEE-754 floating-point sign bit mask
	CONST_(Expo1   , 0x3F800000, 0x3FF0000000000000) // IEEE-754 floating-point [1-2) exponent interval
#undef CONST_


/// Union for twiddling bits of floats
template<class T> struct Twiddle;

template<> struct Twiddle<float>{
	Twiddle(const float& v): f(v){}
	Twiddle(const uint32_t& v): u(v){}
	Twiddle(const int32_t& v): i(v){}
	union{ int32_t i; uint32_t u; float f; };
};

template<> struct Twiddle<double>{
	Twiddle(const double& v): f(v){}
	Twiddle(const uint64_t& v): u(v){}
	Twiddle(const int64_t& v): i(v){}
	union{ int64_t i; uint64_t u; double f; };
};



/// Convert decimal integer to ascii base-36 character
char base10To36(int dec10);

/// Convert ascii base-36 character to decimal integer
int base36To10(char ascii36);

/// Convert a string of 1s and 0s to an integer.

/// @param[in] strBin	binary string where the first character is the most-significant digit
///
uint32_t bitsToUInt(const char * strBin);

/// Returns zero if argument is subnormal, otherwise returns argument
float blockSubnormal(float v);

/// Returns zero if argument is subnormal, otherwise returns argument
double blockSubnormal(double v);

/// Returns temporary copy-constructed object

/// This function can be more convenient than calling a copy constructor since
/// one does not need to explicitly write the type of the object.
template <class T>
T clone(const T& obj){ return T(obj); }

/// Returns 1 if little endian, 0 if big endian
int endian();

/// Returns biased decimal value of 32-bit float exponent field.

/// The true exponent is the return value minus 127.
/// For example, values in [0.5, 1) return 126 (01111110), so the true
///	exponent is 126 - 127 = -1.
uint32_t floatExponent(float v);

/// Returns mantissa field as float between [0, 1).
float floatMantissa(float v);

/// Converts linear integer phase to fraction

///	2^bits is the effective size of the lookup table. \n
///	Note: the fraction only has 24-bits of precision.
float fraction(uint32_t bits, uint32_t phase);

/// Convert 16-bit signed integer to floating point in [-1, 1)
float intToUnit(int16_t v);

/// Type-pun 32-bit unsigned int to 32-bit float

/// This function uses a union to avoid problems with direct pointer casting
/// when the fstrict-aliasing compiler flag is on.
inline float punUF(uint32_t v){ Twiddle<float> u(v); return u.f; }

/// Type-pun 32-bit float to 32-bit unsigned int

/// This function uses a union to avoid problems with direct pointer casting
/// when the fstrict-aliasing compiler flag is on.
inline uint32_t punFU( float v){ Twiddle< float> u(v); return u.u; }

/// Type-pun 32-bit float to 32-bit signed int
inline  int32_t punFI( float v){ Twiddle< float> u(v); return u.i; }

/// Type-pun 64-bit float to 64-bit unsigned int
inline uint64_t punFU(  double v){ Twiddle<double> u(v); return u.u; }

/// Type-pun 64-bit float to 64-bit signed int
inline  int64_t punFI(  double v){ Twiddle<double> u(v); return u.i; }

/// Type-pun 64-bit unsigned int to 64-bit float
inline   double punUF(uint64_t v){ Twiddle<double> u(v); return u.f; }

/// Type-pun 64-bit signed int to 64-bit float
inline   double punIF( int64_t v){ Twiddle<double> u(v); return u.f; }

/// Swap the bytes of a word in-place
template <typename T>
void swapBytes(T& word);

/// Swap the bytes of the words in an array in-place
template <typename T>
void swapBytes(T * data, unsigned count);

/// Convert argument to a string using snprintf
template <class T> std::string toString(const char * fmt, const T& v);

/// Convert numerical type to a string
template <class T> std::string toString(const T& v);

/// Convert array of numerical types to a comma separated string
template <class T>
std::string toString(const T * v, int num, int stride=1);

/// Convert 32-bit unsigned integer to unit float in [0, 1)
template<class T> T uintToUnit (uint32_t v);

/// Convert 32-bit unsigned integer to unit float in [-1, 1)
template<class T> T uintToUnitS(uint32_t v);

/// Convert float in [-1, 1) to 16-bit signed int in [0, 2^16)
int16_t unitToInt16(float v);

/// Convert float in [0, 1) to 32-bit unsigned int in [0, 2^32)

/// This conversion is most accurate on a linear scale.
/// Input values outside [0, 1) result in undefined behavior.
uint32_t unitToUInt(float u);

/// Convert float in [0, 1) to 32-bit unsigned int in [0, 2^32)

/// This conversion is most accurate on an exponential scale.
///	Input values outside [-1, 1) return 0.
///	Values in [-1, 0] behave as positive values in [0, 1).
uint32_t unitToUInt2(float u);

/// Convert float in [0, 1) to 8-bit unsigned int in [0, 256)
uint8_t unitToUInt8(float u);




// Implementation
//------------------------------------------------------------------------------

inline char base10To36(int v){
	static const char * c = "0123456789abcdefghijklmnopqrstuvwxyz";
	if(v>=0 && v<=35) return c[v];
	return '0';
}

inline int base36To10(char v){
	v = tolower(v);
	if(v>='0' && v<='9') return v - '0';
	if(v>='a' && v<='z') return v - 'a' + 10;
	return 0;	// non-alphanumeric
}

inline uint32_t bitsToUInt(const char * string){
	uint32_t v=0; int n = strlen(string);
	for(int i=0; i<n; ++i) if(string[i] == '1') v |= 1<<(n-1-i);
	return v;
}

// alternate version...
//inline uint32_t bitsToUInt(const char * bits){
//	uint32_t i=0, r=0;
//	for(; bits[i] && i<32; ++i) r |= ((bits[i]=='1'?1:0) << (31-i));
//	return r>>(32-i);
//}

/// Sets argument to zero if subnormal
inline float blockSubnormal(float v){
	const uint32_t i = punFU(v);
	const uint32_t frac = i & MaskFrac<float>();
	const uint32_t expo = i & MaskExpo<float>();
	if(expo == 0 && frac != 0) v = 0.f;
	return v;
}

/// Sets argument to zero if subnormal
inline double blockSubnormal(double v){
	const uint64_t i = punFU(v);
	const uint64_t frac = i & MaskFrac<double>();
	const uint64_t expo = i & MaskExpo<double>();
	if(expo == 0 && frac != 0) v = 0.;
	return v;
}

inline int endian(){
	static int x=1;
	return *(char *)&x;
}

inline uint32_t floatExponent(float v){
	return punFU(v) >> 23 & 0xff;
}

inline float floatMantissa(float v){
	uint32_t frac = punFU(v);
	frac = (frac & MaskFrac<float>()) | Expo1<float>();
	return punUF(frac) - 1.f;
}

inline float fraction(uint32_t bits, uint32_t phase){
	phase = phase << bits >> 9 | Expo1<float>();
	return punUF(phase) - 1.f;
}

inline float intToUnit(int16_t v){
	uint32_t vu = (((uint32_t)v) + 0x808000) << 7; // set fraction in float [2, 4)
	return punUF(vu) - 3.f;
}

// This is used by swapBytes
template<int NumBytes> void swapBytesN(void * word);

template<>
inline void swapBytesN<1>(void * word){}

template<>
inline void swapBytesN<2>(void * word){
	uint16_t& v = *(uint16_t *)word;
	v = (v >> 8) | (v << 8);
}

template<>
inline void swapBytesN<3>(void * word){
	uint8_t * v = (uint8_t *)word;
	v[0] ^= v[2];
	v[2] ^= v[0];
	v[0] ^= v[2];
}

template<>
inline void swapBytesN<4>(void * word){
	uint32_t& v = *(uint32_t *)word;
	v	= ((v >> 24))
		| ((v >>  8) & 0x0000ff00UL)
		| ((v <<  8) & 0x00ff0000UL)
		| ((v << 24));
}

template<>
inline void swapBytesN<8>(void * word){
	uint64_t& v = *(uint64_t *)word;
	v	= ((v >> 56))
		| ((v >> 40) & 0x000000000000ff00ULL)
		| ((v >> 24) & 0x0000000000ff0000ULL)
		| ((v >>  8) & 0x00000000ff000000ULL)
		| ((v <<  8) & 0x000000ff00000000ULL)
		| ((v << 24) & 0x0000ff0000000000ULL)
		| ((v << 40) & 0x00ff000000000000ULL)
		| ((v << 56));
}

template<typename T>
inline void swapBytes(T& v){ swapBytesN<sizeof(v)>(&v); }

template<class T>
inline void swapBytes(T * data, unsigned count){
	for(unsigned i=0; i<count; ++i) swapBytes(data[i]);
}

template <class T>
std::string toString(const char * fmt, const T& v){
	char buf[32];
	AL_SNPRINTF(buf, sizeof(buf), fmt, v);
	return std::string(buf);
}

template <class T>
std::string toString(const T * v, int n, int s){
	std::string r;
	for(int i=0; i<n; ++i){
		r += toString(v[i*s]);
		if(i<(n-1)) r += ", ";
	}
	return r;
}

template <class T>
std::string toString(const T& v){
	using namespace std;
	stringstream ss(stringstream::in | stringstream::out);
	ss << v;
	string r;
	ss >> r;
	return r;
}

template<> inline float uintToUnit<float>(uint32_t v){
	v = v >> 9 | Expo1<float>();	// float in [1, 2)
	return punUF(v) - 1.f;
}

template<> inline float uintToUnitS<float>(uint32_t v){
	v = v >> 9 | 0x40000000;		// float in [2, 4)
	return punUF(v) - 3.f;
}

inline int16_t unitToInt16(float v){
	float r = v + 3.f; // put in [2,4)
	return int16_t((al::punFU(r) >> 7) + (1<<15));
}

inline uint32_t unitToUInt(float v){
	++v;	// go into [1,2] range, FP fraction is now result
	return punFU(v) << 9;
}

// TODO: make 64-bit ready
inline uint32_t unitToUInt2(float v){
	uint32_t normalU = punFU(v);
	uint32_t rbs = 126UL - (normalU >> 23UL);
//	printf("%x %lu\n", (normalU | 0x800000) << 8, rbs);
//	printf("%x\n", 0x80000000UL >> rbs);
	return (((normalU | 0x800000UL) << 8UL) & (~ULONG_MAX | 0xffffffffUL)) >> rbs;

//	uint32_t normalU = punFU(v);
//	uint32_t rbs = 118UL - ((normalU >> 23UL) & (~ULONG_MAX | 0x7fffffUL));
////	printf("%x %lu\n", (normalU | 0x800000) << 8, rbs);
////	printf("%x\n", 0x80000000UL >> rbs);
//	return ((normalU & (~ULONG_MAX | 0xffffffUL)) | 0x800000UL) >> rbs;
////	return (((normalU | 0x800000UL) << 8UL) & (~ULONG_MAX | 0xffffffffUL)) >> rbs;

//Her00
//float y = v + 1.f;
//return ((unsigned long&)v) & 0x7FFFFF;      // last 23 bits
}

inline uint8_t unitToUInt8(float u){
	++u;
	return (punFU(u) >> 15) & MaskFrac<float>();
}

} // al::

#endif
