#include "allocore/system/al_MainLoop.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp"

#include <stdlib.h>		// exit
#include <algorithm>	// std::find

// native bindings:

extern "C" void al_main_native_init();
extern "C" void al_main_native_attach(al_sec interval);
extern "C" void al_main_native_enter(al_sec interval);
extern "C" void al_main_native_stop();

#ifdef AL_LINUX
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


////////////////////////////////////////////////////////////////
// GLUT impl:
extern "C" void al_main_glut_init();
extern "C" void al_main_glut_attach(al_sec interval);
extern "C" void al_main_glut_enter(al_sec interval);
extern "C" void al_main_glut_stop();

#ifdef AL_MAINLOOP_NO_GLUT
	extern "C" void al_main_glut_init(){
		AL_WARN("GLUT loop not available");
	}
	extern "C" void al_main_glut_attach(al_sec interval){}
	extern "C" void al_main_glut_enter(al_sec interval){}
	extern "C" void al_main_glut_stop(){}
#else

// BEGIN al_MainLoopGLUT.cpp
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/system/al_Config.h"

#ifdef AL_OSX
	#include <OpenGL/OpenGL.h>
	#include <GLUT/glut.h>
#endif
#ifdef AL_LINUX
	#include <GL/glew.h>
	#include <GL/glut.h>
#endif
#ifdef AL_WINDOWS
	#include <GL/glew.h>
	#include <GL/glut.h>
#endif

static void mainGLUTExitFunc(){
	// call any exit handlers:
	al::Main::get().exit();
}

static void mainGLUTTimerFunc(int id) {
	//printf("mainGLUTTimerFunc\n");
	al::Main& M = al::Main::get();
	M.tick();
	if (M.isRunning()) {
        // schedule another tick:
        glutTimerFunc((unsigned int)(1000.0*M.interval()), mainGLUTTimerFunc, 0);
    }
}

extern "C" void al_main_glut_init(){
	int argc = 1;
	char name[] = {'a','l','l','o','\0'};
	char * argv[] = {name};
	glutInit(&argc,argv);
	atexit(mainGLUTExitFunc);
}

extern "C" void al_main_glut_attach(al_sec interval){
}

extern "C" void al_main_glut_enter(al_sec interval){
	// start periodic timer
	glutTimerFunc(0 /*msec*/, mainGLUTTimerFunc, 0);

	// start the GLUT mainloop
	glutMainLoop();
}

extern "C" void al_main_glut_stop(){
	// GLUT can't be stopped; the only option is a hard exit.
	::exit(0); // Note: this will call our function registered with atexit()
}
// END al_MainLoopGLUT.cpp

#endif


namespace al {

////////////////////////////////////////////////////////////////

Main::Handler :: ~Handler() {
	Main::get().remove(*this);
	onExit();
}

////////////////////////////////////////////////////////////////

Main::Main()
:	mT0(al_time()), mT1(0),
	mInterval(0.01),
	mIntervalActual(0.01),
	mLogicalTime(0),
	mCPU(0),
	mDriver(Main::SLEEP),
	mActive(false)
{
	for(unsigned i=0; i<NUM_DRIVERS; ++i){
		mInited[i] = false;
	}
}

Main::~Main() {
	Main::exit();
}

Main& Main::driver(Driver v) {
	//if (mDriver != GLUT) mDriver = v;
	if(!mInited[v]){
		switch(v){
			case GLUT: al_main_glut_init(); break;
			case NATIVE: al_main_native_init(); break;
			default:;
		}
		mInited[v] = true;
	}

	mDriver = v;
	return *this;
}

void Main::tick() {
	al_sec t1 = al_time();
	mLogicalTime = t1 - mT0;

	mIntervalActual = t1 - mT1;
	mT1 = t1;

	// trigger any scheduled functions:
	mQueue.update(mLogicalTime);

	// call tick handlers...
	std::vector<Handler *>::iterator it = mHandlers.begin();
	while(it != mHandlers.end()){
		(*it)->onTick();
		++it;
	}

	// measure CPU usage:
	al_sec t2 = al_time();
	al_sec used = (t2-t1)/interval();
	// running average:
	mCPU += 0.1 * (used - mCPU);
}

Main& Main::get() {
	// This has to be dynamically allocated,
	// otherwise it can get destroyed at some random time
	static Main * gMain = new Main;
	return *gMain;
}

struct ForceMainThreadMain {
	ForceMainThreadMain() {
		Main::get();
	}
};

static ForceMainThreadMain fmtm;

void Main::start() {
	if (!mActive) {
		mActive = true;

		// Here we enter the main loop which will block until stopped
		switch (mDriver) {
			case Main::GLUT: al_main_glut_enter(interval()); break;
			case Main::NATIVE: al_main_native_enter(interval()); break;
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
			case GLUT: al_main_glut_stop(); break;
			case NATIVE: al_main_native_stop(); break;
			default:; // Here, mActive==false stops the loop
		}
	}
}

void Main::exit() {
	// call exit handlers...
	std::vector<Handler *>::iterator it = mHandlers.begin();
	while(it != mHandlers.end()){
		(*it)->onExit();
		++it;
	}
}


Main& Main::add(Main::Handler& v) {
	if (std::find(mHandlers.begin(), mHandlers.end(), &v) == mHandlers.end()) {
		mHandlers.push_back(&v);
	}
	return *this;
}

Main& Main::remove(Main::Handler& v) {
	std::vector<Handler *>::iterator it = std::find(mHandlers.begin(), mHandlers.end(), &v);
	if (it != mHandlers.end()) {
		mHandlers.erase(it);
	}
	return *this;
}


} //al::

