#include "types/al_MsgQueue.hpp"

using namespace al;

MsgQueue :: MsgQueue(al_sec birth, int size) 
:	mHead(NULL), mTail(NULL), mLen(0), mChunkSize(size), mNow(birth)
{
	// initialize pool:
	growPool();
}

void MsgQueue :: growPool() {
	int size = mChunkSize;
	mPool = new Msg;
	Msg * m = mPool;
	while (size--) {
		m->next = new Msg;
		m = m->next;
	}
	m->next = NULL;
}

/* push a message back into the pool */
void MsgQueue :: recycle(Msg * m) {
	m->next = mPool;
	mPool = m;
}

/* schedule a new message */
void MsgQueue :: sched(al_sec at, msg_func func, char * data, size_t size) {
	
	// get a message-holder from the pool:
	if (mPool == NULL) growPool();
	Msg * m = mPool;
	mPool= m->next;
	
	// prepare Msg:
	m->next = NULL;
	m->t = at;
	m->func = func;
	m->mem = data;
	m->size = size;
	
	// insert into queue
	
	// empty queue? set as new head and tail:
	if (mHead == NULL) {
		mHead = m;
		mTail = m;
		mLen = 1;
		return;
	}
	
	// prepend case? insert as new head:
	if (at < mHead->t) {
		m->next = mHead;
		mHead = m;
		mLen++;
		return;
	}
	
	// append case? add as new tail:
	if (at >= mTail->t) {
		mTail->next = m;
		mTail = m;
		mLen++;
		return;
	}
	
	// else: insert somewhere between head and tail:
	Msg * p = mHead;
	Msg * n = p->next;
	// compare with <= so that events with same timestamp will be in order of insertion 
	while (n && n->t <= at) {
		p = n;
		n = n->next;
	}
	m->next = n;
	p->next = m;
	mLen++;
}

void MsgQueue :: cancel(msg_func func, void * ptr) {
	// iterate entire queue
	Msg * p, * n, * m;
	p = mHead;
	if (p) {	
		n = p->next;
		while (n) {
			if (static_cast<void *>(n->mem) == ptr && func == n->func) {
				// todo: verify this
				m = n->next;
				p->next = m;
				recycle(n);
				n = m;
			} else {
				p = n;
				n = n->next;
			}
		}
	}
}

void MsgQueue :: update(al_sec until, bool defer) {
	
	Msg * m = mHead;
	while (m && m->t <= until) { 
		mHead = m->next;
		
//		if (defer && m->retry > 0.) {
//			m->msg.t = x->now + m->retry;
//			al_pq_sched_msg(x, m);
//		} else {	
		
		mNow = MAX(mNow, m->t); 
		(m->func)(m->t, m->mem);
		
		recycle(m);
		m = mHead;
	}
	mNow = until;
}

