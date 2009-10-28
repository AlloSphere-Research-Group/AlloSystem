#include "types/al_tube.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "math.h"

/* wait-free single-reader single-writer fifo */
struct al_tube {
	unsigned int writer, reader;
	unsigned int size, wrap;
	msg packets;	
	//int blocked; /* prevent new messages */
};

/* create tube with 2^N message slots */
tube al_tube_create(int nbits) {
	tube x = (tube)malloc(sizeof(struct al_tube));
	if (x) {
		x->reader = 0;
		x->writer = 0;
		x->size = 1<<nbits;
		x->wrap = x->size-1;
		x->packets = (msg)calloc(x->size, sizeof(struct al_msg));
		//x->blocked = 0;
		printf("tube using %i bytes\n", sizeof(struct al_tube) + sizeof(struct al_msg) * x->size);
	}
	return x;
}

/* free a tube. 
todo: handle the thread safety ness of this?
*/
void al_tube_free(tube * ptr) {
	tube x;
	if (ptr && *ptr) {
		x = *ptr;
		//x->blocked = 1;
		/* the logic here twists my brain */
		//al_tube_reader_advance(x, HUGE_VALF);
		free(x->packets);
		free(x);
		*ptr = 0;
	}
}

/* return number of scheduled messages in the tube */
int al_tube_used(tube x) {
	return (x->reader > x->writer) ? x->size - (x->reader - x->writer) : x->writer - x->reader;
}

/* get pointer next sendable message */
msg al_tube_write_head(tube x) {
	/* ensure that the next writer is not the same as the reader */
	if (((x->writer + 1) & x->wrap) == x->reader) { // || x->blocked) {
		return NULL;
	} else {
		return x->packets + x->writer;
	}
}

/* get pointer to memory of next sendable message */
char * al_tube_write_head_mem(tube x) {
	/* ensure that the next writer is not the same as the reader */
	if (((x->writer + 1) & x->wrap) == x->reader) { // || x->blocked) {
		return NULL;
	} else {
		return x->packets[x->writer].mem;
	}
}	

/* after filling the message, send it */
void al_tube_write_send_msg(tube x) {
	x->writer = (x->writer + 1) & x->wrap;
}

/* after filling the message data, assign the function to call, and send it */
void al_tube_write_send(tube x, al_sec t, al_msg_func f) {
	msg m = x->packets + x->writer;
	m->func = f;
	m->t = t;
	x->writer = (x->writer + 1) & x->wrap;
}

/* get pointer to memory of next receivable message */
msg al_tube_reader_head(tube x) {
	if (x->writer == x->reader) {
		return NULL;
	} else {
		return x->packets + x->reader;
	}
}

/* after handling the message, return it */
void al_tube_reader_done(tube x) {
	x->reader = (x->reader + 1) & x->wrap;
}

/* convenience wrapper to call all scheduled functions up to a given timestamp */
void al_tube_reader_advance(tube x, al_sec until) {
	msg m = al_tube_reader_head(x);
	while (m && m->t <= until) {
		m->func(m->t, m->mem);
		al_tube_reader_done(x);
		m = al_tube_reader_head(x);
	}
}

extern void al_tube_pq_transfer(tube x, pq q, al_sec until) {
	msg m = al_tube_reader_head(x);
	char * buf;
	
	while (m && m->t <= until) {
		// copy to pq:
		buf = al_pq_msg(q);
		if (buf) {
			memcpy(buf, m->mem, AL_PQ_MSG_ARGS_SIZE);
			al_pq_sched(q, m->t, m->func);
		} else {
			printf("pq empty\n"); /* retry? */
		}
		al_tube_reader_done(x);
		m = al_tube_reader_head(x);
	}
}