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


#include <string>
#include <sstream>		// string conversion
#include "allocore/system/al_Config.h"

namespace al{

#ifndef UINT32_C
#define UINT32_C(v) v ## UL
#endif

#ifndef UINT64_C
#define UINT64_C(v) v ## ULL
#endif

/// Convert decimal integer to ascii base-36 character
char base10To36(int dec10);

/// Convert ascii base-36 character to decimal integer
int base36To10(char ascii36);

/// Convert a string of 1s and 0s to an integer.

/// @param[in] strBin	binary string where the first character is the most-significant digit
///
uint32_t bitsToUInt(const char * strBin);

/// Returns temporary copy-constructed object

/// This function can be more convenient than calling a copy constructor since
/// one does not need to explicitly write the type of the object.
template <class T>
T clone(const T& obj){ return T(obj); }

/// Returns 1 if little endian, 0 if big endian
int endian();

/// Swap the bytes of a word in-place
template <typename T>
void swapBytes(T& word);

/// Swap the bytes of the words in an array in-place
template <typename T>
void swapBytes(T * data, unsigned count);

/// Returns string with all characters converted to lowercase
std::string toLower(const std::string& s);

/// Returns string with all characters converted to uppercase
std::string toUpper(const std::string& s);

/// Convert formatted arguments to a string
std::string toString(const char * fmt, ...);

/// Convert numerical type to a string
template <class T> std::string toString(const T& v);

/// Convert array of numerical types to a comma separated string
template <class T>
std::string toStringArray(const T * v, int num, int stride);

/// Convert string to a typed value
template <class T>
T fromString(const std::string& v);



//------------------------------------------------------------------------------
// Implementation
inline int endian(){
	static int x=1;
	return *(char *)&x;
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
void swapBytes(T * data, unsigned count){
	for(unsigned i=0; i<count; ++i) swapBytes(data[i]);
}

template <class T>
std::string toStringArray(const T * v, int n, int s){
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

template <class T>
T fromString(const std::string& v){
	T res = T();
	std::stringstream ss(v);
	ss >> res;
	return res;
}

} // al::

#endif
