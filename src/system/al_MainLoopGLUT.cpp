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

static void onIdle() {
	MainLoop::get().tick();
}

static void timerFunc(int id) {
	MainLoop& m = MainLoop::get();
	glutTimerFunc((unsigned int)(1000.0*m.interval()), timerFunc, id);
	m.tick();
}

/// implementation of MainLoop for the GLUT target

MainLoop :: MainLoop() 
:	mIsRunning(0), mInterval(0.01), mT0(al_time()),
	mImpl(NULL)
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

void MainLoop :: tick() {
	al_sec logicaltime = al_time() - mT0;
	
	// trigger any scheduled functions:
	mQueue.update(logicaltime);
}

/// in the GLUT case, we better have created a Window before using this...
void MainLoop :: start() {
	MainLoop& M = get();
	if (!M.mIsRunning) {
		M.mIsRunning = 1;
		timerFunc(0);
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



