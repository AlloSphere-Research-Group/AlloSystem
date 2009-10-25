#include "delta.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define ddebug(...) 
//#define ddebug(...) printf(__VA_ARGS__)

static delta_main_t * g_main = NULL;


void delta_init() {
	int i;
	if (g_main == NULL) {
		g_main = (delta_main_t *)calloc(1, sizeof(delta_main_t));
		g_main->isRunning = 0;
		g_main->elapsed = 0;
		g_main->userdata = 0;
		
		g_main->mainpq = delta_pq_create(1000, 0);
		g_main->audiopq = delta_pq_create(1000, 0);
		g_main->inbox = delta_tube_create(10);
		g_main->outbox = delta_tube_create(10);
		g_main->procs = delta_proclist_create();
		
		g_main->latency = 0.03; 
		g_main->samplerate = 44100.0;
		
		for (i=0; i<AUDIO_INPUTS; i++) {
			g_main->inputs[i] = bus_create();
		}
		for (i=0; i<AUDIO_OUTPUTS; i++) {
			g_main->outputs[i] = bus_create();
		}
		
	}
}

delta_main delta_main_get() {
	delta_init();
	return g_main;
}

void delta_quit() {
	if (g_main) {
		/* TODO: flush all data */
		delta_pq_free(&g_main->audiopq);
		delta_pq_free(&g_main->mainpq);
		delta_tube_free(&g_main->inbox);
		delta_tube_free(&g_main->outbox);
		delta_proclist_free(&g_main->procs);
		free(g_main);
		g_main = NULL;
	}
}	

void delta_audio_tick(delta_samplestamp frames) {
	printf(".");
	
	g_main->elapsed += frames;
	delta_sec t = g_main->elapsed / g_main->samplerate;
	
	// copy incoming tube messages to the thread-local priority queue (ensures deterministic ordering)
	delta_tube_pq_transfer(g_main->inbox, g_main->audiopq, t);
	delta_pq_update(g_main->audiopq, t, 0);
	
	// trigger processing:
	process p = g_main->procs->first;
	while (p) {
		// TODO: handle return values (schedule unregister & free message?)
		p->msg.func(t, p->msg.mem);
		p = p->next;
	}
}

void delta_main_tick(delta_sec cputime) {
	printf(">");
	
	/* 
		figure out our desired target time
	*/
	delta_sec t = g_main->elapsed / g_main->samplerate;
	t -= g_main->latency;
	
	/* 
		handle any events to the current timestamp in the main priority queue: 
			(first, transfer in any incoming tube messages)
	*/
	delta_tube_pq_transfer(g_main->outbox, g_main->mainpq, t);
	delta_pq_update(g_main->mainpq, t, 0);
}	

sample * delta_audio_output(int channel) {
	/* TODO: bounds-check on channel? */
	return g_main->outputs[channel]->data;
}



static int delta_audio_proc_register(delta_sec t, char * args) {
	delta_proclist_append(g_main->procs, *(process *)args);
	return 0;
}

static int delta_audio_proc_unregister(delta_sec t, char * args) {
	process x = *(process *)args;
	delta_proclist_remove(g_main->procs, x);
	process * buf = (process *)delta_tube_write_head_mem(g_main->outbox);
	if (buf) {
		*buf = x;
		delta_tube_write_send(g_main->outbox, t, x->freefunc);
	} else {
		return 1;
	}
	return 0;
}

int delta_audio_proc_init(process x, delta_msg_func procmsg, delta_msg_func freemsg) {
	process * buf = (process *)delta_tube_write_head_mem(g_main->inbox);	
	*(process *)x->msg.mem = x;
	x->msg.func = procmsg;
	x->freefunc = freemsg;
	x->id = 0;
	if (buf) {
		*buf = x;
		delta_tube_write_send(g_main->inbox, g_main->mainpq->now, delta_audio_proc_register);
	} else {
		return 1;
	}
	return 0;
}

int delta_audio_proc_gc(process * ptr) {
	process x;
	if (ptr && *ptr) {
		x = *ptr;
		process * buf = (process *)delta_tube_write_head_mem(g_main->inbox);
		if (buf) {
			*buf = x;
			delta_tube_write_send(g_main->inbox, g_main->mainpq->now, delta_audio_proc_unregister);
			return 0;
		} else {
			/* TODO: reschedule */
			printf("error: couldn't unregister process\n");
			return 1;
		}
		*ptr = NULL;
	}
	return 0;
}


/* return the singleton main pq */
pq delta_pq_main() {
	return g_main ? g_main->mainpq : NULL;
}