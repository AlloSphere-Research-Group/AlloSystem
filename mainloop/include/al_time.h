#ifndef INCLUDE_AL_TIME_H
#define INCLUDE_AL_TIME_H

/*
 *  Type definitions and basic cross platform functions regarding time
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#if defined(WIN32) || defined(__WINDOWS_MM__)
	#define AL_WIN32
	#include <windows.h>
#elif defined( __APPLE__ ) && defined( __MACH__ )
	#define AL_OSX
	#include <Carbon/Carbon.h>
#else
	#define AL_LINUX
#endif

#include <limits.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! 
	Timing uses 64-bit unsigned ints for nanosecond units, doubles for second units. 
	8-byte signed integer nanoseconds allows us represent times accurately up to +/- 292.5 years.
*/
#ifdef AL_WIN32
	typedef __int64 al_nsec;		///< nanoseconds type
	typedef double al_sec;		///< seconds type
#else /* Posix (Mac/Linux) */
	typedef long long al_nsec;	///< nanoseconds type
	typedef double al_sec;		///< seconds type
#endif /* platform specific */

#ifdef __cplusplus 
namespace allo {
	typedef al_nsec nsec_t;
	typedef al_sec	sec_t;
}
#endif

/*! temporal limits */
#define AL_NEVER (ULLONG_MAX)
#define AL_ALMOST_NEVER (AL_NEVER-1)

/*! convert nanoseconds/seconds */
inline al_sec al_nsec2sec(al_nsec ns) { return ((al_sec)(ns)) * 1.0e-9; }
inline al_nsec al_sec2nsec(double s) { return (al_nsec) (s * 1.0e9); }

/*! get current system clock time */
extern al_nsec al_time_cpu();
#define al_now_cpu() (al_nsec2sec(al_time_cpu()))

/*! 
	sleep current thread (expressed in seconds, not nanoseconds, since exact amounts are not guaranteed) 
*/
extern void al_time_sleep(al_sec len);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_AL_TIME_H */
