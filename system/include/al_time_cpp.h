#ifndef INCLUDE_AL_TIME_CPP_H
#define INCLUDE_AL_TIME_CPP_H

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
	C++ helper wrapeprs for al_time
*/

#include "al_time.h"

namespace allo{

/// Timer with stopwatch-like functionality for benchmarking, etc.
class Timer {
public:
	Timer(): mStart(0), mStop(0){}

	al_nsec elapsed(){ return mStop - mStart; }					///< Returns nsec between start() and stop() calls
	al_sec elapsedSec(){ return al_time_ns2s * elapsed(); }		///< Returns  sec between start() and stop() calls
	void start(){ mStart=al_time_nsec(); }							///< Set start time as current time
	void stop(){ mStop=al_time_nsec(); }								///< Set stop time as current time

private:
	al_nsec mStart, mStop;	// start and stop times
};

} // allo::

#endif /* INCLUDE_AL_TIME_CPP_H */

