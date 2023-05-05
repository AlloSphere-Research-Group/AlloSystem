#ifndef INCLUDE_AL_OPENGL_HPP
#define INCLUDE_AL_OPENGL_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Platform-specific header includes for OpenGL

	File author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/

#include "allocore/system/al_Config.h"

#if !(defined(AL_GRAPHICS_USE_DEFAULT_BACKEND) || defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES1) || defined(AL_GRAPHICS_USE_OPENGLES2))
	#define AL_GRAPHICS_USE_DEFAULT_BACKEND
#endif

#if defined AL_OSX
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGL
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGL
		#define GL_SILENCE_DEPRECATION
		#include <OpenGL/OpenGL.h>
		#include <OpenGL/gl.h>
		#include <OpenGL/glext.h>
		#define AL_GRAPHICS_INIT_CONTEXT
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#elif defined AL_LINUX
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGL
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGL
		#include <GL/glew.h> // needed for certain parts of OpenGL API
		#include <GL/gl.h>
		#include <GL/glext.h>
		#include <time.h>
		#define AL_GRAPHICS_INIT_CONTEXT\
			{	GLenum err = glewInit();\
				if (GLEW_OK != err){\
					/* Problem: glewInit failed, something is seriously wrong. */\
					fprintf(stderr, "GLEW Init Error: %s\n", glewGetErrorString(err));\
				}\
			}
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#elif defined AL_WINDOWS
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGL
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGL
		#ifndef __MINGW32__
			#define GLEW_NO_GLU // GLU not used and throws errors with Mingw-w64
		#endif
		#include <GL/glew.h> // needed for certain parts of OpenGL API
		#include <GL/gl.h>
		#pragma comment( lib, "winmm.lib")
		#pragma comment( lib, "opengl32.lib" )
		#define AL_GRAPHICS_INIT_CONTEXT\
			{	GLenum err = glewInit();\
				if (GLEW_OK != err){\
					/* Problem: glewInit failed, something is seriously wrong. */\
					fprintf(stderr, "GLEW Init Error: %s\n", glewGetErrorString(err));\
				}\
			}
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#elif defined AL_EMSCRIPTEN
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGLES2
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGLES2
		//#define GL_GLEXT_PROTOTYPES 1
		#include <GLES2/gl2.h>
		//#include <GLES2/gl2ext.h> // just include core API for now
		#define AL_GRAPHICS_INIT_CONTEXT
	#elif defined AL_GRAPHICS_USE_OPENGLES1
		#include <GLES/gl.h>
		#define AL_GRAPHICS_INIT_CONTEXT
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#elif defined __IPHONE_2_0
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGLES1
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGLES1
		#import <OpenGLES/ES1/gl.h>
		#import <OpenGLES/ES1/glext.h>
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#elif defined __IPHONE_3_0
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGLES2
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGLES2
		#import <OpenGLES/ES2/gl.h>
		#import <OpenGLES/ES2/glext.h>
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#endif // platform-specific


/* Define what parts of the OpenGL API are available.
We could possibly check directly for GLenums (e.g. GL_UNSIGNED_INT, GL_TEXTURE_1D)
but are these always guaranteed to be #define'ed?
A bit on GLenum allocations:
https://www.khronos.org/registry/OpenGL/docs/enums.html
*/

#if defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_USE_PROG_PIPELINE
#else
	#define AL_GRAPHICS_USE_FIXED_PIPELINE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES1)
	#define AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_INT32
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_DOUBLE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_POLYGON_MODE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_POLYGON_SMOOTH
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES1)
	#define AL_GRAPHICS_SUPPORTS_STROKE_SMOOTH
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_SHADE_MODEL
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3)
	#define AL_GRAPHICS_SUPPORTS_SET_RW_BUFFERS
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_LR_BUFFERS
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_DEPTH_COMP
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_BLEND_EQ
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_SHADER
	#if defined(AL_GRAPHICS_USE_OPENGL)
		#define AL_GRAPHICS_SUPPORTS_GEOMETRY_SHADER
	#endif
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_STREAM_DRAW
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_DRAW_RANGE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_FBO
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_PBO
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_MAP_BUFFER
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_TEXTURE_1D
	#define AL_GRAPHICS_SUPPORTS_TEXTURE_3D
	#define AL_GRAPHICS_SUPPORTS_WRAP_EXTRA
#endif

#if !defined(AL_GRAPHICS_USE_OPENGLES2) && !defined(AL_GRAPHICS_USE_OPENGLES3)
	#define AL_GRAPHICS_TEXTURE_NEEDS_ENABLE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2)
	#define AL_GRAPHICS_SUPPORTS_MIPMAP
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_COLOR_MATERIAL_SPEC
#endif

static const char * glGetErrorString(bool verbose=true){
	auto err = glGetError();
	#define CS(GL_ERR, desc) case GL_ERR: return verbose ? #GL_ERR ", " desc : #GL_ERR;
	switch(err){
		case GL_NO_ERROR: return "";
		CS(GL_INVALID_ENUM, "An unacceptable value is specified for an enumerated argument.")
		CS(GL_INVALID_VALUE, "A numeric argument is out of range.")
		CS(GL_INVALID_OPERATION, "The specified operation is not allowed in the current state.")
	#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
		CS(GL_INVALID_FRAMEBUFFER_OPERATION, "The framebuffer object is not complete.")
	#endif
		CS(GL_OUT_OF_MEMORY, "There is not enough memory left to execute the command.")
	#ifdef GL_STACK_OVERFLOW
		CS(GL_STACK_OVERFLOW, "This command would cause a stack overflow.")
	#endif
	#ifdef GL_STACK_UNDERFLOW
		CS(GL_STACK_UNDERFLOW, "This command would cause a stack underflow.")
	#endif
	#ifdef GL_TABLE_TOO_LARGE
		CS(GL_TABLE_TOO_LARGE, "The specified table exceeds the implementation's maximum supported table size.")
	#endif
		default: return "Unknown error code.";
	}
	#undef CS
}

#endif /* include guard */
