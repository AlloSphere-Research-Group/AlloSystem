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

	MainLoop(main_tick_handler tickhandler, main_quit_handler quithandler, void * userdata);

	/// takes over control of the current thread
	void enter();
	
	/// releases control of the current thread 
	/// in some implementations, may exit the application
	void exit();
	
	/// current scheduler logical time
	al_nsec time_nsec() { return mLogicalTime; }
	al_sec time() { return time_nsec() * al_time_ns2s; }

protected:

	bool mIsRunning;				/* flag true (1) when in the main loop */
	double mInterval;			/* in seconds */
	al_nsec mT0, mLogicalTime;		/* birth time (wall clock), scheduler time (logical) */
	main_tick_handler mTickHandler;	/* user-supplied event handler */
	main_quit_handler mQuitHandler; /* (optional) user-supplied quit handler */
	void * mUserData;			/* passed to the handler */
	
	MsgQueue mQueue;			/// functor scheduler attached to the main loop

	void tick();

};

} // al::

#endif /* include guard */
