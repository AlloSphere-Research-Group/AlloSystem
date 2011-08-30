#ifndef INC_AL_UTIL_FRAMEBUFFERGL_HPP
#define INC_AL_UTIL_FRAMEBUFFERGL_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
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


	File description:
	Render-to-texture utility

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/graphics/al_GraphicsOpenGL.hpp"
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
 */

namespace al {
	
class SimpleFBO : public TextureGL {
public:
	SimpleFBO(unsigned width=512, unsigned height=512)
	:	TextureGL(width, height),
		mFboId(0),
		mRboId(0),
		mClearColor(0)
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
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width(), height());
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
			} 
			
			// switch back to window-system-provided framebuffer
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			
			if (GraphicsGL::gl_error("SimpleFBO onCreate")) exit(0);
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
	}
	
	Color clearColor() const { return mClearColor; }
	Color& clearColor() { return mClearColor; }
	
	void clear() {
		glViewport(0, 0, width(), height());
		glClearColor(mClearColor.r, mClearColor.g, mClearColor.b, 1.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	void leave() {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		
		// trigger mipmap generation explicitly
		// NOTE: If GL_GENERATE_MIPMAP is set to GL_TRUE, then glCopyTexSubImage2D()
		// triggers mipmap generation automatically. However, the texture attached
		// onto a FBO should generate mipmaps manually via glGenerateMipmapEXT().
		bind();
		glGenerateMipmapEXT(GL_TEXTURE_2D);
		unbind();
		
		if (GraphicsGL::gl_error("SimpleFBO onLeave")) exit(0);
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
	
};
	
} // al::

#endif