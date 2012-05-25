#ifndef INCLUDE_AL_MAINLOOP_HPP
#define INCLUDE_AL_MAINLOOP_HPP

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
	Abstraction over an application mainloop

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/system/al_Config.h"
#include "allocore/system/al_Time.h"
#include "allocore/types/al_MsgQueue.hpp"

#include <vector>
#include <algorithm>

namespace al {


class Main {
public:

	// interface for handlers:
	class Handler {
	public:
		virtual ~Handler();
		virtual void onTick() {}
		virtual void onExit() {}
	};

	enum Driver {
		SLEEP = 0,
		GLUT,
		NATIVE
	};
	
	/// mainloop is a singleton; this is how to access it:
	static Main& get();
	
	/// set the timer interval in seconds (minimum 1 millisecond)
	/// actual behavior is driver dependent
	Main& interval(al_sec v) {
		mInterval = v > 0.001 ? v : 0.001;
		return *this;
	}
	
	/// takes over control of the current thread
	/// starts the clock-driven scheduler
	void start();
	
	/// releases control of the current thread where it was start()ed
	/// stops the clock-driven scheduler
	/// in some implementations (e.g. GLUT), may exit the application
	void stop();
	
	bool isRunning() const { return mActive; }
	
	/// requested time between updates:
	al_sec interval() const { return mInterval; }
	
	/// measured time between updates:
	al_sec intervalActual() const { return mIntervalActual; }
	
	/// current scheduler (logical) time
	/// (seconds since start())
	al_sec now() const { return mQueue.now(); }
	
	/// real time
	/// (seconds since start())
	al_sec realtime() { return al_time() - get().mT0; }
	
	/// time when main loop was created
	al_sec T0() { return mT0; }
	
	/// percentage of interval time used in processing
	double cpu() const { return mCPU; }
	
	/// use this to schedule timed functions in this mainloop
	/// (the mainloop itself will take care of updating this queue)
	MsgQueue& queue() { return mQueue; }
	
	/// register callbacks for Mainloop events:
	Main& add(Main::Handler& v);
	Main& remove(Main::Handler& v);
	
	// INTERNAL USE:
	
	/// trigger a mainloop step (typically for implementation use only)
	void tick();
	
	/// calls any registerd Handlers' onExit() methods
	void exit();
	
	// used to switch the driver
	// typically not called by user code
	// but e.g. creating a GLUT window will switch to GLUT mode
	// or creating a Native window will switch to NATIVE mode
	Main& driver(Driver v) {
		if (mDriver != GLUT) mDriver = v;
		return *this;
	}
	
private:
	// private constructor for singleton pattern
	Main();
	~Main();
	
	al_sec mT0, mT1;
	al_sec mInterval, mIntervalActual;
	al_sec mLogicalTime;
	double mCPU;
	
	/// timing driver; initially SLEEP
	Driver mDriver;
	
	/// functor scheduler attached to the main loop
	MsgQueue mQueue;
	
	std::vector<Handler *> mHandlers;
	
	bool mActive;
};

// deprecated; for backwards compatibility only
namespace MainLoop {
	inline Main& get() { return Main::get(); }
	inline void start() { return Main::get().start(); }
	inline void stop() { return Main::get().stop(); }
	inline al_sec now() { return Main::get().now(); }
	inline al_sec realtime() { return Main::get().realtime(); }
	inline MsgQueue& queue() { return Main::get().queue(); }
	inline void interval(al_sec v) { Main::get().interval(v); }
	inline al_sec interval() { return Main::get().interval(); }
	inline al_sec intervalActual() { return Main::get().intervalActual(); }
	inline double cpu() { return Main::get().cpu(); }
	inline al_sec T0() { return Main::get().T0(); }
	inline bool isRunning(){ return Main::get().isRunning(); }
};

} // al::

#endif /* include guard */
