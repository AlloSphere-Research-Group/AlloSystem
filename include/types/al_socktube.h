#ifndef INCLUDE_AL_SOCKTUBE_H
#define INCLUDE_AL_SOCKTUBE_H 1

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
	Single-writer / single-reader non-blocking fifo stream
		for sending messages between a pair of threads
		(built using socketpair)
*/
typedef struct al_socktube_struct {
	int socks[2];
	
} al_socktube_t;
typedef al_socktube_t * al_socktube;

AL_API al_socktube al_socktube_create();
AL_API void al_socktube_free(al_socktube * x);

/*
	read data from the socktube (up to maximum length len)
	returns bytes read
*/
AL_API int al_socktube_parent_read(al_socktube x, char * buffer, size_t len);
AL_API int al_socktube_child_read(al_socktube x, char * buffer, size_t len);

/*
	write data to the socktube (of length len)
	returns 0 on successful write
*/
AL_API int al_socktube_parent_write(al_socktube x, char * buffer, size_t len);
AL_API int al_socktube_child_write(al_socktube x, char * buffer, size_t len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* include guard */
