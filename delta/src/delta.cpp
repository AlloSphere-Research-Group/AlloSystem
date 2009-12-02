#include "delta.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <list>
#include <vector>

#define ddebug(...) 
//#define ddebug(...) printf(__VA_ARGS__)

/*
	Main object
*/
struct delta_root {
	
	/* settings */
	delta_config config;
	
	/* registered sub-states */
	std::list<delta> states;
	
	/* audio IO busses */
	std::vector<bus> inputs;
	std::vector<bus> outputs;
	
	/* flag true (1) between start & stop */
	int isRunning;		
	int isAudioRunning;	
};
typedef struct delta_root * root;

static root g_root = NULL;

struct delta_state {
	/* time since birth in samples */
	samplestamp elapsed;	
	
	/* main/audio thread scheduled events */
	pq mainpq, audiopq;
	/* messaging to and from audio thread */
	tube inbox, outbox;
	/* registered audio processes (visited per callback) */
	proclist procs;
	
	/* future use? */
	void * userdata;	

};

void delta_init() {
	if (g_root == NULL) {
		g_root = new struct delta_root;
		g_root->config.inchannels = 0;
		g_root->config.outchannels = 0;
		delta_configure(delta_config_default());
	}
}

void delta_exit() {
	if (g_root != NULL) {
		
		for (int i=0; i<g_root->config.inchannels; i++) {
			bus_free(&g_root->inputs[i]);
		}
		
		for (int i=0; i<g_root->config.outchannels; i++) {
			bus_free(&g_root->outputs[i]);
		}
		
		std::list<delta>::iterator it = g_root->states.begin();
		while (it != g_root->states.end()) {
			delta D = *it;
			delta_close(&D);
			it = g_root->states.erase(it);
		}
	
		delete g_root;
		g_root = NULL;
	}
}


const delta_config delta_config_current() {
	delta_init();
	return g_root->config;
}
const delta_config delta_config_default() {
	const delta_config config = { 44100.0, 64, 2, 2, 0.03 };
	return config;
}
void delta_configure(delta_config config) {
	delta_init();
	
	/* only a multiple of 64 is safe */
	config.blocksize = config.blocksize - (config.blocksize % DELTA_SIGNAL_DIM);
	config.blocksize = config.blocksize < DELTA_SIGNAL_DIM ? DELTA_SIGNAL_DIM : config.blocksize;
	
	/* this shouldn't be called during audio execution!! */
	g_root->isRunning = 0;
	g_root->isAudioRunning = 0;
	g_root->config.latency = config.latency; 
	g_root->config.samplerate = config.samplerate;
	g_root->config.blocksize = config.blocksize;
	
	/* we only ever expand the channel count: */
	if (config.inchannels > g_root->config.inchannels)
		g_root->inputs.resize(config.inchannels);
	if (config.outchannels > g_root->config.outchannels)
		g_root->outputs.resize(config.outchannels);
	for (int i=g_root->config.inchannels; i<config.inchannels; i++) {
		g_root->inputs[i] = bus_create(NULL);
	}
	for (int i=g_root->config.outchannels; i<config.outchannels; i++) {
		g_root->outputs[i] = bus_create(NULL);
	}	
	g_root->config.inchannels = config.inchannels;
	g_root->config.outchannels = config.outchannels;
}


delta delta_create(void * userdata) {
	delta_init();
	
	delta D = new struct delta_state();
	if (D) {
		g_root->states.push_back(D);
		D->mainpq = al_pq_create(1000, 0);
		D->audiopq = al_pq_create(1000, 0);
		D->inbox = al_tube_create(10);
		D->outbox = al_tube_create(10);
		D->procs = delta_proclist_create();
		D->userdata = userdata;
	}
	return D;
}

void delta_close(delta * ptr) {
	delta D;
	if (ptr && *ptr) {
		D = *ptr;		
		g_root->states.remove(D);		
		/* TODO: flush all data */
		al_pq_free(&D->audiopq, 0);
		al_pq_free(&D->mainpq, 0);
		al_tube_free(&D->inbox);
		al_tube_free(&D->outbox);
		delta_proclist_free(&D->procs);		
		delete D;
		*ptr = NULL;
	}
}


void delta_audio_tick() {
	
	/*
		first pass:
	*/
	if (!g_root->isAudioRunning) {
		g_root->isAudioRunning = 1;
	}
	
	std::list<delta>::iterator it = g_root->states.begin();
	while (it != g_root->states.end()) {
		delta d = *it++;
	
		d->elapsed += g_root->config.blocksize;
		al_sec t = d->elapsed / g_root->config.samplerate;
	
		// copy incoming tube messages to the thread-local priority queue (ensures deterministic ordering)
		al_tube_pq_transfer(d->inbox, d->audiopq, t);
		al_pq_update(d->audiopq, t, 0);
		
		// trigger processing:
		process p = d->procs->first;
		while (p) {
			// TODO: handle return values (schedule unregister & free message?)
			p->proc.func((delta)d, p->proc.mem);
			p = p->next;
		}
	}
}

void delta_main_tick() {
	al_sec t;
	g_root->isRunning = 1;
	
	std::list<delta>::iterator it = g_root->states.begin();
	while (it != g_root->states.end()) {
		delta d = *it++;
		
		/* 
			figure out our desired target time
				- note that the main thread here *follows* the audio thread
			
			TODO: handle cases where actual latency is too big / too small
		*/
<<<<<<< .mine
		t = (d->elapsed / g_root->config.samplerate) + g_root->config.latency;
=======
		t = (D->elapsed / D->samplerate) + D->latency;
>>>>>>> .r978
		
		/* 
			handle any events to the current timestamp in the main priority queue: 
				(first, transfer in any incoming tube messages)
		*/
		al_tube_pq_transfer(d->outbox, d->mainpq, t);
		al_pq_update(d->mainpq, t, 0);
	}
}	

al_sec delta_main_now(delta D) {
	return D->mainpq->now;
}

al_sec delta_latency(delta D) {
	return g_root->config.latency;
}

void delta_set_latency(delta D, al_sec latency) {
	g_root->config.latency = latency;
}

samplestamp delta_audio_samplestamp(delta D) {
	return D->elapsed;
}

double delta_samplerate(delta D) {
	return g_root->config.samplerate;
}

samplestamp delta_blocksize(delta D) {
	return g_root->config.blocksize;
}

int delta_inchannels(delta D) {
	return g_root->config.inchannels;
}
int delta_outchannels(delta D) {
	return g_root->config.outchannels;
}

sample * delta_audio_output(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return g_root->outputs[channel]->data;
}

sample * delta_audio_input(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return g_root->inputs[channel]->data;
}

bus delta_audio_outbus(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return g_root->outputs[channel];
}
bus delta_audio_inbus(delta D, int channel) {
	/* TODO: bounds-check on channel? */
	return g_root->inputs[channel];
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