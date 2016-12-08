#ifndef INCLUDE_AL_PERIODIC_THREAD_HPP
#define INCLUDE_AL_PERIODIC_THREAD_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Thread that calls a function periodically

	File author(s):
	Lance Putnam, 2013, putnam.lance@gmail.com
*/

#include "allocore/system/al_Thread.hpp"
#include "allocore/system/al_Time.hpp"

namespace al{

/// Thread that calls a function periodically

/// The sleep time is dynamically adjusted based on the time taken by the
/// user-supplied thread function. This prevents drift that would occur in a
/// more simplistic implementation using a fixed sleep interval.
///
/// @ingroup allocore
class PeriodicThread : public Thread{
public:

	/// @param[in] periodSec	calling period in seconds
	PeriodicThread(double periodSec=1);

	/// Copy constructor
	PeriodicThread(const PeriodicThread& other);


	/// Set autocorrection factor

	/// This parameter is used to compensate for occasional iterations that take
	/// longer than the expected iteration period. Smaller values mean the
	/// timing corrections will be spread over a larger number of iterations.
	/// If all iterations take longer than the period, then no autocorrection
	/// measures will be able to make up for the lost time.
	///
	/// @param[in] factor	Maximum fraction of one period, in [0,1], to try to
	///						make up each iteration if behind on timing.
	PeriodicThread& autocorrect(float factor);

	/// Set period, in seconds
	PeriodicThread& period(double sec);

	/// Get period, in seconds
	double period() const;

	/// Start calling the supplied function periodically
	void start(ThreadFunction& func);

	/// Stop the thread
	void stop();


	// Stuff for assignment
	friend void swap(PeriodicThread& a, PeriodicThread& b);
	PeriodicThread& operator= (PeriodicThread other);

private:
	static void * sPeriodicFunc(void * userData);
	void go();

	al_nsec mPeriod;
	al_nsec mTimeCurr, mTimePrev;	// time measurements between frames
	al_nsec mWait;					// actual time to sleep between frames
	al_nsec mTimeBehind;
	float mAutocorrect;
	ThreadFunction * mUserFunc;
	bool mRun;
};

} // al::

#endif
