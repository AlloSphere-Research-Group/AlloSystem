#ifndef INCLUDE_AL_TIME_HPP
#define INCLUDE_AL_TIME_HPP

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
	C++ helper wrappers for al_time

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

/*
	
*/

#include "allocore/system/al_Time.h"

namespace al{

/// Sleep for an interval of seconds
inline void wait(al_sec dt){ al_sleep(dt); }

/// Get current wall time in seconds
inline al_sec walltime(){ return al_time(); }
inline al_sec timeNow(){ return al_time(); }

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



/// Self-correcting timer

///	Helper object for events intended to run at a particular rate/period,
///		but which might be subject to drift, latency and jitter.
///	A curve mapping logical to real time can be drawn over an event period 
///		by interpolating between t0 and t1
///	@see http://www.kokkinizita.net/papers/usingdll.pdf
class DelayLockedLoop {
public:
	
	DelayLockedLoop(al_sec step_period, double bandwidth = 0.5) {
		tperiod = step_period;
		setBandwidth(bandwidth);
		mReset = true;
	}
	
	/// Set degree of smoothing
	void setBandwidth(double bandwidth);
	
	/// Call this after an xrun: will reset the timing adjustments
	void reset() { mReset = true; }
	
	/// Trigger this from the periodic event
	
	/// @param[in] realtime		source time that we are trying to smoothly follow
	///
	void step(al_sec realtime);
	void operator()(al_sec realtime) { step(realtime); }
	
	/// Get the current period estimation (smoothed)
	al_sec period_smoothed() const { return t2; }

	/// Get the current rate estimation (smoothed) 
	al_sec rate_smoothed() const { return 1./t2; }

	/// Get the ideal period
	al_sec period_ideal() const { return tperiod; }

	/// Get the ideal rate
	al_sec rate_ideal() const { return 1./tperiod; }
	
	/// Returns time estimate between current and next event

	/// This returns an estimate of corresponding real-time between current 
	/// event & next by linear interpolation of current event timestamp & 
	/// projected next event timestamp.
	al_sec realtime_interp(double alpha) const { return t0 + alpha*(t1-t0); }

protected:
	al_sec tperiod;	// event period in seconds
	al_sec t0;		// 0th-order component: timestamp of the current event
	al_sec t1;		// 1st-order component: ideally the timestamp of the next event
	al_sec t2;		// 2nd-order component (akin to acceleration, or smoothed event period)
	double mB, mC;	// 1st & 2nd order weights	
	bool mReset;
};

/// A timing source that can be locked to realtime or driven manually
/// 
class Clock {
public:

	/// Constructor that defaults to realtime mode
	Clock(bool useRT = true) 
	: mNow(0), mReferenceTime(al_time()), mDT(0.33), mFPS(1./mDT), mFrame(0), bUseRT(useRT) 
	{}

	/// Constructor that defaults to a fixed 'frame rate'
	Clock(al_sec dt) 
	: mNow(0), mReferenceTime(al_time()), mDT(dt), mFPS(1./mDT), mFrame(0), bUseRT(false) 
	{}
	
	/// get current clock time
	al_sec now() const { return mNow; }
	al_sec operator()() const { return mNow; }
	
	/// get current delta time
	al_sec dt() const { return mDT; }
	
	/// get current FPS:
	double fps() const { return mFPS; }
	
	/// get current frame:
	unsigned frame() const { return mFrame; }
	
	/// get current mode:
	bool rt() const { return bUseRT; }
	
	/// update the internal clock. 
	al_sec update() { 
		if (bUseRT) {
			al_sec t2 = al_time() - mReferenceTime;
			mDT = t2 - mNow;
			mNow = t2;
			mFrame++;
			mFPS = mFPS + 0.1 * ((1./mDT) - mFPS);
		} else {
			mNow += mDT;
			mFrame++;
			mFPS = 1./mDT;
		}
		return now();
	}
	al_sec update(al_sec dt) { 
		mDT = dt;
		return update();
	}
	
	/// set RT mode:
	void useRT() {
		if (!bUseRT) {
			// need to reset reference time:
			mReferenceTime = al_time() - mNow;
		}
		bUseRT = true;
	}
	/// set NRT mode with specific frame rate:
	void useNRT(al_sec dt) {
		mDT = dt;
		bUseRT = false;
	}

protected:
	al_sec mNow, mReferenceTime, mDT;
	double mFPS;
	unsigned mFrame;
	bool bUseRT;
};



} // al::

#endif /* INCLUDE_AL_TIME_CPP_H */

