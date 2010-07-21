#include "system/al_MainLoop.hpp"

#include <stdio.h>		// snprintf
#include <stdlib.h>		// exit
#include "system/al_Config.h"
#include "system/al_MainLoop.h"

#ifdef AL_OSX
	#include <OpenGL/OpenGL.h>
	#include <GLUT/glut.h>
	#define AL_GRAPHICS_INIT_CONTEXT\
		/* prevents tearing */ \
		{	GLint MacHackVBL = 1;\
			CGLContextObj ctx = CGLGetCurrentContext();\
			CGLSetParameter(ctx,  kCGLCPSwapInterval, &MacHackVBL); }
#endif
#ifdef AL_LINUX
	#include <GL/glew.h>
	#include <GL/glut.h>

	#define AL_GRAPHICS_INIT_CONTEXT\
		{	GLenum err = glewInit();\
			if (GLEW_OK != err){\
  				/* Problem: glewInit failed, something is seriously wrong. */\
  				fprintf(stderr, "GLEW Init Error: %s\n", glewGetErrorString(err));\
			}\
		}
#endif
#ifdef AL_WIN32
	#include <windows.h>
	#include <GL/glut.h>

	#define AL_GRAPHICS_INIT_CONTEXT
#endif

namespace al{

/// implementation of MainLoop for the GLUT target

MainLoop :: MainLoop() 
:	mImpl(NULL),
	mInterval(0.01), mActualInterval(0),
	mT0(al_time()), mLastTickTime(mT0),
	mCPU(0),
	mIsRunning(false)
{
	static bool doInit=true;
	if(doInit){
		doInit=false;
		int argc = 0;
		char * argv[] = {0};
		glutInit(&argc,argv);
	}
}

MainLoop :: ~MainLoop() {}

static void timerFunc(int id) { MainLoop::get().tick(); }

void MainLoop :: tick() {
	al_sec realtime = al_time();	
	al_sec logicaltime = realtime - mT0;
	
	/* 
		CPU usage measurement
		Measures relationship between ideal tick interval (mInterval), 
			and the actual duration between ticks.
		Thus, it measures cost of both mQueue.update(), 
			plus every other activity in this thread between successive ticks
		When using GLUT and vsync, it will always be substatial.
	*/
	mActualInterval = realtime - mLastTickTime;
	double cpu = (mActualInterval - mInterval)/mInterval;
	mCPU += 0.1 * (cpu - mCPU);		// running average
	mLastTickTime = realtime;		// store realtime for use in next tick()
	
	// trigger any scheduled functions:
	mQueue.update(logicaltime);
	
	// schedule another tick:
	glutTimerFunc((unsigned int)(1000.0*mInterval), timerFunc, 0);
}

/// in the GLUT case, we better have created a Window before using this...
void MainLoop :: start() {
	MainLoop& M = get();
	if (!M.mIsRunning) {
		M.mIsRunning = 1;
		M.mT0 = al_time();
		M.mLastTickTime = M.mT0-M.mInterval;
		glutTimerFunc((unsigned int)(1000.0*M.interval()), timerFunc, 0);
		glutMainLoop(); 
	}
}

void MainLoop :: stop() {
	MainLoop& M = get();
	if (M.mIsRunning) {
		M.mIsRunning = 0;
		// GLUT can't be stopped; the only option is a hard exit. Yeah, it sucks that bad.
		exit(0);
	}
}

void MainLoop :: interval(al_sec interval) { 
	get().mInterval = interval; 
}

} // al::



