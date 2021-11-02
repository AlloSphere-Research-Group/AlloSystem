#include <algorithm>	// std::find
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp"
#include "allocore/system/al_Time.h" // al_sleep

// native bindings:

extern "C" void al_main_native_init();
extern "C" void al_main_native_attach(al_sec interval);
extern "C" void al_main_native_enter(al_sec interval);
extern "C" void al_main_native_stop();

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>

	extern "C" void al_main_native_init(){}
	extern "C" void al_main_native_attach(al_sec interval){}
	extern "C" void al_main_native_enter(al_sec interval){
		// If fps<=0, uses the browser’s requestAnimationFrame mechanism
		// to call the function.
		int fps = int(1./interval);
		emscripten_set_main_loop(
			[](){ al::Main::get().tick(); },
			fps, 1
		);
	}
	extern "C" void al_main_native_stop(){
		emscripten_cancel_main_loop();
	}

#elif defined AL_LINUX
	extern "C" void al_main_native_init(){
		AL_WARN("Linux native loop not yet implemented");
	}
	extern "C" void al_main_native_attach(al_sec interval){}
	extern "C" void al_main_native_enter(al_sec interval){}
	extern "C" void al_main_native_stop(){}

#elif defined AL_WINDOWS
	extern "C" void al_main_native_init(){
		AL_WARN("Win32 native loop not yet implemented");
	}
	extern "C" void al_main_native_attach(al_sec interval){}
	extern "C" void al_main_native_enter(al_sec interval){}
	extern "C" void al_main_native_stop(){}
#endif


namespace al {

////////////////////////////////////////////////////////////////

Main::Handler :: ~Handler() {
	Main::get().remove(*this);
	onExit();
}

////////////////////////////////////////////////////////////////

al_sec timeInSec(){ return al_steady_time(); }

Main::Main()
:	mT0(timeInSec()), mT1(0),
	mInterval(0.01),
	mIntervalActual(0.01),
	mLogicalTime(0),
	mCPU(0),
	mActive(false)
{
	for(auto& v : mInited) v = false;
	driver(Main::DEFAULT);
}

Main::~Main() {
	Main::exit();
}

/*static*/ Main& Main::get() {
	// This has to be dynamically allocated,
	// otherwise it can get destroyed at some random time
	static Main * gMain = new Main;
	return *gMain;
}

Main& Main::driver(Driver v) {

	if(v == Main::DEFAULT){
		#ifdef __EMSCRIPTEN__
		v = NATIVE;
		#else
		v = SLEEP;
		#endif
	}

	if(!mInited[v]){
		bool inited = true;
		switch(v){
			case NATIVE: al_main_native_init(); break;
			case USER:
				if(userInit) userInit();
				else inited = false;
				break;
			default:;
		}
		mInited[v] = inited;
	}

	mDriver = v;
	return *this;
}

Main& Main::interval(al_sec v) {
	mInterval = v > 0.001 ? v : 0.001;
	return *this;
}

al_sec Main::realtime() {
	return timeInSec() - get().mT0;
}

void Main::tick() {
	al_sec t1 = timeInSec();
	mLogicalTime = t1 - mT0;

	mIntervalActual = t1 - mT1;
	mT1 = t1;

	// trigger any scheduled functions:
	mQueue.update(mLogicalTime);

	// call tick handlers...
	for(auto * h : mHandlers) h->onTick();

	// measure CPU usage:
	al_sec t2 = timeInSec();
	al_sec used = (t2-t1)/interval();
	// running average:
	mCPU += 0.1 * (used - mCPU);
}

void Main::start() {
	if (!mActive) {
		mActive = true;

		// Here we enter the main loop which will block until stopped
		switch (mDriver) {
			case Main::NATIVE: al_main_native_enter(interval()); break;
			case Main::USER: if(userEnter) userEnter(interval()); break;
			default:
				// default sleep version
				while (mActive) {
					tick();
					al_sleep(interval());
				}
				break;
		}

		// if we got here, then the mainloop was started, and then stopped:
		// trigger exit handlers:
		Main::exit();
	}
}

void Main::stop() {
	if (mActive) {
		mActive = false;

		switch(mDriver){
			case NATIVE: al_main_native_stop(); break;
			case USER: if(userStop) userStop(); break;
			default:; // Here, mActive==false stops the loop
		}
	}
}

void Main::exit() {
	// call exit handlers...
	for(auto * h : mHandlers) h->onExit();
}


Main& Main::add(Main::Handler& v) {
	if (std::find(mHandlers.begin(), mHandlers.end(), &v) == mHandlers.end()) {
		mHandlers.push_back(&v);
	}
	return *this;
}

Main& Main::remove(Main::Handler& v) {
	auto it = std::find(mHandlers.begin(), mHandlers.end(), &v);
	if (it != mHandlers.end()) {
		mHandlers.erase(it);
	}
	return *this;
}


} //al::

