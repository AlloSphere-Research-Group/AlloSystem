#ifndef INC_AL_TIME_H
#define INC_AL_TIME_H

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Timing and sleep functions with millisecond (Win32) or nanosecond (Unix)
	resolution. Win32 requires linking to winmm.lib for multimedia timers.

	Author(s):
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

/**! Get the real (calendar) time since Jan. 1, 1970 UTC */
extern al_sec al_system_time();
extern al_nsec al_system_time_nsec();

/**! Get monotonic time for calculating time intervals */
extern al_sec al_steady_time();
extern al_nsec al_steady_time_nsec();

/**! Suspend calling thread's execution for dt sec/nsec */
extern void al_sleep(al_sec dt);
extern void al_sleep_nsec(al_nsec dt);

/**! Convenience function to sleep until a target wall-clock time */
extern void al_sleep_until(al_sec target);

/**! \deprecated use al_system_time */
extern al_sec al_time();
extern al_nsec al_time_nsec();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
