#ifndef INC_AL_UTIL_FRAMEBUFFERGL_HPP
#define INC_AL_UTIL_FRAMEBUFFERGL_HPP

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
	
	class SimpleFBO {
	public:
		SimpleFBO(unsigned width=512, unsigned height=512)
		:	width(width),
			height(height)
		{
			//fbo = 0; depthbuffer = 0;
			//tex.target(Texture::TEXTURE_2D);
			
			textureId = rboId = fboId = 0;
			
			mArray.format(4, AlloUInt8Ty, width, height);
		}
		
		GLuint textureId;
		//TextureGL texture;
		GLuint rboId;
		GLuint fboId;
		unsigned width, height;
		Color bg;
		
		// array into which texture will be copied:
		Array mArray;	
		
		void reset(int w, int h) {
			onDestroy();
			width = w; height = h;
			mArray.format(4, AlloUInt8Ty, width, height);
			onCreate();
		}
		
		// retrieve internal array:
		al::Array& array() { return mArray; }
		// raw access:
		char * data() { return mArray.data.ptr; }
		
		void onCreate() {
			
			
//			glInfo glInfo;
//			glInfo.getInfo();
//			//glInfo.printSelf();
//			
//			if(!glInfo.isExtensionSupported("GL_EXT_framebuffer_object")) {
//				printf("FBO not supported on this GPU\n");
//				exit(0);
//			}
			
			// create a texture object
			glGenTextures(1, &textureId);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			// create a framebuffer object, you need to delete them when program exits.
			glGenFramebuffersEXT(1, &fboId);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
			
			// create a renderbuffer object to store depth info
			// NOTE: A depth renderable image should be attached the FBO for depth test.
			// If we don't attach a depth renderable image to the FBO, then
			// the rendering output will be corrupted because of missing depth test.
			// If you also need stencil test for your rendering, then you must
			// attach additional image to the stencil attachement point, too.
			glGenRenderbuffersEXT(1, &rboId);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rboId);
			// or GL_DEPTH_COMPONENT24
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height);
			//glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
			glEnable(GL_DEPTH_TEST);
			
			// attach a texture to FBO color attachement point
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);
			
			// attach a renderbuffer to depth attachment point
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);
			
			
			// check FBO status
			//printFramebufferInfo();
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
				printf("error creating FBO (%d)\n", status);
			} else {
				printf("created FBO\n");
			}

			
			// switch back to window-system-provided framebuffer
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			
			if (GraphicsGL::gl_error("SimpleFBO onCreate")) exit(0);
		}
		
		void onDestroy() {
			if (fboId) {
				glDeleteFramebuffersEXT(1, &textureId);
				fboId = 0;
			}
			if (textureId) {
				glDeleteTextures(1, &textureId);
				textureId = 0;
			}
			if (rboId) {
				glDeleteRenderbuffersEXT(1, &textureId);
				rboId = 0;
			}
		}
		
		void onEnter() {
			
			if (GraphicsGL::gl_error("SimpleFBO onEnter (pre)")) exit(0);
			
			// set rendering destination to FBO
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
			
//			glPushAttrib(GL_VIEWPORT_BIT);
//			glViewport(0, 0, width, height);
			
			//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			//glPushAttrib(GL_VIEWPORT_BIT);
			//glViewport(0, 0, width, height);
			
//			// clear buffers
//			glClearColor(bg.r, bg.g, bg.b, 1.);
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			if (GraphicsGL::gl_error("SimpleFBO onEnter (post)")) exit(0);
			
		}
		
		void onLeave() {
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glBindRenderbuffer(GL_RENDERBUFFER, 0);
			
//			// GL_VIEWPORT_BIT
//			glPopAttrib();

			// unbind FBO
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			
			// trigger mipmaps generation explicitly
			// NOTE: If GL_GENERATE_MIPMAP is set to GL_TRUE, then glCopyTexSubImage2D()
			// triggers mipmap generation automatically. However, the texture attached
			// onto a FBO should generate mipmaps manually via glGenerateMipmapEXT().
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureId);
			glGenerateMipmapEXT(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			
			if (GraphicsGL::gl_error("SimpleFBO onLeave")) exit(0);
			
		}
		
		// Note: this will reformat the array:
		void readPixels(Array& dst) {
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
			dst.format(3, AlloUInt8Ty, width, height);
			glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, dst.data.ptr);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		}
		
		void draw(Graphics& gl) {
			gl.matrixMode(gl.PROJECTION);
			gl.loadMatrix(Matrix4d::ortho(0, 1, 0, 1, -100, 100));
			
			gl.matrixMode(gl.MODELVIEW);
			gl.loadIdentity();
			
			gl.lighting(false);
			gl.blending(false);
			
			//tex.bind();
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureId);
			gl.color(1, 1, 1, 1);
			gl.begin(gl.QUADS);
			gl.texCoord	( 0, 0);
			gl.vertex	( 0, 0, 0);
			gl.texCoord	( 0, 1);
			gl.vertex	( 0, 1, 0);
			gl.texCoord	( 1, 1);
			gl.vertex	( 1, 1, 0);
			gl.texCoord	( 1, 0);
			gl.vertex	( 1, 0, 0);
			gl.end();
			//tex.unbind();
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
		
//		void printFramebufferInfo()
//		{
//			using namespace std;
//			
//			cout << "\n***** FBO STATUS *****\n";
//			
//			// print max # of colorbuffers supported by FBO
//			int colorBufferCount = 0;
//			glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &colorBufferCount);
//			cout << "Max Number of Color Buffer Attachment Points: " << colorBufferCount << endl;
//			
//			int objectType;
//			int objectId;
//			
//			// print info of the colorbuffer attachable image
//			for(int i = 0; i < colorBufferCount; ++i)
//			{
//				glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//														 GL_COLOR_ATTACHMENT0_EXT+i,
//														 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
//														 &objectType);
//				if(objectType != GL_NONE)
//				{
//					glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//															 GL_COLOR_ATTACHMENT0_EXT+i,
//															 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
//															 &objectId);
//					
//					std::string formatName;
//					
//					cout << "Color Attachment " << i << ": ";
//					if(objectType == GL_TEXTURE)
//						cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
//					else if(objectType == GL_RENDERBUFFER_EXT)
//						cout << "GL_RENDERBUFFER_EXT, " << getRenderbufferParameters(objectId) << endl;
//				}
//			}
//			
//			// print info of the depthbuffer attachable image
//			glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//													 GL_DEPTH_ATTACHMENT_EXT,
//													 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
//													 &objectType);
//			if(objectType != GL_NONE)
//			{
//				glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//														 GL_DEPTH_ATTACHMENT_EXT,
//														 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
//														 &objectId);
//				
//				cout << "Depth Attachment: ";
//				switch(objectType)
//				{
//					case GL_TEXTURE:
//						cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
//						break;
//					case GL_RENDERBUFFER_EXT:
//						cout << "GL_RENDERBUFFER_EXT, " << getRenderbufferParameters(objectId) << endl;
//						break;
//				}
//			}
//			
//			// print info of the stencilbuffer attachable image
//			glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//													 GL_STENCIL_ATTACHMENT_EXT,
//													 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
//													 &objectType);
//			if(objectType != GL_NONE)
//			{
//				glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//														 GL_STENCIL_ATTACHMENT_EXT,
//														 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
//														 &objectId);
//				
//				cout << "Stencil Attachment: ";
//				switch(objectType)
//				{
//					case GL_TEXTURE:
//						cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
//						break;
//					case GL_RENDERBUFFER_EXT:
//						cout << "GL_RENDERBUFFER_EXT, " << getRenderbufferParameters(objectId) << endl;
//						break;
//				}
//			}
//			
//			cout << endl;
//		}
		
	};
	
	
//class RenderBuffer : public GPUObject {
//public:
//	
//protected: 	
//	virtual void onCreate() {
//		glGenRenderbuffers(1, (GLuint *)&mID);
//	};
//	virtual void onDestroy() {
//		printf("destroy fbo\n");
//		glDeleteRenderbuffers(1, (GLuint *)&mID);
//	};
//};
//
//class FrameBuffer : public GPUObject {
//public:
//	FrameBuffer() : GPUObject() {}
//	
//	/*
//	 When an FBO is bound to a target, the available surfaces change. 
//	 FBOs do not have buffers like GL_FRONT, GL_BACK, GL_AUXi, GL_ACCUM etc.
//	 */
//	void bind() {
//		// set read & write to same FBO using GL_FRAMEBUFFER
//		// alternatives could be GL_READ_FRAMEBUFFER or GL_DRAW_FRAMEBUFFER
//		// to allow reading from one FBO (glCopyPixels) and writing to another (glDraw*)
//		glBindFramebuffer(GL_FRAMEBUFFER, mID);
//	}
//	
//	void unbind() {
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	}
//	
//	/*
//	 Bound FBOs have several possible attachment points, including GL_COLOR_ATTACHMENT0 and GL_DEPTH_ATTACHMENT
//	 */
//	void attach(Texture& tex) {
//		
//	}
//	
//protected: 	
//	virtual void onCreate() {
//		printf("create fbo\n");
//		glGenFramebuffers(1, (GLuint *)&mID);
//		bind();
//	};
//	virtual void onDestroy() {
//		printf("destroy fbo\n");
//		glDeleteFramebuffers(1, (GLuint *)&mID);
//	};
//};
	
} // al::

#endif