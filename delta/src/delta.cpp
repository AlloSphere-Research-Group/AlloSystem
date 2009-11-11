#include "delta.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define ddebug(...) 
//#define ddebug(...) printf(__VA_ARGS__)

/*
	Main object
*/
struct delta_state {
	/* flag true (1) between start & stop */
	int isRunning;		
	int isAudioRunning;	
	/* time since birth in samples */
	samplestamp elapsed;	
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
	samplestamp blocksize;
	int inchannels, outchannels;
	
	/* audio IO busses */
	bus * inputs;
	bus * outputs;	
	
	/* future use? */
	void * userdata;	

};

static delta g_main = NULL;


delta delta_main_init(double samplerate, al_sec latency) {
	int i;
	delta D = (delta_state *)calloc(1, sizeof(delta_state));
	D->isRunning = 0;
	D->isAudioRunning = 0;
	D->elapsed = 0;
	D->userdata = 0;
	
	D->mainpq = al_pq_create(1000, 0);
	D->audiopq = al_pq_create(1000, 0);
	D->inbox = al_tube_create(10);
	D->outbox = al_tube_create(10);
	D->procs = delta_proclist_create();
	
	D->latency = latency; 
	D->samplerate = samplerate;
	D->blocksize = DELTA_SIGNAL_DIM;
	D->inchannels = AUDIO_INPUTS;
	D->outchannels = AUDIO_OUTPUTS;
	
	D->inputs = (bus *)malloc(sizeof(bus) * D->inchannels);
	for (i=0; i<D->inchannels; i++) {
		D->inputs[i] = bus_create(D);
	}
	
	D->outputs = (bus *)malloc(sizeof(bus) * D->outchannels);
	for (i=0; i<D->outchannels; i++) {
		D->outputs[i] = bus_create(D);
	}
	
	if (g_main == NULL) {
		g_main = D;
	}
	return D;
}

delta delta_get() {
	return g_main;
}

void delta_main_quit(delta * ptr) {
	int i;
	delta D;
	if (ptr && *ptr) {
		D = *ptr;
		/* TODO: flush all data */
		al_pq_free(&D->audiopq, 0);
		al_pq_free(&D->mainpq, 0);
		al_tube_free(&D->inbox);
		al_tube_free(&D->outbox);
		delta_proclist_free(&D->procs);
		
		for (i=0; i<D->inchannels; i++) {
			bus_free(&D->inputs[i]);
		}
		for (i=0; i<D->outchannels; i++) {
			bus_free(&D->outputs[i]);
		}
		free(D->inputs);
		free(D->outputs);
		
		free(D);
		*ptr = NULL;
	}
}	

void delta_audio_tick(delta D) {
	//printf(".");
	
	/*
		first pass:
	*/
	if (!D->isAudioRunning) {
		D->isAudioRunning = 1;
	}
	
	D->elapsed += D->blocksize;
	al_sec t = D->elapsed / D->samplerate;
	
	// copy incoming tube messages to the thread-local priority queue (ensures deterministic ordering)
	al_tube_pq_transfer(D->inbox, D->audiopq, t);
	al_pq_update(D->audiopq, t, 0);
	
	// trigger processing:
	process p = D->procs->first;
	while (p) {
		// TODO: handle return values (schedule unregister & free message?)
		p->proc.func((delta)D, p->proc.mem);
		p = p->next;
	}
}

al_sec delta_main_tick(delta D) {
	int i;
	al_sec t;
	D->isRunning = 1;
	
	/* 
		run this tick a few times, 
			since audio thread usually recurrs more frequently than main
			(recalculate t a couple of times)
	*/
	static int loops = 2;
	for (i=0; i<loops; i++) 
	{
		/* 
			figure out our desired target time
				- note that the main thread here *follows* the audio thread
			
			TODO: handle cases where actual latency is too big / too small
		*/
		t = (D->elapsed / D->samplerate) + D->latency;
		
		/* 
			handle any events to the current timestamp in the main priority queue: 
				(first, transfer in any incoming tube messages)
		*/
		al_tube_pq_transfer(D->outbox, D->mainpq, t);
		al_pq_update(D->mainpq, t, 0);
	}
	
	return t;
}	

al_sec delta_main_now(delta D) {
	return D->mainpq->now;
}

al_sec delta_latency(delta D) {
	return D->latency;
}

void delta_set_latency(delta D, al_sec latency) {
	D->latency = latency;
}

samplestamp delta_audio_samplestamp(delta D) {
	return D->elapsed;
}

double delta_samplerate(delta D) {
	return D->samplerate;
}

samplestamp delta_blocksize(delta D) {
	return D->blocksize;
}

int delta_inchannels(delta D) {
	return D->inchannels;
}
int delta_outchannels(delta D) {
	return D->outchannels;
}

sample * delta_audio_output(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return D->outputs[channel]->data;
}

sample * delta_audio_input(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return D->inputs[channel]->data;
}

bus delta_audio_outbus(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return D->outputs[channel];
}
bus delta_audio_inbus(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return D->inputs[channel];
}

pq delta_main_pq(delta D) {
	return D->mainpq;
}

pq delta_audio_pq(delta D) {
	return D->audiopq;
}

tube delta_inbox(delta D) {
	return D->inbox;
}
tube delta_outbox(delta D) {
	return D->outbox;
}

typedef struct {
	delta D;
	process p;
} proc_msg_data;

static int delta_audio_proc_register(al_sec t, char * args) {
	proc_msg_data * data = (proc_msg_data *)args; 
	delta_proclist_append(data->D->procs, data->p);
	return 0;
}

static int delta_audio_proc_unregister(al_sec t, char * args) {
	proc_msg_data * data = (proc_msg_data *)args; 
	delta D = data->D;
	process x = data->p;
	delta_proclist_remove(D->procs, x);
	process * buf = (process *)al_tube_write_head_mem(D->outbox);
	if (buf) {
		*buf = x;
		al_tube_write_send(D->outbox, t, x->freefunc);
	} else {
		return 1;
	}
	return 0;
}

int delta_audio_proc_init(delta D, process x, delta_proc_func procmsg, al_msg_func freemsg) {
	proc_msg_data * buf = (proc_msg_data *)al_tube_write_head_mem(D->inbox);	
	*(process *)x->proc.mem = x;
	x->proc.func = procmsg;
	x->freefunc = freemsg;
	x->id = 0;
	if (buf) {
		buf->D = D;
		buf->p = x;
		al_tube_write_send(D->inbox, D->mainpq->now, delta_audio_proc_register);
	} else {
		return 1;
	}
	return 0;
}

int delta_audio_proc_gc(delta D, process * ptr) {
	process x;
	if (ptr && *ptr) {
		x = *ptr;
		process * buf = (process *)al_tube_write_head_mem(D->inbox);
		if (buf) {
			*buf = x;
			al_tube_write_send(D->inbox, D->mainpq->now, delta_audio_proc_unregister);
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