#ifndef INCLUDE_AL_OPENGLGLFW_HPP
#define INCLUDE_AL_OPENGLGLFW_HPP


#if defined AL_OSX
	#define AL_GRAPHICS_USE_OPENGL
	#include <OpenGL/gl.h>
	#define GLFW_INCLUDE_GLCOREARB
	// #define GLFW_INCLUDE_GLU
	// #define GLFW_INCLUDE_GLEXT
	#include <GLFW/glfw3.h>
	#define AL_GRAPHICS_INIT_CONTEXT

#elif defined AL_LINUX
	#define AL_GRAPHICS_USE_OPENGL
	#include <GL/glew.h> // needed for certain parts of OpenGL API
	#define GLFW_INCLUDE_GLCOREARB
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
	#define GLFW_INCLUDE_GLCOREARB
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