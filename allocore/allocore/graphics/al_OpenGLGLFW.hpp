#ifndef INCLUDE_AL_OPENGLGLFW_HPP
#define INCLUDE_AL_OPENGLGLFW_HPP

#if defined AL_OSX
	#define AL_GRAPHICS_USE_OPENGL
	#include <OpenGL/gl3.h>
	// #include <OpenGL/gl3ext.h>
	#define GLFW_INCLUDE_NONE
	// #define GLFW_INCLUDE_GLCOREARB
	// #define GLFW_INCLUDE_GLEXT
	#include <GLFW/glfw3.h>
	#define AL_GRAPHICS_INIT_CONTEXT

#elif defined AL_LINUX
	#define AL_GRAPHICS_USE_OPENGL
	#include <GL/glew.h> // needed for certain parts of OpenGL API
	#include <GL/glcorearb.h>
	// #include <GL/glcoreext.h>
	#define GLFW_INCLUDE_NONE
	// #define GLFW_INCLUDE_GLCOREARB
	// #define GLFW_INCLUDE_GLEXT
	#include <GLFW/glfw3.h>
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
	#include <GL/glcorearb.h>
	// #include <GL/glcorext.h>
	#define GLFW_INCLUDE_NONE
	// #define GLFW_INCLUDE_GLCOREARB
	// #define GLFW_INCLUDE_GLEXT
	#include <GLFW/glfw3.h>
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
		// TODO
	#endif
	#ifdef __IPHONE_3_0
		#define AL_GRAPHICS_USE_OPENGLES2
		// TODO
	#endif
#endif


#endif