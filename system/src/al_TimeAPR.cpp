#include "system/al_Time.h"
#include "apr_time.h"


al_sec al_time() {
	const apr_time_t t = apr_time_now(); // microseconds
	return ((al_sec)t) * 1.0e-6; 
}

al_nsec al_time_nsec() {
	const apr_time_t t = apr_time_now(); // microseconds
	return ((al_nsec)t) * 1e3; 
}

void al_sleep(al_sec v) {
	apr_interval_time_t t = (apr_interval_time_t)(v * 1.0e6);
	apr_sleep(t);
}

void al_sleep_nsec(al_nsec v) {
	apr_interval_time_t t = v * 1e3;
	apr_sleep(t);
}

void al_sleep_until(al_sec target) {
	al_sec dt = target - al_time();
	if (dt > 0) al_sleep(dt);
}
