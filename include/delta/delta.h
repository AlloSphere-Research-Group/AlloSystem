#ifndef INCLUDE_DELTA_H
#define INCLUDE_DELTA_H

/*
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

/*
	Audio/main thread scheduling
	
	Delta offers a basic set of scheduling primitives
	Audio is used as the principal clock source, due to its stability.
	The main thread is assumed to be 'ahead' of the audio thread by some ideal latency.
	
	It should be possible to drive Delta from any audio system, e.g. portaudio, VST, etc.
	The host audio driver must provide a repeated callback (from a single thread)	
*/

#include "system/al_time.h"
#include "types/al_pq.h"
#include "types/al_tube.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	Types
*/

/**! internal timestamps in terms of samples */
typedef long long int delta_samplestamp;

/**! 64-bit processing */
typedef double sample;

/* these will not remain macros */
#define SIGNAL_DIM (64)
#define AUDIO_INPUTS 2
#define AUDIO_OUTPUTS 2

///* Standard function signature for the handler of a message */
//typedef int (*al_msg_func)(al_sec t, char *);
//
///* Maximum memory footprint of a message */
//#define DELTA_MSG_ARGS_SIZE (52)
//struct al_msg {
//	al_sec t;
//	al_msg_func func;
//	char mem[DELTA_MSG_ARGS_SIZE];
//};
//typedef struct al_msg * msg;

/*
	delta_process header is itself a al_msg; suitable for insertion in a proclist:
*/
struct delta_process {
	struct al_msg msg;
	al_msg_func freefunc;
	struct delta_process * next;
	unsigned int id; /* assigned by whichever proclist contains it, zero otherwise */
};
typedef struct delta_process * process;

struct delta_proclist {
	process first, last; 
	unsigned int nextid;
};
typedef struct delta_proclist * proclist;

/*
	Bus
*/
struct delta_bus {
	struct delta_process proc;
	sample * data;
	sample doublebuffer[SIGNAL_DIM * 2];
	int front; /* 0 or 1 */
};
typedef struct delta_bus * bus;


/*
	Main singleton object
*/
typedef struct {
	/* flag true (1) between start & stop */
	int isRunning;					
	/* time since birth in samples */
	delta_samplestamp elapsed;	
	/* suggested latency */	
	al_sec latency;		
	
	/* main/audio thread scheduled events */
	pq mainpq, audiopq;
	/* messaging to and from audio thread */
	tube inbox, outbox;
	/* registered audio processes (visited per callback) */
	proclist procs;
	
	/* settings */
	double samplerate;
	
	/* audio IO busses */
	bus inputs[AUDIO_INPUTS];
	bus outputs[AUDIO_OUTPUTS];	
	
	/* future use? */
	void * userdata;	

} delta_main_t;
typedef delta_main_t * delta_main;


/*
	Methods
*/

/**! Library init/close */
extern void delta_main_init();
extern void delta_main_quit();

/**! Entry point from main thread; e.g. main loop */
extern void delta_main_tick();

/**! Current main-thread logical time */
extern al_sec delta_main_now();

/**! Entry point from audio thread; e.g. audio callback */
extern void delta_audio_tick(delta_samplestamp frames);

extern delta_main delta_main_get();

/* create/destroy a proclist */
extern proclist delta_proclist_create();
extern void delta_proclist_free(proclist * x);

/* insert/remove a process to a proclist */
extern void delta_proclist_append(proclist x, process p);
extern void delta_proclist_prepend(proclist x, process p);
extern void delta_proclist_remove(proclist x, process p);



/* bus definition */
extern int bus_proc(al_sec t, char * args);
extern int bus_free_msg(al_sec t, char * args);
extern bus bus_create();
extern sample * bus_read(bus self, process reader);

extern sample * delta_audio_output(int channel);

extern int delta_audio_proc_init(process x, al_msg_func procmsg, al_msg_func freemsg);
extern int delta_audio_proc_gc(process * ptr);

extern int bus_nofree_msg(al_sec t, char * args);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DELTA_H */
