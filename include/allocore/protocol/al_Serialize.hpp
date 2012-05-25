#ifndef INCLUDE_AL_SERIALIZE_HPP
#define INCLUDE_AL_SERIALIZE_HPP

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
	This is a C++ wrapper around the C serialization structs/functions.

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


#include "al_Serialize.h"
#include <vector>
#include <string>

namespace al{

namespace ser{

template<class T> inline uint32_t encode(char * b, const T * v, uint32_t n){ return 0; }


template<> inline uint32_t encode(char * b, const float * v, uint32_t n){ return serEncodeFloat32(b, v, n); }
template<> inline uint32_t encode(char * b, const double * v, uint32_t n){ return serEncodeFloat64(b, v, n); }
template<> inline uint32_t encode(char * b, const char * v, uint32_t n){ return serEncodeInt8(b, (const int8_t *)v, n); }
template<> inline uint32_t encode(char * b, const int8_t * v, uint32_t n){ return serEncodeInt8(b, v, n); }
template<> inline uint32_t encode(char * b, const int16_t * v, uint32_t n){ return serEncodeInt16(b, v, n); }
template<> inline uint32_t encode(char * b, const int32_t * v, uint32_t n){ return serEncodeInt32(b, v, n); }
template<> inline uint32_t encode(char * b, const int64_t * v, uint32_t n){ return serEncodeInt64(b, v, n); }
template<> inline uint32_t encode(char * b, const bool * v, uint32_t n){ return serEncodeUInt8(b, (const uint8_t *)v, n); }
template<> inline uint32_t encode(char * b, const uint8_t * v, uint32_t n){ return serEncodeUInt8(b, v, n); }
template<> inline uint32_t encode(char * b, const uint16_t * v, uint32_t n){ return serEncodeUInt16(b, v, n); }
template<> inline uint32_t encode(char * b, const uint32_t * v, uint32_t n){ return serEncodeUInt32(b, v, n); }
template<> inline uint32_t encode(char * b, const uint64_t * v, uint32_t n){ return serEncodeUInt64(b, v, n); }


template<class T> uint8_t getType();
template<> inline uint8_t getType<float   >(){ return 'f'; }
template<> inline uint8_t getType<double  >(){ return 'd'; }
template<> inline uint8_t getType<bool    >(){ return 't'; }
template<> inline uint8_t getType<uint8_t >(){ return 't'; }
template<> inline uint8_t getType<uint16_t>(){ return 'T'; }
template<> inline uint8_t getType<uint32_t>(){ return 'u'; }
template<> inline uint8_t getType<uint64_t>(){ return 'U'; }
template<> inline uint8_t getType<int8_t  >(){ return 'h'; }
template<> inline uint8_t getType<int16_t >(){ return 'H'; }
template<> inline uint8_t getType<int32_t >(){ return 'i'; }
template<> inline uint8_t getType<int64_t >(){ return 'I'; }

} // ser::


///
struct Serializer{

	template <class T>
	Serializer& operator<< (T v);

	Serializer& operator<< (const char * v);
	Serializer& operator<< (const std::string& v);
	
	template <class T>
	Serializer& add(const T * v, uint32_t num);

	const std::vector<char>& buf() const;

private:
	std::vector<char> mBuf, mTemp;
	
	template <class T> void checkSize(uint32_t n=1);
};


///
struct Deserializer{
	
	Deserializer(const std::vector<char>& b);
	
	Deserializer(const char * b, uint32_t n);
	
	template <class T>
	Deserializer& operator>> (T& v);
	Deserializer& operator>> (char * v);
	Deserializer& operator>> (std::string& v);

	const std::vector<char>& buf() const;

private:
	int mStart;
	std::vector<char> mBuf;
	char * bufDec();
};




// =============================================================================
// Implementation
// =============================================================================


template <class T> Serializer& Serializer::operator<< (T v){
	return add(&v, 1);
}

template <class T> Serializer& Serializer::add(const T * v, uint32_t num){
	checkSize<T>(num);
	uint32_t n = ser::encode(&mTemp[0], v, num);
	mBuf.insert(mBuf.end(), mTemp.begin(), mTemp.begin()+n);
	return *this;		
}

template <class T> void Serializer::checkSize(uint32_t n){
	uint32_t need = n*sizeof(T) + serHeaderSize();
	if(mTemp.size() < need) mTemp.resize(need);
}


template <class T> Deserializer& Deserializer::operator>> (T& v){
	uint32_t n = serDecode(bufDec(), &v);
	mStart += n;
	return *this;
}

//template <class T>
//Deserializer& decode(const T * v, uint32_t num){
//	uint32_t n = decode(bufDec(), v);
//	return *this;		
//}

} // al::


#endif
