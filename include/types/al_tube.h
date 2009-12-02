#ifndef INCLUDE_AL_TUBE_H
#define INCLUDE_AL_TUBE_H

/*
 *	Single-reader, single-writer FIFO
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

#include "al_pq.h"

#include <limits.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

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
typedef struct al_tube * tube;

/* create tube with 2^N message slots */
extern tube al_tube_create(int nbits);

/* free a tube */
extern void al_tube_free(tube * x);

/* return number of scheduled messages in the tube */
extern int al_tube_used(tube x);

/* get pointer next sendable message */
extern msg al_tube_write_head(tube x);
/* after filling the message, send it */
extern void al_tube_write_send_msg(tube x);

/* alternative API: */
/* get pointer to data of next sendable message */
extern char * al_tube_write_head_mem(tube x);
/* after filling the message data, assign the function to call, and send it */
extern void al_tube_write_send(tube x, al_sec t, al_msg_func f);

/* get pointer to memory of next receivable message */
extern msg al_tube_reader_head(tube x);
/* after handling the message, return it */
extern void al_tube_reader_done(tube x);

/* alternative API: */
/* convenience wrapper to call all scheduled functions up to a given timestamp */
extern void al_tube_reader_advance(tube x, al_sec until);

/* convenience wrapper to shunt message from an incoming tube to a local priority queue */
extern void al_tube_pq_transfer(tube x, pq q, al_sec until);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INCLUDE_AL_TUBE_H */

