#ifndef INCLUDE_AL_RUNLOOP_H
#define INCLUDE_AL_RUNLOOP_H 1

#include "system/al_config.h"
#include "system/al_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	A secondary thread run loop
		(manages thread firing & sleeping)
	
	Looks quite similar to al_mainloop...
*/

typedef struct al_runloop * runloop;
typedef void (*al_runloop_tick_handler)(al_nsec time, void * userdata);
typedef void (*al_runloop_quit_handler)(void * userdata);

/* called from main thread (or any thread?) */
AL_API runloop al_runloop_create(al_sec interval, al_runloop_tick_handler tickhandler, void * userdata, al_runloop_quit_handler quithandler);

/* called from within the runloop thread */
AL_API void al_runloop_terminate_self(runloop rl);

/* called from outside the runloop thread */
AL_API void al_runloop_terminate(runloop rl);

#ifdef __cplusplus
}
#endif

#endif /* include guard */