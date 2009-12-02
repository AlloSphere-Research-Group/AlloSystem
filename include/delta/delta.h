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
		(TODO: provide option to choose between audio clock and cpu clock)
	The main thread is assumed to be 'ahead' of the audio thread by some ideal latency.
	
	It should be possible to drive Delta from any audio system, e.g. portaudio, VST, etc.
	
	Usage:
	
	Initalize with delta_main_init()
	
	The host audio driver (single thread) must trigger delta_audio_tick() repeatedly
	
	The host (main thread) must also trigger delta_main_tick() repeatedly
	
	Once the audio driver triggers have ended (e.g. audio thread joined), 
		release resources with delta_main_quit()
*/

/*
	Dependencies
*/
#include "system/al_time.h"
#include "types/al_pq.h"
#include "types/al_tube.h"

#ifdef __cplusplus
extern "C" {
#endif

/* these will not remain macros */
#define AUDIO_INPUTS 2
#define AUDIO_OUTPUTS 2
#define DELTA_SIGNAL_DIM (64)

/*
	Types
*/

/*
	An opaque struct for a main/audio scheduler
*/
typedef struct delta_state * delta;

/**! internal timestamps in terms of samples */
typedef long long int samplestamp;

/**! 64-bit processing */
typedef double sample;

/*
	A struct used to get/set global configuration:
*/
typedef struct {
	double samplerate;
	samplestamp blocksize;		/* should be a power of 2 */
	unsigned int inchannels, outchannels;
	al_sec latency;
} delta_config;

/* 
	Standard function signature for an audio process 
*/
typedef samplestamp (*delta_proc_func)(delta D, char *);

/* Maximum memory footprint of a message */
#define DELTA_PROC_ARGS_SIZE (52)
struct delta_proc {
	samplestamp t;
	delta_proc_func func;
	char mem[DELTA_PROC_ARGS_SIZE];
};
typedef struct delta_proc * proc;



/*
	delta_process header is itself a al_msg; suitable for insertion in a proclist:
*/
struct delta_process {
	struct delta_proc proc;
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
	sample doublebuffer[DELTA_SIGNAL_DIM * 2];
	int front; /* 0 or 1 */
};
typedef struct delta_bus * bus;




/*
	init/close
*/
extern void delta_init();
extern void delta_exit();

/* 
	retrieve current and default audio configurations
*/
extern const delta_config delta_config_current();
extern const delta_config delta_config_default();
/*
	Warning: this call is not safe to make while audio processing is occuring!
*/
extern void delta_configure(delta_config config);

extern delta delta_create(void * userdata);
extern void delta_close(delta * D);

/**! 
	Entry point from main thread; e.g. main loop
	 
*/
extern void delta_main_tick();

/**! 
	Entry point from audio thread; e.g. audio callback 
	Assumes IO frame increment equal to delta_blocksize(); 
		(you need to ringbuffer etc. if not)
*/
extern void delta_audio_tick();

/**! Current main-thread logical time */
extern al_sec delta_main_now(delta D);

/**! 
	Get/Set the main-thread latency 
		Main thread logical time will attempt to run ahead of audio thread by this amount
		It should approximate the period at which delta_main_tick() is called
*/
extern al_sec delta_latency(delta D);
extern void delta_set_latency(delta D, al_sec latency);

/**! Current audio-thread elapsed time */
extern samplestamp delta_audio_samplestamp(delta D);

/**! Global accessors: */
extern double delta_samplerate(delta D);
extern samplestamp delta_blocksize(delta D);
extern int delta_inchannels(delta D);
extern int delta_outchannels(delta D);



/**!
	Messaging
		inbox is for sending messages from main to audio thread
		outbox is for sending messages from audio to main thread
*/
extern pq delta_main_pq(delta D);
extern pq delta_audio_pq(delta D);
extern tube delta_inbox(delta D);
extern tube delta_outbox(delta D);


/**!
	Audio IO
*/
extern bus delta_audio_outbus(delta D, int channel);
extern bus delta_audio_inbus(delta D, int channel);
extern sample * delta_audio_output(delta D, int channel);
extern sample * delta_audio_input(delta D, int channel);



/*
	Methods
*/


/* create/destroy a proclist */
extern proclist delta_proclist_create();
extern void delta_proclist_free(proclist * x);

/* insert/remove a process to a proclist */
extern void delta_proclist_append(proclist x, process p);
extern void delta_proclist_prepend(proclist x, process p);
extern void delta_proclist_remove(proclist x, process p);


/* internal use */
extern int delta_audio_proc_init(delta D, process x, delta_proc_func procmsg, al_msg_func freemsg);
extern int delta_audio_proc_gc(delta D, process * ptr);

/* bus definition */
extern bus bus_create(delta D);
extern void bus_free(bus * x);
extern sample * bus_read(bus self, process reader);

extern samplestamp bus_proc(delta D, char * args);
extern int bus_free_msg(al_sec t, char * args);
extern int bus_nofree_msg(al_sec t, char * args);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DELTA_H */
