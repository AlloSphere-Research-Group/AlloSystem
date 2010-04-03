#include "system/al_Time.h"
#include "apr_time.h"

/*
APR API documentation
--------------------------------------------------------------------------------
typedef apr_int64_t apr_time_t
number of microseconds since 00:00:00 january 1, 1970 UTC
--------------------------------------------------------------------------------
void apr_sleep	(	apr_interval_time_t 	t	 ) 	
Sleep for the specified number of micro-seconds.

Parameters:
t 	desired amount of time to sleep.
Warning:
May sleep for longer than the specified time.
--------------------------------------------------------------------------------
*/


al_sec al_time() {
	const apr_time_t t = apr_time_now();	// microseconds
	return ((al_sec)t) * 1.0e-6;			// 1 s / 1e6 us
}

al_nsec al_time_nsec() {
	const apr_time_t t = apr_time_now();	// microseconds
	return ((al_nsec)t) * 1e3;				// 1000 ns / 1 us
}

void al_sleep(al_sec v) {
	apr_interval_time_t t = (apr_interval_time_t)(v * 1.0e6);
	apr_sleep(t);
}

void al_sleep_nsec(al_nsec v) {
	apr_interval_time_t t = v * 1e-3;
	apr_sleep(t);
}

void al_sleep_until(al_sec target) {
	al_sec dt = target - al_time();
	if (dt > 0) al_sleep(dt);
}
