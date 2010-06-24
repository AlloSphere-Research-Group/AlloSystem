#ifndef INCLUDE_AL_MSGQUEUE_HPP
#define INCLUDE_AL_MSGQUEUE_HPP

/*
 *	Priority queue of scheduled function calls
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

#include <string.h>

#include "system/al_Config.h"

namespace al {

// messages that are larger than this will be heap copied
#define AL_PQ_MSG_ARGS_SIZE (52)

class MsgQueue {
public:	

	typedef void (*msg_func)(al_sec t, char * args);
	
	MsgQueue(al_sec birth = 0, int size = 256);
	
	// for truly accurate scheduling, always use this as logical time:
	al_sec now() { return mNow; }

	void update(al_sec until, bool defer = false);
	void advance(al_sec period, bool defer = false) { update(mNow + period, defer); }
	
	void sched(al_sec at, msg_func func, char * data, size_t size);
	
	// removes any scheduled msg with matching func & data pointers:
	void cancel(msg_func func, void * data);
	
	
protected:

	// Messages in the queue have the following structure:
	struct Msg {
		struct Msg * next;
		size_t size;
		al_sec t;
		msg_func func;
		char * mem;
	};
	
	Msg * mHead;
	Msg * mTail;
	Msg * mPool;
	int mLen, mChunkSize;
	al_sec mNow;
	
	void growPool();
	void recycle(Msg * m);
};


	
} // al::

#endif // include guard
