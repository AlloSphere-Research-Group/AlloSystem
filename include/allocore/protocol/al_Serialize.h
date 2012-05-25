#ifndef INCLUDE_AL_SERIALIZE_H
#define INCLUDE_AL_SERIALIZE_H

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
	General serialization and deserialization for file and network i/o.

	Either single elements or arrays of elements can be read or written to a
	buffer. Each group of elements is preceeded by a header that tells the type of
	elements and the size of the array.

	A string is a C-style null-terminated array of characters.
	A boolean is a single byte where a value of 0 is false and non-zero value is true.

	Byte ordering is little endian. If the target architecture is big endian,
	then pass in the preprocessor flag -DSER_IS_BIG_ENDIAN.

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


#include <stddef.h>
#include <string.h>
#include "allocore/system/al_Config.h"


#ifdef __cplusplus
extern "C" {
#endif

//#define SER_IS_BIG_ENDIAN

/* Serialization data types */
enum{
	SER_FLOAT32	= 'f',	/* 32-bit IEEE float */
	SER_FLOAT64	= 'd',	/* 64-bit IEEE float */
	SER_INT8	= 'h',	/*  8-bit 2's-complement signed integer */
	SER_INT16	= 'H',	/* 16-bit 2's-complement signed integer */
	SER_INT32	= 'i',	/* 32-bit 2's-complement signed integer */
	SER_INT64	= 'I',	/* 64-bit 2's-complement signed integer */
	SER_UINT8	= 't',	/*  8-bit unsigned integer */
	SER_UINT16	= 'T',	/* 16-bit unsigned integer */
	SER_UINT32	= 'u',	/* 32-bit unsigned integer */
	SER_UINT64	= 'U',	/* 64-bit unsigned integer */
	SER_SUB		= '_'	/* substructure */
};


#define SER_HEADER_SIZE 5

/* Serialized data header */
struct SerHeader{
	uint8_t type;		/* type of data */
	uint32_t num;		/* number of data elements */
};


/* Serialized data header. Don't think this is needed... */
//struct SerStruct{
//	SerHeader header;
//	void * data;
//};


/* Copy 1-byte elements */
static uint32_t serCopy1(void * dst, const void * src, uint32_t num);

/* Copy 2-byte elements in little endian byte order */
static uint32_t serCopy2(void * dst, const void * src, uint32_t num);

/* Copy 4-byte elements in little endian byte order */
static uint32_t serCopy4(void * dst, const void * src, uint32_t num);

/* Copy 8-byte elements in little endian byte order */
static uint32_t serCopy8(void * dst, const void * src, uint32_t num);

/* Decode serialized data. Returns number of bytes parsed */
uint32_t serDecode(const char * b, void * data);

/*  */
struct SerHeader serGetHeader(const char * buf);

/* Returns header size in bytes */
static inline int serHeaderSize(){ return SER_HEADER_SIZE; }

/* Returns size of data type in bytes */
int serTypeSize(uint8_t t);

uint32_t serElementsSize(const struct SerHeader * h);

static void serSwap(char * a, char * b);
static void serSwapBytes2(void * v);
static void serSwapBytes4(void * v);
static void serSwapBytes8(void * v);

/* Returns human-readable string of header */
const char * serStringifyHeader(const struct SerHeader * h);

/* Returns human-readable string of type */
const char * serStringifyType(uint8_t t);



	





#define SOH serHeaderSize()

static inline void serHeaderWrite(char * b, uint8_t type, uint32_t num){
	b[0] = type;
	serCopy4(b+1, &num, 1);
}

#define DO(B, N)\
	serHeaderWrite(b, type, N);\
	return SOH + serCopy##B(b+SOH, v, N)
static inline uint32_t serEncode1(uint8_t type, char * b, const void * v, uint32_t n){ DO(1,n); }
static inline uint32_t serEncode2(uint8_t type, char * b, const void * v, uint32_t n){ DO(2,n); }
static inline uint32_t serEncode4(uint8_t type, char * b, const void * v, uint32_t n){ DO(4,n); }
static inline uint32_t serEncode8(uint8_t type, char * b, const void * v, uint32_t n){ DO(8,n); }
#undef DO

static inline uint32_t serEncodeFloat32(char * b, const float * v   , uint32_t n){ return serEncode4(SER_FLOAT32, b,v,n); }
static inline uint32_t serEncodeFloat64(char * b, const double * v  , uint32_t n){ return serEncode8(SER_FLOAT64, b,v,n); }
static inline uint32_t serEncodeInt8   (char * b, const int8_t * v  , uint32_t n){ return serEncode1(SER_INT8   , b,v,n); }
static inline uint32_t serEncodeInt16  (char * b, const int16_t * v , uint32_t n){ return serEncode2(SER_INT16  , b,v,n); }
static inline uint32_t serEncodeInt32  (char * b, const int32_t * v , uint32_t n){ return serEncode4(SER_INT32  , b,v,n); }
static inline uint32_t serEncodeInt64  (char * b, const int64_t * v , uint32_t n){ return serEncode8(SER_INT64  , b,v,n); }
static inline uint32_t serEncodeUInt8  (char * b, const uint8_t * v , uint32_t n){ return serEncode1(SER_UINT8  , b,v,n); }
static inline uint32_t serEncodeUInt16 (char * b, const uint16_t * v, uint32_t n){ return serEncode2(SER_UINT16 , b,v,n); }
static inline uint32_t serEncodeUInt32 (char * b, const uint32_t * v, uint32_t n){ return serEncode4(SER_UINT32 , b,v,n); }
static inline uint32_t serEncodeUInt64 (char * b, const uint64_t * v, uint32_t n){ return serEncode8(SER_UINT64 , b,v,n); }




static inline void serSwap(char * a, char * b){ char t=*a; *a=*b; *b=t; }

static inline void serSwapBytes2(void * v){	
	char * b = (char *)v;
	serSwap(b  , b+1);
}

static inline void serSwapBytes4(void * v){	
	char * b = (char *)v;
	serSwap(b  , b+3);
	serSwap(b+1, b+2);
}

static inline void serSwapBytes8(void * v){	
	char * b = (char *)v;
	serSwap(b  , b+7);
	serSwap(b+1, b+6);
	serSwap(b+2, b+5);
	serSwap(b+3, b+4);
}


static inline uint32_t serCopy1(void * d, const void * s, uint32_t n){
	memcpy(d,s,n); return n;
}

#define DEF_LE(B, S)\
static inline uint32_t serCopy##B(void * d, const void * s, uint32_t n){\
	n = n<<S;\
	memcpy(d,s,n);\
	return n;\
}

#define DEF_BE(p, B, S)\
static inline uint32_t serCopy##B(void * d, const void * s, uint32_t n){\
	n = n<<S;\
	memcpy(d,s,n);\
	char * t = (char *)d;\
	for(uint32_t i=0; i<n; i+=B) swapBytes##B(t+i);\
	return n;\
}

#ifdef SER_IS_BIG_ENDIAN
DEF_BE(2,1) DEF_BE(4,2) DEF_BE(8,3)
#else
DEF_LE(2,1) DEF_LE(4,2) DEF_LE(8,3)
#endif

#undef DEF_BE
#undef DEF_LE

#undef SOH

#ifdef __cplusplus
}
#endif

#endif
