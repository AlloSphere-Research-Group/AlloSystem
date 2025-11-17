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

/* Optional preprocessor constants:

AL_GRAPHICS_USE_OPENGL_EXT
Include OpenGL extensions for target flavor of OpenGL. Not included by default to speed up compilation.
*/

#if !(     defined(AL_GRAPHICS_USE_DEFAULT_BACKEND)\
		|| defined(AL_GRAPHICS_USE_OPENGL)\
		|| defined(AL_GRAPHICS_USE_OPENGLES1)\
		|| defined(AL_GRAPHICS_USE_OPENGLES2)\
		|| defined(AL_GRAPHICS_USE_OPENGLES3)\
		|| defined(AL_GRAPHICS_USE_OPENGLES3_1)\
		|| defined(AL_GRAPHICS_USE_OPENGLES3_2)\
	)
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
		#ifdef AL_GRAPHICS_USE_OPENGL_EXT
			#include <OpenGL/glext.h>
		#endif
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
		#ifdef AL_GRAPHICS_USE_OPENGL_EXT
			#include <GL/glext.h>
		#endif
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
	#if   defined AL_GRAPHICS_USE_OPENGLES3_2
		#include <GLES3/gl32.h>
	#elif defined AL_GRAPHICS_USE_OPENGLES3_1
		#include <GLES3/gl31.h>
	#elif defined AL_GRAPHICS_USE_OPENGLES3
		#include <GLES3/gl3.h>
	#elif defined AL_GRAPHICS_USE_OPENGLES2
		#include <GLES2/gl2.h>
	#elif defined AL_GRAPHICS_USE_OPENGLES1
		#include <GLES/gl.h>
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

	#ifdef AL_GRAPHICS_USE_OPENGL_EXT
		//#define GL_GLEXT_PROTOTYPES 1
		#ifdef AL_GRAPHICS_USE_OPENGLES1
			#include <GLES/glext.h>
		#else
			#include <GLES2/gl2ext.h> // yes, ES2 version for ES2 *and* ES3
		#endif
	#endif

	#define AL_GRAPHICS_INIT_CONTEXT

#elif defined __IPHONE_2_0
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGLES1
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGLES1
		#import <OpenGLES/ES1/gl.h>
		#ifdef AL_GRAPHICS_USE_OPENGL_EXT
			#import <OpenGLES/ES1/glext.h>
		#endif
	#else
		#error "Specified graphics backend not supported on this platform"
	#endif

#elif defined __IPHONE_3_0
	#ifdef AL_GRAPHICS_USE_DEFAULT_BACKEND
		#define AL_GRAPHICS_USE_OPENGLES2
	#endif
	#ifdef AL_GRAPHICS_USE_OPENGLES2
		#import <OpenGLES/ES2/gl.h>
		#ifdef AL_GRAPHICS_USE_OPENGL_EXT
			#import <OpenGLES/ES2/glext.h>
		#endif
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

#if defined(AL_GRAPHICS_USE_OPENGLES3) || defined(AL_GRAPHICS_USE_OPENGLES3_1) || defined(AL_GRAPHICS_USE_OPENGLES3_2)
	// Any GLES version
	#define AL_GRAPHICS_USE_OPENGLES3_X
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_PROG_PIPELINE
	#define AL_GRAPHICS_SUPPORTS_SHADER
	#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3_2)
		#define AL_GRAPHICS_SUPPORTS_GEOMETRY_SHADER
	#endif
	#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3_1) || defined(AL_GRAPHICS_USE_OPENGLES3_2)
		#define AL_GRAPHICS_SUPPORTS_COMPUTE_SHADER
	#endif
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES1)
	#define AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
#endif

#ifdef AL_GRAPHICS_SUPPORTS_FIXED_PIPELINE
	#define AL_GRAPHICS_USE_FIXED_PIPELINE
#else
	#define AL_GRAPHICS_USE_PROG_PIPELINE
#endif

// GL_INT and GL_UNSIGNED support as used with, e.g., glTexSubImage2D
#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
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

#if defined(AL_GRAPHICS_USE_OPENGL)
	//see glDrawBuffer
	#define AL_GRAPHICS_SUPPORTS_SET_W_BUFFER
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	//see glReadBuffer
	#define AL_GRAPHICS_SUPPORTS_SET_R_BUFFER
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_LR_BUFFERS
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_DEPTH_COMP
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_DEPTH_COMP24
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_DEPTH_COMP16
#endif

#ifndef AL_GRAPHICS_USE_OPENGLES1
	#define AL_GRAPHICS_SUPPORTS_BLEND_EQ
#endif

#ifndef AL_GRAPHICS_USE_OPENGLES1
	#define AL_GRAPHICS_SUPPORTS_STREAM_DRAW
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_DRAW_RANGE
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES2) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_FBO
	#ifndef AL_GRAPHICS_USE_OPENGLES2
		//see glBindFramebuffer
		#define AL_GRAPHICS_SUPPORTS_FBO_RW_BIND
	#endif
#endif

#if defined(AL_GRAPHICS_USE_OPENGL) || defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_PBO
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_MAP_BUFFER
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_TEXTURE_1D
	#define AL_GRAPHICS_SUPPORTS_TEXTURE_3D
	//see glTexParameteri(?,GL_TEXTURE_WRAP_S,___)
	#define AL_GRAPHICS_SUPPORTS_WRAP_CLAMP_EXTRA // CLAMP, CLAMP_TO_BORDER
#endif

#if defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_SUPPORTS_TEXTURE_3D
#endif

#ifndef AL_GRAPHICS_USE_OPENGLES1
	#define AL_GRAPHICS_SUPPORTS_WRAP_REPEAT_EXTRA // MIRRORED_REPEAT
#endif

#if !defined(AL_GRAPHICS_USE_OPENGLES2) && !defined(AL_GRAPHICS_USE_OPENGLES3_X)
	#define AL_GRAPHICS_TEXTURE_NEEDS_ENABLE
#endif

#ifndef AL_GRAPHICS_USE_OPENGLES1
	//see glTexParameteri(?,GL_TEXTURE_MIN_FILTER,___)
	#define AL_GRAPHICS_SUPPORTS_MIPMAP
#endif

#if defined(AL_GRAPHICS_USE_OPENGL)
	#define AL_GRAPHICS_SUPPORTS_GET_TEX_IMAGE
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
