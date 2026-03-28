#ifndef INC_AL_MAINLOOP_HPP
#define INC_AL_MAINLOOP_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Abstraction over an application main loop

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <vector>
#include "allocore/system/al_Config.h" // al_sec
#include "allocore/types/al_MsgQueue.hpp"

namespace al {

/// @addtogroup allocore
/// @{

/// Encapsulates a program's main run loop

/// A run loop is used for programs that need to persist and execute actions
/// at a periodic interval.
class Main {
public:

	/// A custom function object that gets called by the main loop
	class Handler {
	public:
		virtual ~Handler();
		virtual void onTick() {}
		virtual void onExit() {}
	};


	/// Main loop driver
	enum Driver {
		DEFAULT = 0,///< Use an appropriate default driver (usually SLEEP)
		SLEEP,		///< Uses platform-specific sleep function
		NATIVE,		///< Use platform specific run loop
		USER,		///< Use user-defined functions
		NUM_DRIVERS
	};


	/// Main is a singleton; this is how to access it
	static Main& get();


	/// Set the timer interval in seconds (minimum 1 millisecond)

	/// The actual behavior of this function is driver dependent.
	///
	Main& interval(al_sec v);

	/// Start the main loop

	/// This takes over control of the current thread and starts the
	/// clock-driven scheduler.
	void start();

	/// Stop the main loop

	/// This releases control of the current thread where it was start()ed
	/// and stops the clock-driven scheduler. In some implementations
	/// (e.g. GLUT), it may exit the application.
	void stop();

	bool isRunning() const { return mActive; }

	/// Get requested time between updates
	al_sec interval() const { return mInterval; }

	/// Get measured time between updates
	al_sec intervalActual() const { return mIntervalActual; }

	/// Get current scheduler (logical) time (seconds since start())
	al_sec now() const { return mQueue.now(); }

	/// Get real time (seconds since start())
	al_sec realtime();

	/// Get time when main loop was created
	al_sec T0() { return mT0; }

	/// Get percentage of interval time used in processing
	double cpu() const { return mCPU; }

	/// Use this to schedule timed functions in this loop

	/// The mainloop itself will take care of updating this queue.
	///
	MsgQueue& queue() { return mQueue; }

	/// Register callback for loop events
	Main& add(Main::Handler& v);
	Main& remove(Main::Handler& v);

	// INTERNAL USE:

	// Trigger a mainloop step (typically for implementation use only)
	void tick();

	// Calls any registerd Handlers' onExit() methods
	void exit();

	// Switch backend driver
	// Typically not called by user code
	// but e.g. creating a Native window will switch to NATIVE mode
	Main& driver(Driver v);

	// Must set before calling driver(Main::USER)
	void (* userInit)() = nullptr;
	void (* userAttach)(al_sec interval) = nullptr;
	void (* userEnter)(al_sec interval) = nullptr;
	void (* userStop)() = nullptr;

private:
	// private constructor for singleton pattern
	Main();
	~Main();

	al_sec mT0, mT1;
	al_sec mInterval, mIntervalActual;
	al_sec mLogicalTime;
	double mCPU;

	// timing driver
	Driver mDriver;

	// functor scheduler attached to the main loop
	MsgQueue mQueue;

	std::vector<Handler *> mHandlers;

	bool mActive;
	bool mInited[NUM_DRIVERS];
};

/// @} // end allocore group

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
#endif
