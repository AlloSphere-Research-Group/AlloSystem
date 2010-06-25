#ifndef INCLUDE_AL_MAINLOOP_HPP
#define INCLUDE_AL_MAINLOOP_HPP

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

#include "system/al_Config.h"
#include "system/al_Time.h"
#include "system/al_MainLoop.h"
#include "types/al_MsgQueue.hpp"

namespace al {

/// Typically a single instance in the main thread
class MainLoop {
public:
	
	/// mainloop is a singleton; this is how to access it:
	static MainLoop& get();

	/// takes over control of the current thread
	static void start();
	
	/// releases control of the current thread 
	/// in some implementations, may exit the application
	static void stop();
	
	/// current scheduler time
	static al_sec now() { return get().mQueue.now(); }
	
	/// use this to schedule timed functions in this mainloop
	/// (the mainloop itself will take care of updating this queue)
	static MsgQueue& queue() { return get().mQueue; }
	
	/// set tick period. actual behavior is implementation dependent
	static void interval(al_sec interval);
	static al_sec interval() { return get().mInterval; }
	
	/// trigger a mainloop step (typically for implementation use only)
	void tick();

protected:

	friend class MainLoopImpl;
	class MainLoopImpl * mImpl;

	bool mIsRunning;				/* flag true (1) when in the main loop */
	double mInterval;				/* in seconds */
	al_sec mT0;						/* birth time (wall clock), scheduler time (logical) */
	
	MsgQueue mQueue;				/// functor scheduler attached to the main loop
	
	MainLoop();
	virtual ~MainLoop();
};

inline MainLoop& MainLoop :: get() {
	static MainLoop mainloop;
	return mainloop;
}

} // al::

#endif /* include guard */
