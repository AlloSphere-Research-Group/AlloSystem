#ifndef INC_AL_UTIL_FRAMEBUFFERGL_HPP
#define INC_AL_UTIL_FRAMEBUFFERGL_HPP

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
	Render-to-texture utility

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "alloutil/al_OpenGL.hpp"

/*
 Dealing with Framebuffers in OpenGL can be tricky. This object tries to make it easier.
 @see http://www.opengl.org/wiki/Framebuffer_Object
 
 Image: a 2D array of pixels with specific format (e.g. array, texture)
 Layered image: sequence of images of same size
 Texture: object with several images with same format (but not necessarily same size, e.g. mip-maps)
 Renderbuffer: object contains a single image. Can only be attached to FBOs.
 Framebuffer-attachable image: Any image that can be attached to a framebuffer object.
 Framebuffer-attachable layered image: Any layered image that can be attached to a framebuffer object.
 Attachment point: A named location within a framebuffer object that a framebuffer-attachable image or layered image can be attached to. Attachment points can have limitations on the format of the images attached there.
 Attach: To connect one object to another. This is not limited to FBOs, but attaching is a big part of them. Attachment is different from binding. Objects are bound to the context; they are attached to each other.
 
 MULTISAMPLING
 See http://stackoverflow.com/questions/765434/glreadpixels-from-fbo-fails-with-multisampling
 
 Looks like a multi-sampling FBO has to have a color renderbuffer rather than a texture... 'sophisticated'
 */

namespace al {
	
class SimpleFBO : public TextureGL {
public:
	SimpleFBO(unsigned width=512, unsigned height=512, unsigned multisampling=0)
	:	TextureGL(width, height),
		mFboId(0),
		mRboId(0),
		mClearColor(0),
		mMultiSamples(multisampling)
	{}
	
	virtual ~SimpleFBO() {}
	
	virtual void onCreate() {
		if (mID == 0) {
			// create a texture object
			TextureGL::onCreate();
				
			// create a framebuffer object, you need to delete them when program exits.
			glGenFramebuffersEXT(1, &mFboId);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
			
			// create a renderbuffer object to store depth info
			// NOTE: A depth renderable image should be attached the FBO for depth test.
			// If we don't attach a depth renderable image to the FBO, then
			// the rendering output will be corrupted because of missing depth test.
			// If you also need stencil test for your rendering, then you must
			// attach additional image to the stencil attachement point, too.
			glGenRenderbuffersEXT(1, &mRboId);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRboId);
			// or GL_DEPTH_COMPONENT24
			
			if (mMultiSamples) {
				glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, mMultiSamples, GL_DEPTH_COMPONENT24, width(), height());
			} else {
				glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width(), height());
			}
			//glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
			glEnable(GL_DEPTH_TEST);
			
			// attach a texture to FBO color attachement point
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, id(), 0);
			
			// attach a renderbuffer to depth attachment point
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRboId);
			
			// check FBO status
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
				printf("error creating FBO (%d)\n", status);
	
				switch (status) {
					#define CASE(enum) case enum: printf("error creating FBO (%d = %s)\n", status, #enum); break;
					CASE( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT )
					CASE( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT )
					CASE( GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT )
					CASE( GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT )
					CASE( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT )
					CASE( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT )
					CASE( GL_FRAMEBUFFER_UNSUPPORTED_EXT )
					#undef CASE
				}
				destroy();
			}
			
			// switch back to window-system-provided framebuffer
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			
			if (Graphics::error("SimpleFBO onCreate")) exit(0);
		}
	}
	
	void onDestroy() {
		if (mFboId) {
			glDeleteFramebuffersEXT(1, &mFboId);
			mFboId = 0;
		}
		if (mRboId) {
			glDeleteRenderbuffersEXT(1, &mRboId);
			mRboId = 0;
		}
		TextureGL::onDestroy();
	}
	
	// start rendering to FBO.
	// typically would want to call clear() right after this.
	void enter() {
		validate();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	}
	
	Color clearColor() const { return mClearColor; }
	Color& clearColor() { return mClearColor; }
	
	void clear() {
		glViewport(0, 0, width(), height());
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, width(), height());
		
		glClearColor(mClearColor.r, mClearColor.g, mClearColor.b, 1.);				glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	void leave() {
		glDisable(GL_SCISSOR_TEST);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		
		// trigger mipmap generation explicitly
		// NOTE: If GL_GENERATE_MIPMAP is set to GL_TRUE, then glCopyTexSubImage2D()
		// triggers mipmap generation automatically. However, the texture attached
		// onto a FBO should generate mipmaps manually via glGenerateMipmapEXT().
		bind();
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		unbind();
		
		if (Graphics::error("SimpleFBO onLeave")) exit(0);
	}
	
	// Note: this will reformat the dst array:
	void readPixels(Array& dst) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboId);
		dst.format(3, AlloUInt8Ty, width(), height());
		glReadPixels(0, 0, dst.width(), dst.height(), formatFromArray(dst), typeFromArray(dst), dst.data.ptr);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	
protected:	
	GLuint mFboId, mRboId;
	Color mClearColor;
	unsigned mMultiSamples;
	
};
	
} // al::

#endif