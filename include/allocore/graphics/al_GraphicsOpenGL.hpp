#ifndef INCLUDE_AL_GRAPHICS_OPENGL_H
#define INCLUDE_AL_GRAPHICS_OPENGL_H

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "allocore/system/al_Config.h"

// OpenGL platform-dependent includes

#if defined(AL_OSX)
	#define AL_GRAPHICS_USE_OPENGL	
	#include <OpenGL/OpenGL.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>
	#include <OpenGL/glu.h>	
	#define AL_GRAPHICS_INIT_CONTEXT\
		/* prevents tearing */ \
		{	GLint MacHackVBL = 1;\
			CGLContextObj ctx = CGLGetCurrentContext();\
			CGLSetParameter(ctx,  kCGLCPSwapInterval, &MacHackVBL); }
			
#elif defined(AL_LINUX)
	#define AL_GRAPHICS_USE_OPENGL
	#include <GL/glew.h>
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
#elif defined(AL_WIN32)
	#define AL_GRAPHICS_USE_OPENGL
	
	#include <windows.h>
	#include <gl/gl.h>
	#include <gl/glu.h>
	#pragma comment( lib, "winmm.lib")
	#pragma comment( lib, "opengl32.lib" )
	#pragma comment( lib, "glu32.lib" )	
	#define AL_GRAPHICS_INIT_CONTEXT
	
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

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Surface.hpp"
#include "allocore/types/al_Array.hpp"

namespace al {

class GraphicsGL : public Graphics {
public:

	struct SurfaceData{
		SurfaceData()
		:	depth_id(0)
		{}
		
		~SurfaceData() {}
	
		int depth_id;
	};
	
	static bool gl_error(const char *msg = "");
	
	static GLenum gl_primitive(Graphics::Primitive v);
	static GLenum gl_antialias_mode(Graphics::AntiAliasMode v);
	static GLenum gl_matrix_mode(Graphics::MatrixMode v);
	static GLenum gl_blend_func(Graphics::BlendFunc v);
	static GLenum gl_polygon_mode(Graphics::PolygonMode v);
	static GLenum gl_capability(Graphics::Capability v);
	static GLenum type_for_array_type(AlloTy type);
	static GLenum target_from_texture_target(Texture::Target target);
	
	GraphicsGL();
	virtual ~GraphicsGL();

	// OpenGL specific:
	void enableLight(bool enable, int idx);

	// Textures
	virtual void textureCreate(Texture *tex);
	virtual void textureDestroy(Texture *tex);
	virtual void textureBind(Texture *tex, int unit);
	virtual void textureUnbind(Texture *tex, int unit);
	virtual void textureEnter(Texture *tex, int unit);
	virtual void textureLeave(Texture *tex, int unit);
	virtual void textureSubmit(Texture *tex);
	virtual void textureToArray(Texture *tex);
	
	// surfaces
	virtual Surface * surfaceNew();
	virtual void surfaceCreate(Surface *surface);
	virtual void surfaceDestroy(Surface *surface);
	virtual void surfaceBind(Surface *surface);
	virtual void surfaceUnbind(Surface *surface);
	virtual void surfaceEnter(Surface *surface);
	virtual void surfaceLeave(Surface *surface);
	virtual void surfaceClear(Surface *surface);
	virtual void surfaceCopy(Surface *surface, Texture *texture);

private:
	virtual void p_currentColor(double r, double g, double b, double a=1.);

	// Rendering State
	virtual void p_blending(bool enable, BlendFunc src, BlendFunc dst);
	virtual void p_enable(Capability cap);
	virtual void p_disable(Capability cap);

	// Frame
	virtual void p_clear(int attribMask);
	virtual void p_clearColor(float r, float g, float b, float a);
	
	// Coordinate Transforms
	virtual void p_viewport(int x, int y, int width, int height);
	virtual void p_matrixMode(MatrixMode mode);

	virtual void p_pushMatrix();
	virtual void p_popMatrix();
	virtual void p_loadIdentity();
	virtual void p_loadMatrix(const Matrix4d &m);
	virtual void p_multMatrix(const Matrix4d &m);
	virtual void p_translate(double x, double y, double z);
	virtual void p_rotate(double angle, double x, double y, double z);
	virtual void p_scale(double x, double y, double z);
	
	virtual void p_draw(const Mesh& v);

	virtual void p_antialiasing(AntiAliasMode v);
	virtual void p_lineWidth(double v);
	virtual void p_pointSize(double v);
	virtual void p_polygonMode(PolygonMode m, Face f);
	virtual void p_shadeModel(ShadeModel m);
	
	virtual void p_fog(float end, float start, const Color& c);
};

} // al::

#endif /* include guard */
