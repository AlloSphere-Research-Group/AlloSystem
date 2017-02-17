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

#ifdef AL_WINDOWS

	#ifdef AL_EXPORTS
		#define AL_API __declspec(dllexport)
	#else
		#define AL_API __declspec(dllimport)
	#endif

#else
	#define AL_API extern
#endif

#define AL_SNPRINTF snprintf
#define AL_VSNPRINTF vsnprintf

#ifdef __MINGW32__
	#undef AL_SNPRINTF
	#define AL_SNPRINTF _snprintf
	#undef AL_VSNPRINTF
	#define AL_VSNPRINTF _vsnprintf
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

#endif /* INCLUDE_AL_SYSTEM_CONFIG_H */
