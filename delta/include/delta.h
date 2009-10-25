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

#ifdef __cplusplus
extern "C" {
#endif

/*
	Types
*/

/**! public timestamps in terms of seconds */
typedef double delta_sec;

/**! internal timestamps in terms of samples */
typedef long long int delta_samplestamp;

/**! 64-bit processing */
typedef double sample;

/* these will not remain macros */
#define SIGNAL_DIM (64)
#define AUDIO_INPUTS 2
#define AUDIO_OUTPUTS 2

/* Maximum memory footprint of a message */
#define DELTA_MSG_ARGS_SIZE (52)

/* Standard function signature for the handler of a message */
typedef int (*delta_msg_func)(delta_sec t, char *);

struct delta_msg {
	delta_sec t;
	delta_msg_func func;
	char mem[DELTA_MSG_ARGS_SIZE];
};
typedef struct delta_msg * msg;

/*
	A priority queue
		random access insertion, sorted access retrieval/removal
		for single-threaded application
		sorts on insertion, with fast-path for first/last positions
*/
/* A message in the queue */
struct delta_pq_msg {
	struct delta_msg msg;
	struct delta_pq_msg * next;
};
typedef struct delta_pq_msg * pq_msg;

struct delta_pq {
	delta_sec now; /* scheduler logical time */
	pq_msg head, tail, pool;
	int len;
	delta_sec retry_period; /* if a message call fails */
};
typedef struct delta_pq * pq;

/*
	delta_process header is itself a delta_msg; suitable for insertion in a proclist:
*/
struct delta_process {
	struct delta_msg msg;
	delta_msg_func freefunc;
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
	Single-writer / single-reader wait-free fifo for sending messages between a pair of threads
	
	|||||||||||||||||||||||||||||
				rw 
	 send packets (thread 1)
				r......w
	 receive packets (thread 2)
				 .....rw
	|||||||||||||||||||||||||||||
*/
typedef struct delta_tube * tube;

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
	delta_sec latency;		
	
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
extern void delta_init();
extern void delta_quit();

/**! Entry point from main thread; e.g. main loop */
extern void delta_main_tick(delta_sec cputime);

/**! Entry point from audio thread; e.g. audio callback */
extern void delta_audio_tick(delta_samplestamp frames);

extern delta_main delta_main_get();

/* return the singleton main pq */
extern pq delta_pq_main();

/* allocate a new priority queue */
extern pq delta_pq_create(int size, delta_sec birth);
/* free a priority queue */
extern void delta_pq_free(pq * ptr);

extern int delta_pq_used(pq x);

/* grab memory for the next message */
extern char * delta_pq_msg(pq x);
/* schedule it */
extern void delta_pq_sched(pq x, delta_sec t, delta_msg_func f);

/* remove any scheduled messages with *mem == ptr (matching addresses) */
extern void delta_pq_cancel_ptr(pq x, void * ptr);

/* read the top message */
extern pq_msg delta_pq_top(pq x);
/* pop the top message, return the next */
extern pq_msg delta_pq_pop(pq x);

/* alternative API: */
/* convenience wrapper to call all scheduled functions up to a given timestamp */
extern void delta_pq_update(pq x, delta_sec until, int defer);
extern void delta_pq_advance(pq x, delta_sec step, int defer);

/* create/destroy a proclist */
extern proclist delta_proclist_create();
extern void delta_proclist_free(proclist * x);

/* insert/remove a process to a proclist */
extern void delta_proclist_append(proclist x, process p);
extern void delta_proclist_prepend(proclist x, process p);
extern void delta_proclist_remove(proclist x, process p);

/* create tube with 2^N message slots */
extern tube delta_tube_create(int nbits);

/* free a tube */
extern void delta_tube_free(tube * x);

/* return number of scheduled messages in the tube */
extern int delta_tube_used(tube x);

/* get pointer next sendable message */
extern msg delta_tube_write_head(tube x);
/* after filling the message, send it */
extern void delta_tube_write_send_msg(tube x);

/* alternative API: */
/* get pointer to data of next sendable message */
extern char * delta_tube_write_head_mem(tube x);
/* after filling the message data, assign the function to call, and send it */
extern void delta_tube_write_send(tube x, delta_sec t, delta_msg_func f);

/* get pointer to memory of next receivable message */
extern msg delta_tube_reader_head(tube x);
/* after handling the message, return it */
extern void delta_tube_reader_done(tube x);

/* alternative API: */
/* convenience wrapper to call all scheduled functions up to a given timestamp */
extern void delta_tube_reader_advance(tube x, delta_sec until);

/* convenience wrapper to shunt message from an incoming tube to a local priority queue */
extern void delta_tube_pq_transfer(tube x, pq q, delta_sec until);

/* bus definition */
extern int bus_proc(delta_sec t, char * args);
extern int bus_free_msg(delta_sec t, char * args);
extern bus bus_create();
extern sample * bus_read(bus self, process reader);

extern sample * delta_audio_output(int channel);

extern int delta_audio_proc_init(process x, delta_msg_func procmsg, delta_msg_func freemsg);
extern int delta_audio_proc_gc(process * ptr);

extern int bus_nofree_msg(delta_sec t, char * args);


#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_DELTA_H */
