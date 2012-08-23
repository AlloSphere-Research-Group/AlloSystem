#ifndef INCLUDE_AL_SYSTEM_CONFIG_H
#define INCLUDE_AL_SYSTEM_CONFIG_H 1

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
	General OS-dependent configurations

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#define AL_SYSTEM_LIB_VERSION 0.01

#if defined(WIN32) || defined(__WINDOWS_MM__) || defined(WIN64)
	#define AL_WINDOWS 1
	#define WIN32_LEAN_AND_MEAN
	#define VC_EXTRALEAN
	#include <windows.h>

	// undefine macros of common words
	#ifdef DELETE
	#undef DELETE
	#endif
	#ifdef max
	#undef max
	#endif
	#ifdef min
	#undef min
	#endif
	// windef.h defines these for backwards compatability with 16-bit compilers
	#ifdef near
	#undef near
	#endif
	#ifdef far
	#undef far
	#endif
	
	#ifdef AL_EXPORTS
		#define AL_API __declspec(dllexport)
	#else
		#define AL_API __declspec(dllimport)
	#endif

#elif defined( __APPLE__ ) && defined( __MACH__ )
	#define AL_OSX 1
	#define AL_API extern

#else
	#define AL_LINUX 1
	#define AL_API extern
#endif

/* 
	primitive typedefs
*/
#ifdef AL_WINDOWS
	#include <stdint.h>
	#define AL_PRINTF_LL "I64"
#else
	#include "allocore/system/pstdint.h"
	#define AL_PRINTF_LL "ll"
#endif

	
typedef long long int al_nsec;				/**< nanoseconds type (accurate to +/- 292.5 years) */
typedef double al_sec;						/**< seconds type */

#define AL_STRINGIFY(...) #__VA_ARGS__
#define AL_DEBUGLN printf("In %s: line %d\n", __FILE__, __LINE__);

#ifndef AL_MIN
	#define AL_MIN(A,B)	({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
#endif

#ifndef AL_MAX
	#define AL_MAX(A,B)	({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })
#endif

#endif /* INCLUDE_AL_SYSTEM_CONFIG_H */
