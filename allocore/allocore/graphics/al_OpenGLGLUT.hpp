#ifndef INCLUDE_AL_OPENGLGLUT_HPP
#define INCLUDE_AL_OPENGLGLUT_HPP


#include "allocore/system/al_Config.h"

#if defined AL_OSX
	#define AL_GRAPHICS_USE_OPENGL
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>
	#define AL_GRAPHICS_INIT_CONTEXT
//	#define AL_GRAPHICS_INIT_CONTEXT\
//		/* prevents tearing */ \
//		{	GLint MacHackVBL = 1;\
//			CGLContextObj ctx = CGLGetCurrentContext();\
//			CGLSetParameter(ctx, kCGLCPSwapInterval, &MacHackVBL); }

#elif defined AL_LINUX
	#define AL_GRAPHICS_USE_OPENGL
	#include <GL/glew.h> // needed for certain parts of OpenGL API
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <time.h>
	#define AL_GRAPHICS_INIT_CONTEXT\
		{	GLenum err = glewInit();\
			if (GLEW_OK != err){\
  				/* Problem: glewInit failed, something is seriously wrong. */\
  				fprintf(stderr, "GLEW Init Error: %s\n", glewGetErrorString(err));\
			}\
		}
#elif defined AL_WINDOWS
	#define AL_GRAPHICS_USE_OPENGL
	#include <GL/glew.h> // needed for certain parts of OpenGL API
	#include <GL/gl.h>
	#include <GL/glu.h>
	#pragma comment( lib, "winmm.lib")
	#pragma comment( lib, "opengl32.lib" )
	#pragma comment( lib, "glu32.lib" )
	#define AL_GRAPHICS_INIT_CONTEXT\
		{	GLenum err = glewInit();\
			if (GLEW_OK != err){\
  				/* Problem: glewInit failed, something is seriously wrong. */\
  				fprintf(stderr, "GLEW Init Error: %s\n", glewGetErrorString(err));\
			}\
		}

#else
	#ifdef __IPHONE_2_0
		#define AL_GRAPHICS_USE_OPENGLES1

		#import <OpenGLES/ES1/gl.h>
		#import <OpenGLES/ES1/glext.h>
	#endif
	#ifdef __IPHONE_3_0
		#define AL_GRAPHICS_USE_OPENGLES2

		#import <OpenGLES/ES2/gl.h>
		#import <OpenGLES/ES2/glext.h>
	#endif
#endif


#endif