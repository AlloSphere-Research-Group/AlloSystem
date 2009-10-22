#ifndef AL_TIME_H_INC
#define AL_TIME_H_INC

/*
Timing and sleep functions with millisecond (Win32) or nanosecond (Unix)
resolution. Win32 requires linking to winmm.lib for multimedia timers.
*/

#ifdef WIN32
	#include <windows.h>
	//typedef __int64 al_nsec;		/**< nanoseconds type (accurate to +/- 292.5 years) */
#else
	#include <sys/time.h>
	#include <time.h>
	//typedef long long al_nsec;	/**< nanoseconds type (accurate to +/- 292.5 years) */
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef long long int al_nsec;				/**< nanoseconds type (accurate to +/- 292.5 years) */
typedef double al_sec;						/**< seconds type */

static void al_timing_init();				/**< Called once on application start (C only) */
static void al_timing_quit();				/**< Called once on application exit (C only) */
static al_nsec al_time();					/**< Get current time from OS */
static void al_sleep(al_nsec dt);			/**< Suspend calling thread's execution for dt nsec */
static void al_sleep_sec(al_sec dt);		/**< Suspend calling thread's execution for dt sec */
static al_nsec al_sleep_until(al_nsec t);	/**< Suspend calling thread's execution until absolute time, t. Returns ns slept. */
static al_sec al_nsec2sec(al_nsec nsec);	/**< Convert nsec to sec */
static al_nsec al_sec2nsec(al_sec sec);		/**< Convert sec to nsec */

#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus
namespace allo{

/// Timer with stopwatch-like functionality for benchmarking, etc.
class Timer {
public:
	Timer(): mStart(0), mStop(0){}

	al_nsec elapsed(){ return mStop - mStart; }					///< Returns nsec between start() and stop() calls
	al_sec elapsedSec(){ return al_nsec2sec(elapsed()); }		///< Returns  sec between start() and stop() calls
	al_sec elapsedMSec(){ return ((al_sec)elapsed()*1e-6); }	///< Returns msec between start() and stop() calls
	void start(){ mStart=al_time(); }						///< Set start time as current time
	void stop(){ mStop=al_time(); }							///< Set stop time as current time

private:
	al_nsec mStart, mStop;	// start and stop times
};

} // allo::
#endif



/* Implementation *************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

static inline double al_nsec2sec(al_nsec v){ return al_sec(v * 1e-9); }
static inline al_nsec al_sec2nsec(al_sec v){ return al_nsec(v * 1e9); }
static inline void al_sleep_sec(al_sec v){ al_sleep(al_sec2nsec(v)); }
static inline al_nsec al_sleep_until(al_nsec v){
	al_nsec now = al_time();
	if(v > now) al_sleep(v - now);
	return v - now;
}


/* platform specific */
#ifdef WIN32
	/*
	timeBeginPeriod() sets the granularity (in msec) of the timer for the 
	application that calls it. timeEndPeriod() should be called when the application
	exits with the same argument used for timeBeginPeriod().

	Note also that calling timeBeginPeriod() also affects the granularity of some
	other timing calls, such as CreateWaitableTimer() and WaitForSingleObject().
	*/

	static inline void al_timing_init(){ timeBeginPeriod(1); }
	static inline void al_timing_quit(){ timeEndPeriod(1); }

	static inline al_nsec al_time(){
		return (al_nsec)timeGetTime() * (al_nsec)1e6;
	}

	static inline void al_sleep(al_nsec v){
		Sleep((DWORD)(v / (al_nsec)1e6));
	}

#else

	static inline void al_timing_init(){}
	static inline void al_timing_quit(){}

	#define NS_S (al_nsec)1e9

	static inline al_nsec al_time(){
		timeval t;
		gettimeofday(&t, NULL);	
		return ((al_nsec)t.tv_sec) * NS_S + (al_nsec)(t.tv_usec * 1000);
	}

	static inline void al_sleep(al_nsec v){
		time_t sec = (time_t)(v / NS_S);
		timespec tspec = { sec, (long)(v - ((al_nsec)sec * NS_S)) }; // { sec, nsec }
		nanosleep(&tspec, NULL);
	}

	#undef NS_S
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus
namespace allo{
namespace{
struct TimeSingleton{
	TimeSingleton(){ al_timing_init(); }
	~TimeSingleton(){ al_timing_quit(); }
};
static TimeSingleton timeSingleton;
}
}
#endif

#endif

