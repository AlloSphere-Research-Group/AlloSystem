#include "al_time.h"

/* Windows */
#ifdef AL_WIN32
	#include <windows.h>
	
	al_nsec al_time_cpu() {
		return al_sec2nsec(timeGetTime());
	}
	
	void al_time_sleep(al_sec len) {
		Sleep((DWORD)(al_nsec2sec(len));
	}
	
/* Posix (Mac, Linux) */
#else
	#include <sys/time.h>
	#include <time.h>
	
	al_nsec al_time_cpu() {
		timeval t;
		gettimeofday(&t, NULL);	
		return al_sec2nsec(t.tv_sec) + (al_nsec)(t.tv_usec * 1000);
	}
	
	void al_time_sleep(al_sec len) {
		time_t sec = (time_t)len;
		al_nsec nsec = al_sec2nsec(len - (al_sec)sec);
		timespec tspec = { sec, nsec }; 
		while (nanosleep(&tspec, &tspec) == -1)
			continue;
	}
	
#endif /* platform specific */
