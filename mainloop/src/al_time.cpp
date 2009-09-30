#include "al_main.h"

#ifdef AL_OSX

	#include <CoreAudio/HostTime.h>
	
	/*
		Rather than using a system call (gettimeofday()) we make use of CoreAudio, which is cheap & high resolution.
		A side-benefit of this is that it will correspond directly with the audio processing.
		Current time: AudioConvertHostTimeToNanos(AudioGetCurrentHostTime()); 
		CoreAudio also gives us a minimum resolution: AudioConvertHostTimeToNanos(AudioGetHostClockMinimumTimeDelta());
	*/
	al_nsec al_time_cpu() {
		return AudioConvertHostTimeToNanos(AudioGetCurrentHostTime());
	}

#endif

#ifdef AL_OSX || AL_LINUX
	
	#include <time.h>
	
	void al_sleep(al_sec len) {
		al_nsec nsec = al_sec2nsec(len);
		struct timespec req = { 0, 0 };
		req.tv_sec = (time_t)(len);
		req.tv_nsec = (nsec - (req.tv_sec * 1e9));
		
		while (nanosleep(&req, &req) == -1)
			continue;
	}
	
#endif