#include "types/al_pq.h"
#include "types/al_tube.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "math.h"

#define ddebug(...) 
//#define ddebug(...) printf(__VA_ARGS__)

struct al_msg {
	al_sec t;
	al_msg_func func;
	char mem[AL_PQ_MSG_ARGS_SIZE];
	// size_t size;
};

/*
	A priority queue
		random access insertion, sorted access retrieval/removal
		for single-threaded application
		sorts on insertion, with fast-path for first/last positions
*/
/* A message in the queue */
struct al_pq_msg {
	struct al_msg msg;
	struct al_pq_msg * next;
	al_sec retry;	// retry period, or 0 for non-deferable messages
};

struct al_pq {
	al_sec now; /* scheduler logical time */
	pq_msg head, tail, pool;
	int len;
	al_sec retry_period; /* if a message call fails */
};

/* 
	Alternative experimental implementation of pq
	
	I would just use stl, but 
		1. the order of equal-valued priorities is not determinate
		2. random removal is not possible
	
	What happens if the queue pool has no more memory?
		Add extra memory?
	Why not just use stl::vector or apr_pool_t for the msg pool, so it can grow as needed?
		An apr_pool_t will work well, since we can safely dump it all when the queue is destroyed. 
		vector would need an extra flag in msg to know to free it 
	
*/

/* wait-free single-reader single-writer fifo */
struct al_tube {
	unsigned int writer, reader;
	unsigned int size, wrap;
	msg packets;	
	//int blocked; /* prevent new messages */
};

/* 
	Alternative experimental implementation of tube
	
	What happens if the tube is empty (overflow)?
	
	If the current message is actually the last available one,
		Need to create a temporary buffer to store messages.
		All new messages will go to the temporary buffer until more room is available again. Use stl::list?
		The update() call also needs to handle the temporary buffer accordingly; until the list is emptied, then we free it, and go back to normal operations again.
		
	There could be several points in the pending tube where this has happened, but always temporary. 
		
	Changes; involves a lot of new ifs... and need a 'islist' flag in msg
	
		head_mem 
			if writer is a dynamic list
				append one new msg() and return pointer to it
			else if it is the last available slot (writer + 2) == reader
				create the list, append one new msg() and return pointer to it
			else (normal case)
				return pointer to current msg
				
		send 
			if it is NOT the last available slot (writer + 2) != reader
				increment writer
				
		reader_head
			if reader == writer return NULL
			if reader is dynamic list, return list front
			else return reader index
		reader_done
			if reader is dynamic list, pop from list
			if list is empty, increment reader index
*/



/* allocate a new priority queue */
pq al_pq_create(int size, al_sec birth) {
	pq_msg array;
	int i;
	pq x = (pq)malloc(sizeof(struct al_pq));
	array = (pq_msg)malloc(size * sizeof(struct al_pq_msg));
	if (x && array) {
		x->head = x->tail = NULL;
		for (i=0; i<size; i++) {
			memset(array[i].msg.mem, 0, AL_PQ_MSG_ARGS_SIZE);
			array[i].msg.func = NULL;
			array[i].retry = 0;
			if (i==size-1) {
				array[i].next = NULL;
			} else {
				array[i].next = &array[i+1];
			}
		}
		x->pool = array;
		x->len = 0;
		x->now = birth;
		x->retry_period = 1;
		ddebug("pq using %i bytes\n", sizeof(struct al_pq) + sizeof(struct al_pq_msg) * size);
	}
	return x;
}

/* free a priority queue */
void al_pq_free(pq * ptr, int flush) {
	pq x;
	if (ptr && *ptr) {
		x = *ptr;
		if (flush) {
			/* flush scheduled messages */
			al_pq_update(x, HUGE_VALF, 0);
		}
		free(x);
		*ptr = 0;
	}
}

/* grab memory for the next message */
char * al_pq_msg(pq x) {
	if (x->pool == NULL) 
		return NULL;
	return x->pool->msg.mem;
}

/* (re?)schedule a message */
void al_pq_sched_msg(pq x, pq_msg m) {
	pq_msg p, n;
	m->next = NULL;
	
	/* fill new msg struct */
	
	/* insert into queue */
	
	/* empty queue case */
	if (x->head == NULL) {
		ddebug("al_pq_sched case: empty %f %f\n", x->now, m->msg.t);
		x->head = m;
		x->tail = m;
		x->len = 1;
		return;
	}
	
	ddebug("head %f ", x->head->msg.t);
	
	/* prepend case */
	if (m->msg.t < x->head->msg.t) {
		ddebug("al_pq_sched case: prepend %f %f\n", x->now, m->msg.t);
		m->next = x->head;
		x->head = m;
		x->len++;
		ddebug("new head %f\n", x->head->msg.t);
		return;
	}
	
	/* append case */
	if (m->msg.t >= x->tail->msg.t) {
		ddebug("al_pq_sched case: append %f %f\n", x->now, m->msg.t);
		x->tail->next = m;
		x->tail = m;
		x->len++;
		return;
	}
	
	/* somewhere between head & tail */
	ddebug("al_pq_sched case: insert %f %f\n", x->now, m->msg.t);
	p = x->head;
	n = x->head->next;
	/* compare with <= so that events with same timestamp will be in order of insertion */
	while (n && n->msg.t <= m->msg.t) {
		p = n;
		n = n->next;
	}
	m->next = n;
	p->next = m;
	x->len++;
}

/* schedule a new message */
void al_pq_sched(pq x, al_sec t, al_sec retry, al_msg_func f) {
	pq_msg m;
	
	/* pop from pool */
	m = x->pool;
	if (m == NULL) {
		ddebug("error: empty pool");
		return;
	}
	x->pool = m->next;
	
	
	/* fill new msg struct */
	m->next = NULL;
	m->msg.t = t;
	m->msg.func = f;
	m->retry = retry;
	
	/* insert into queue */
	al_pq_sched_msg(x, m);
}

/* push a message back into the pool */
void al_pq_recycle(pq x, pq_msg m) {
	m->next = x->pool;
	x->pool = m;
}


// TODO: this is wrong; need to check value of mem, not address!
void al_pq_cancel_ptr(pq x, void * ptr) {
	// iterate entire queue
	pq_msg p, n, m;
	p = x->head;
	
	//printf("cancel pq msg %p (%i)\n", ptr, x->len);
	
	/* check head(s) */
	while (p && *(void **)p->msg.mem == ptr) {
		x->head = p->next;
		p->next = x->pool;
		x->pool = p;
		p = x->head;
	}
	
	/* make sure there are still values */
	if (p == NULL) return;
	
	n = p->next;
	while (n) {
		if (*(void **)n->msg.mem == ptr) {
			// todo: verify this
			m = n->next;
			p->next = m;
			al_pq_recycle(x, n);
			n = m;
		} else {
			p = n;
			n = n->next;
		}
	}
}

/* free one */
void al_pq_msg_free(pq_msg * ptr) {
	// TODO: use a per-queue pool?
	pq_msg x;
	if (ptr && *ptr) {
		x = *ptr;
		free(x);
		*ptr = 0;
	}
}

/* read the top message */
pq_msg al_pq_top(pq x) {
	return x->head;
}
/* pop the top message, return the next */
pq_msg al_pq_pop(pq x) {
	pq_msg n = NULL;
	if (x->head) {
		n = x->head;
		x->head = n->next;
		x->len--;
	}
	return n;
}

/* convenience wrapper to call all scheduled functions up to a given timestamp */
void al_pq_update(pq x, al_sec until, int defer) {
	int result;
	pq_msg m = al_pq_top(x);
	ddebug("update to %f\n", until);
	while (m && m->msg.t <= until) {
		x->now = MAX(x->now, m->msg.t); 
		al_pq_pop(x);
		ddebug("call next %f\n", m->msg.t);
		
		// deferable?
		if (defer && m->retry > 0.) {
			m->msg.t = x->now + m->retry;
			al_pq_sched_msg(x, m);
		} else {	
			result = m->msg.func(m->msg.t, m->msg.mem);
			if (result) {
				/* message was not handled; reschedule it: */
				m->msg.t = x->now + x->retry_period;
				al_pq_sched_msg(x, m);
			} else {
				al_pq_recycle(x, m);
			}
		}
		m = al_pq_top(x);
	}
	/* update clock */
	x->now = until;
}
void al_pq_advance(pq x, al_sec step, int defer) {
	al_pq_update(x, x->now + step, defer);
}


int al_pq_used(pq x) {
	return x->len;
}

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
		ddebug("tube using %i bytes\n", sizeof(struct al_tube) + sizeof(struct al_msg) * x->size);
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
			al_pq_sched(q, m->t, 0, m->func);
		} else {
			printf("pq empty\n"); /* retry? */
		}
		al_tube_reader_done(x);
		m = al_tube_reader_head(x);
	}
}
