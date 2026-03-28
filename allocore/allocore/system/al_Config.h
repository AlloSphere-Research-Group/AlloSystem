#ifndef INC_AL_SYSTEM_CONFIG_H
#define INC_AL_SYSTEM_CONFIG_H

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	General OS-dependent configurations

	Author(s):
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

#if (defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(__WINDOWS_MM__))
	#if !defined(AL_WINDOWS)
		#define AL_WINDOWS 1
	#endif
#elif (defined(__APPLE__) && defined(__MACH__))
	#if !defined(AL_OSX)
		#define AL_OSX 1
	#endif
#elif (defined(__EMSCRIPTEN__))
	#if !defined(AL_EMSCRIPTEN)
		#define AL_EMSCRIPTEN 1
	#endif
#elif !defined(AL_LINUX)
	#define AL_LINUX 1
#endif

#ifdef AL_WINDOWS
	#ifdef AL_EXPORTS
		#define AL_API __declspec(dllexport)
	#else
		#define AL_API __declspec(dllimport)
	#endif
#else
	#define AL_API extern
#endif

#ifdef __MINGW32__
	#define AL_SNPRINTF _snprintf
	#define AL_VSNPRINTF _vsnprintf
#else
	#define AL_SNPRINTF snprintf
	#define AL_VSNPRINTF vsnprintf
#endif

/*
	primitive typedefs
*/
#if !defined(AL_WINDOWS) || defined(__MSYS__) || defined(__MINGW64__)
	#include "allocore/system/pstdint.h"
	#define AL_PRINTF_LL "ll"
#else
	#include <stdint.h>
	#define AL_PRINTF_LL "I64"
#endif

typedef long long int al_nsec;				/**< nanoseconds type (accurate to +/- 292.5 years) */
typedef double al_sec;						/**< seconds type */

#define AL_STRINGIFY(...) #__VA_ARGS__
#define AL_DEBUGLN printf("In %s: line %d\n", __FILE__, __LINE__);

#endif
