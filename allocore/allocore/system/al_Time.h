#ifndef INCLUDE_AL_TIME_H
#define INCLUDE_AL_TIME_H

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
	Timing and sleep functions with millisecond (Win32) or nanosecond (Unix)
	resolution. Win32 requires linking to winmm.lib for multimedia timers.
	
	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/system/al_Config.h"
#include <limits.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/**! temporal limits */
#define AL_TIME_NSEC_NEVER (ULLONG_MAX)

#ifdef AL_WINDOWS
/**! print format for al_nsec */
#define AL_NSEC_FMT "I64d"
#else
/**! print format for al_nsec */
#define AL_NSEC_FMT "lld"
#endif

/**! conversion factors for nanoseconds/seconds */
#define al_time_ns2s		1.0e-9
#define al_time_s2ns		1.0e9

/**! Get current time from OS */
extern al_sec al_time();					
extern al_nsec al_time_nsec();				

/**! Suspend calling thread's execution for dt sec/nsec */
extern void al_sleep(al_sec dt);		
extern void al_sleep_nsec(al_nsec dt);	

/**! convenience function to sleep until a target wall-clock time */
extern void al_sleep_until(al_sec target);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INCLUDE_AL_TIME_H */

