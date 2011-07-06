/*
 *  seq.cpp
 *  allo
 *
 *  Created by Graham Wakefield on 5/18/11.

A short example of real-time / non-real time rendering.

All events are time-stamped and executed in order.

RT mode:
	audio driver provides callbacks to synthesize audio.
	
NRT mode:
	audio driver is triggered manually, results are sent to a separate recorder to write to file.


TODO: 
	add flags to gam::Recorder to flag overflows (when > size samples are written but not read.)
	Or move Recorder implementation to a class here.
 */


#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shapes.hpp"

#include "allocore/graphics/al_GraphicsOpenGL.hpp"
#include "allocore/graphics/al_GPUObject.hpp"


#include "Gamma/Recorder.h"
#include "Gamma/SoundFile.h"

namespace al {

class SimpleFBO {
public:
	SimpleFBO(Graphics& gl, unsigned width, unsigned height)
	:	width(width),
		height(height)
		//tex(gl,width,height,Texture::RGBA,Texture::UCHAR,Texture::CLAMP)
	{
		//fbo = 0; depthbuffer = 0;
		//tex.target(Texture::TEXTURE_2D);

		textureId = rboId = fboId = 0;
	}

	GLuint textureId;
	GLuint rboId;
	GLuint fboId;
	unsigned width, height;

	void onCreate() {

//		glInfo glInfo;
//		glInfo.getInfo();
//		//glInfo.printSelf();
//
//		if(!glInfo.isExtensionSupported("GL_EXT_framebuffer_object")) {
//			printf("FBO not supported on this GPU\n");
//			exit(0);
//		}

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


		// attach a texture to FBO color attachement point
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);

        // attach a renderbuffer to depth attachment point
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);


		// check FBO status
		//printFramebufferInfo();
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("error creating FBO (%d)\n", status);
		}

		// switch back to window-system-provided framebuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		if (GraphicsGL::gl_error("SimpleFBO onCreate")) exit(0);
	}

	bool onResize(int w, int h){
//		printf("Resized FBO to %ix%i\n", w, h);

//		width = w; height = h;
//		tex.dimensions(w, h);
//		tex.target(Texture::TEXTURE_2D);

//		glEnable(GL_TEXTURE_2D);
//		glBindTexture(GL_TEXTURE_2D, tex.id());
//		glGenerateMipmap(GL_TEXTURE_2D);
//		glBindTexture(GL_TEXTURE_2D, 0);
//		glDisable(GL_TEXTURE_2D);

		return true;
	}

	void onEnter() {

		if (GraphicsGL::gl_error("SimpleFBO onEnter (pre)")) exit(0);

		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, width, height);


		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glPushAttrib(GL_VIEWPORT_BIT);
		//glViewport(0, 0, width, height);

		// set rendering destination to FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

		// clear buffers
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (GraphicsGL::gl_error("SimpleFBO onEnter (post)")) exit(0);

	}

	void onLeave() {
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// unbind FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		// GL_VIEWPORT_BIT
		glPopAttrib();

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

//	void printFramebufferInfo()
//	{
//		using namespace std;
//
//		cout << "\n***** FBO STATUS *****\n";
//
//		// print max # of colorbuffers supported by FBO
//		int colorBufferCount = 0;
//		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &colorBufferCount);
//		cout << "Max Number of Color Buffer Attachment Points: " << colorBufferCount << endl;
//
//		int objectType;
//		int objectId;
//
//		// print info of the colorbuffer attachable image
//		for(int i = 0; i < colorBufferCount; ++i)
//		{
//			glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//													 GL_COLOR_ATTACHMENT0_EXT+i,
//													 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
//													 &objectType);
//			if(objectType != GL_NONE)
//			{
//				glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//														 GL_COLOR_ATTACHMENT0_EXT+i,
//														 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
//														 &objectId);
//
//				std::string formatName;
//
//				cout << "Color Attachment " << i << ": ";
//				if(objectType == GL_TEXTURE)
//					cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
//				else if(objectType == GL_RENDERBUFFER_EXT)
//					cout << "GL_RENDERBUFFER_EXT, " << getRenderbufferParameters(objectId) << endl;
//			}
//		}
//
//		// print info of the depthbuffer attachable image
//		glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//												 GL_DEPTH_ATTACHMENT_EXT,
//												 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
//												 &objectType);
//		if(objectType != GL_NONE)
//		{
//			glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//													 GL_DEPTH_ATTACHMENT_EXT,
//													 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
//													 &objectId);
//
//			cout << "Depth Attachment: ";
//			switch(objectType)
//			{
//			case GL_TEXTURE:
//				cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
//				break;
//			case GL_RENDERBUFFER_EXT:
//				cout << "GL_RENDERBUFFER_EXT, " << getRenderbufferParameters(objectId) << endl;
//				break;
//			}
//		}
//
//		// print info of the stencilbuffer attachable image
//		glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//												 GL_STENCIL_ATTACHMENT_EXT,
//												 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
//												 &objectType);
//		if(objectType != GL_NONE)
//		{
//			glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER_EXT,
//													 GL_STENCIL_ATTACHMENT_EXT,
//													 GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
//													 &objectId);
//
//			cout << "Stencil Attachment: ";
//			switch(objectType)
//			{
//			case GL_TEXTURE:
//				cout << "GL_TEXTURE, " << getTextureParameters(objectId) << endl;
//				break;
//			case GL_RENDERBUFFER_EXT:
//				cout << "GL_RENDERBUFFER_EXT, " << getRenderbufferParameters(objectId) << endl;
//				break;
//			}
//		}
//
//		cout << endl;
//	}

};


// @see http://www.opengl.org/wiki/Renderbuffer_Objects
// multisampling==0 means no multisampling
class RenderBuffer : public GPUObject {
public:

	enum Component {
		DEPTH24,
		RGBA8
	};
	
	static GLenum toGLComponent(Component a) {
		switch (a) {
			case DEPTH24:
				return GL_DEPTH_COMPONENT24;
			default:	// DEPTH
				return GL_RGBA8;
		}
	}

	RenderBuffer(int width, int height, Component component=RGBA8, int multisampling=0) 
	:	GPUObject(),
		mWidth(width),
		mHeight(height),
		mMultiSample(multisampling),
		mComponent(component),
		mAllocated(false) {
		printf("RBO %ix%i\n", mWidth, mHeight);
	}
	virtual ~RenderBuffer() {}
	
	// bind to context:
	void bind() {
		validate();
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mID);
		// allocate storage if required:
		if (!mAllocated) {
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, mMultiSample, toGLComponent(mComponent), mWidth, mHeight);
			mAllocated = true;
		}
	}
	void unbind() {}
	
	// read pixels from render buffer:
	void readPixels(void * pixels) {
		GLenum format = GL_RGBA;		// format & type describe the layout of array
		GLenum type = GL_UNSIGNED_BYTE;
		glReadPixels(0, 0, mWidth, mHeight, format, type, pixels);
	}
	void readPixels(Array& pixels) {
		GLenum type = GraphicsGL::type_for_array_type(pixels.header.type);
		GLenum format;
		switch (pixels.header.components) {
			case 1:
				format = GL_LUMINANCE;
				break;
			case 2:
				format = GL_LUMINANCE_ALPHA;
				break;
			case 3:
				format = GL_RGB;
				break;
			default:
				format = GL_RGBA;
				break;
		}
		glReadPixels(0, 0, mWidth, mHeight, format, type, (GLvoid *)pixels.data.ptr);
	}
	
	void copyToTexture() {
		//glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
		// Once the data is written to the base level of the texture, call glGenerateMipmapEXT(GL_TEXTURE_2D) while the texture is bound to the GL_TEXTURE_2D target.
	}

protected: 	
	virtual void onCreate() {
		glGenRenderbuffersEXT(1, (GLuint *)&mID);
	};
	virtual void onDestroy() {
		glDeleteRenderbuffersEXT(1, (GLuint *)&mID);
	};
	
	int mWidth, mHeight;
	int mMultiSample;
	Component mComponent;
	bool mAllocated;
};

/*
	An FBO is a way to render to GPU memory other than the main output buffers.
	The FBO object itself does not provide the memory, instead either Texture or RenderBuffer objects can be attached to the FBO at different attachment points, such as COLOR0 and DEPTH.
*/
class FrameBuffer : public GPUObject {
public:
	enum Attachment {
		DEPTH,
		COLOR0,
	};
	
	static GLenum toGLAttachment(Attachment a) {
		switch (a) {
			case COLOR0:
				return GL_COLOR_ATTACHMENT0_EXT;
			default:	// DEPTH
				return GL_DEPTH_ATTACHMENT_EXT;
		}
	}

	FrameBuffer() : GPUObject() {}
	
	/*
		When an FBO is bound to a target, the available surfaces change. 
		FBOs do not have buffers like GL_FRONT, GL_BACK, GL_AUXi, GL_ACCUM etc.
	*/
	void bind() {
		validate();
		// set read & write to same FBO using GL_FRAMEBUFFER
		// alternatives could be GL_READ_FRAMEBUFFER or GL_DRAW_FRAMEBUFFER
		// to allow reading from one FBO (glCopyPixels) and writing to another (glDraw*)
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mID);
	}
	
	bool verify() {
		GLenum status;
		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		
		switch (status) {
			case GL_FRAMEBUFFER_COMPLETE_EXT:
				return true;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
				return false;
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
				printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n");
				return false;
			default:
				printf("FRAMEBUFFER INCOMPLETE %x\n", status);
				return false;
		}
		if(status==GL_FRAMEBUFFER_COMPLETE_EXT) {
			 return true;
		} else {
			printf("FrameBuffer incomplete %x\n", status);
		}

		GraphicsGL::gl_error("FrameBuffer incomplete");
		return false;
	}
	
	void unbind() {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	
	/*
		Bound FBOs have several possible attachment points, including GL_COLOR_ATTACHMENT0 and GL_DEPTH_ATTACHMENT
		Both Textures and RenderBuffers can be attached to these points.
		RBOs may be faster than Textures, but do not provide mipmapping capabilities.
		
		Textures would normally use target GL_TEXTURE_2D
	*/
	void attachRenderBufferToComponent(RenderBuffer& rbo, Attachment attachment=DEPTH) {
		rbo.bind();
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, toGLAttachment(attachment), GL_RENDERBUFFER_EXT, rbo.id());
	}
	void detachRenderBufferFromComponent(RenderBuffer& rbo, Attachment attachment=DEPTH) {
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, toGLAttachment(attachment), GL_RENDERBUFFER_EXT, 0);
	}
	
	void attachTextureToComponent(Texture& tex, Attachment attachment=COLOR0) {
		GLint mipmap_level = 0;	// TODO.
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, toGLAttachment(attachment), GraphicsGL::target_from_texture_target(tex.target()), tex.id(), mipmap_level);
	}
	void detachTextureFromComponent(Texture& tex, Attachment attachment=COLOR0) {
		GLint mipmap_level = 0;	// TODO.
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, toGLAttachment(attachment), GL_TEXTURE_2D, 0, mipmap_level);
	}
	
protected: 	
	virtual void onCreate() {
		glGenFramebuffersEXT(1, (GLuint *)&mID);
	};
	virtual void onDestroy() {
		unbind();
		glDeleteFramebuffersEXT(1, (GLuint *)&mID);
	};
};

} // al::

using namespace al;

GraphicsGL gl;
Window win;

class TestScene : public Drawable, public WindowEventHandler {
public:
	TestScene(int width, int height) 
	:	width(width),
		height(height),
		rboDepth(width, height, RenderBuffer::DEPTH24, 0),
		rboRGB(width, height, RenderBuffer::RGBA8, 0),
		tex(gl, width, height, Texture::RGBA, Texture::UCHAR),
		arrRGB(4, AlloUInt8Ty, width, height),
		arrDepth(1, AlloUInt8Ty, width, height),
		sfbo(gl, width, height)
	{
		mesh.primitive(Graphics::TRIANGLES);
		addSphere(mesh);
		mesh.decompress();
		mesh.generateNormals();
		
		tex.target(Texture::TEXTURE_2D);
		
		frame = 0;
	}
	virtual ~TestScene() {}
	
	virtual bool onCreate(){ 
		// FBOs can attach 'images', either Renderbuffer objects or Textures
		// Texture has advantage of immediate re-use
		// RBO may be more efficient
		printf("create fbo at %ix%i\n", width, height);
		
		// create RBO:
		glGenRenderbuffersEXT(1, &rbo_id);
		// bind to use it:
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo_id);
		// create storage:
		GLenum internalFormat = GL_DEPTH_COMPONENT24;	
		// max size: GL_MAX_RENDERBUFFER_SIZE_EXT
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, width, height);
		// using multisampling:
		//int multisamples = 1; // 0 means no multisampling
		//glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, multisamples, internalFormat, width, height);
		// get info:
		GLint value;
//		GL_RENDERBUFFER_WIDTH_EXT
//		GL_RENDERBUFFER_HEIGHT_EXT
//		GL_RENDERBUFFER_INTERNAL_FORMAT_EXT
//		GL_RENDERBUFFER_RED_SIZE_EXT
//		GL_RENDERBUFFER_GREEN_SIZE_EXT
//		GL_RENDERBUFFER_BLUE_SIZE_EXT
//		GL_RENDERBUFFER_ALPHA_SIZE_EXT
//		GL_RENDERBUFFER_DEPTH_SIZE_EXT
//		GL_RENDERBUFFER_STENCIL_SIZE_EXT
		glGetRenderbufferParameterivEXT(GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &value);
		printf("depth size %i\n", value);
		// done setting up RBO:
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		
		// create RBO:
		glGenRenderbuffersEXT(1, &rborgb_id);
		// bind to use it:
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rborgb_id);
		// create storage:
		internalFormat = GL_RGBA;	
		// max size: GL_MAX_RENDERBUFFER_SIZE_EXT
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, width, height);
		// using multisampling:
		//int multisamples = 1; // 0 means no multisampling
		//glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, multisamples, internalFormat, width, height);
		// done setting up RBO:
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		
		// create Texture:
		glGenTextures(1, &tex_id);
		// bind to use:
		glBindTexture(GL_TEXTURE_2D, tex_id);
		// setup parameters:
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		// done with texture:
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// create Texture:
		glGenTextures(1, &texDepth_id);
		// bind to use:
		glBindTexture(GL_TEXTURE_2D, texDepth_id);
		// setup parameters:
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		// done with texture:
		glBindTexture(GL_TEXTURE_2D, 0);
		
		// create fbo
		glGenFramebuffersEXT(1, &fbo_id);
		// bind it to use it
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
		// attach texture to color attachment point:
		//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, tex_id, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, rborgb_id);
		// attach rbo to depth attachment point:
		//glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbo_id);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, texDepth_id, 0);
		
		// check FBO status
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			printf("FBO invalid %i\n", status);
		}
		// unbind fbo after setup:
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	
		GraphicsGL::gl_error("onCreate");
			
		//sfbo.onCreate();
		return true; 
	}	
	
	virtual bool onDestroy(){ 
		glDeleteTextures(1, &tex_id);
		glDeleteRenderbuffersEXT(1, &rbo_id);
		glDeleteFramebuffersEXT(1, &fbo_id);
		return true; 
	}
	
	virtual void onDraw(Graphics& gl) {
		light();
		material();
		gl.depthTesting(true);
		gl.blending(false);
		for (int i=1; i<10; i++) {
			gl.pushMatrix();
			gl.translate(i*sin(i), i*cos(i), i*10);
			gl.rotate(frame, 0., 1., 0.);
			gl.scale(i);
			gl.draw(mesh);
			gl.popMatrix();
		}
	}
	
	virtual bool onFrame() {
		if ((++frame) % 30 > 15) {
		
			// bind fbo:
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_id);
			// clear buffers
			glClearColor(0, 0, 1, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glPushAttrib(GL_VIEWPORT_BIT);
			glViewport(0, 0, width, height);

			gl.matrixMode(gl.PROJECTION);
			gl.loadMatrix(Matrix4d::perspective(45, win.width()/(float)win.height(), 0.1, 100));
			gl.matrixMode(gl.MODELVIEW);
			gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-10), Vec3d(0,0,0), Vec3d(0,1,0)));
		
		
			// draw scene
			onDraw(gl);
			
			// now copy pixels from renderbuffer:
			glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, arrRGB.data.ptr);
			
			glPopAttrib();// viewport
			// done with fbo:
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			
			// copy pixels to texture:
			glBindTexture(GL_TEXTURE_2D, tex_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, arrRGB.data.ptr);
			
			// trigger generation of mipmaps for texture:
			glGenerateMipmapEXT(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			GraphicsGL::gl_error("after fbo");
			
			// now show the texture:
			glClearColor(0, 1, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, win.width(), win.height());
			
			gl.matrixMode(gl.PROJECTION);
			gl.loadMatrix(Matrix4d::ortho(0, 1, 0, 1, -100, 100));
			gl.matrixMode(gl.MODELVIEW);
			gl.loadIdentity();
			
//			gl.matrixMode(gl.PROJECTION);
//			gl.loadMatrix(Matrix4d::perspective(45, 720./480., 0.1, 100));
//			gl.matrixMode(gl.MODELVIEW);
//			gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-3), Vec3d(0,0,0), Vec3d(0,1,0)));
			
			gl.lighting(false);
			gl.depthTesting(false);
			glEnable(GL_TEXTURE_2D);
			//glBindTexture(GL_TEXTURE_2D, tex_id);
			glBindTexture(GL_TEXTURE_2D, texDepth_id);
			gl.color(1,1,1);
			gl.begin(gl.QUADS);
				gl.texCoord(0, 0);
				gl.vertex(0, 0, 0);
				gl.texCoord(0, 1);
				gl.vertex(0, 1, 0);
				gl.texCoord(1, 1);
				gl.vertex(1, 1, 0);
				gl.texCoord(1, 0);
				gl.vertex(1, 0, 0);
			gl.end();
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		} else {
			glClearColor(0, 0, 1, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport(0, 0, win.width(), win.height());
			
			gl.matrixMode(gl.PROJECTION);
			gl.loadMatrix(Matrix4d::perspective(45, win.width()/(float)win.height(), 0.1, 100));
			gl.matrixMode(gl.MODELVIEW);
			gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-10), Vec3d(0,0,0), Vec3d(0,1,0)));
		
			onDraw(gl);
		}

			
//		fbo.bind();
//		//fbo.attachTextureToComponent(tex, FrameBuffer::COLOR0);
//		fbo.attachRenderBufferToComponent(rboRGB, FrameBuffer::COLOR0);
//		fbo.attachRenderBufferToComponent(rboDepth, FrameBuffer::DEPTH);
//		fbo.verify();
//		
//		gl.clearColor(0,0,0,0);
//		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
//		gl.viewport(0., 0., win.width(), win.height());	// TODO: pass w/h here
//		
//		sfbo.onEnter();
//		onDraw(gl);
//		sfbo.onLeave();
//		
//		sfbo.draw(gl);
//		
//		fbo.unbind();
//		
//		rboRGB.readPixels(arrRGB);
//		rboDepth.readPixels(arrDepth);
//		
//		tex.validate();
//		
//		// submit to texture:
//		//tex.fromArray(&arrRGB);
//		tex.fromArray(&arrDepth);
//		tex.target(Texture::TEXTURE_2D);
//		
//		// now need to pull the data from the rbo into a texture, bind it, and draw it
//		gl.clearColor(0,0,1,0);
//		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
//		gl.viewport(0., 0., win.width(), win.height());
//		
//		gl.matrixMode(gl.PROJECTION);
//		gl.loadMatrix(Matrix4d::ortho(0, 1, 0, 1, -100, 100));
//
//		gl.matrixMode(gl.MODELVIEW);
//		gl.loadIdentity();
//
//		gl.lighting(false);
//		gl.blending(false);
//
//		tex.bind();
//		glGenerateMipmapEXT(GL_TEXTURE_2D);
//		gl.begin(gl.QUADS);
//			gl.texCoord(0, 0);
//			gl.vertex(0, 0, 0);
//			gl.texCoord(0, 1);
//			gl.vertex(0, 1, 0);
//			gl.texCoord(1, 1);
//			gl.vertex(1, 0.9, 0);
//			gl.texCoord(1, 0);
//			gl.vertex(1, 0, 0);
//		gl.end();
//		tex.unbind();
		return true;
	} 
	
	int width, height;
	Light light;
	Material material;
	Mesh mesh;
	Stereographic stereo;
	int frame;
	
	RenderBuffer rboDepth, rboRGB;
	FrameBuffer fbo;
	Texture tex;
	Array arrRGB, arrDepth;
	
	SimpleFBO sfbo;
	
	GLuint fbo_id, rbo_id, rborgb_id, tex_id, texDepth_id;
};

TestScene testscene(720,480);

// unsigned (and unsigned long on a 32-bit compiler), at 44.1kHz, can count about 27 hours worth of samples.
// unsigned long long however can count 13264000 years.
typedef unsigned long long sampletime;

/*
	Attaches to an AudioIO callback.
	When start() has been called, 
		begins capturing the out buffers into an internal ringbuffer
		and a background thread process writes this ringbuffer to disk
	Calling stop() or destroying the AudioCapture object will finish writing the file.
*/
// currently fixed to float32 output format
class AudioCapture : public AudioCallback {
public:

	AudioCapture(AudioIO * host, std::string fileName, int channels=2, double sampleRate=44100, int bufferSize=8192, double sleep = 0.01) 
	:	mAudioIO(host),
		mFileName(fileName),
		mChans(channels),
		mSR(sampleRate),
		mFrames(bufferSize),
		mReadIndex(0), 
		mWriteIndex(0),
		mSleep(sleep),
		mOverflows(0),
		mUnderflows(0),
		mRecording(false)
	{
		mRing.resize(mFrames*mChans);
		host->append(this);
	}
	
	virtual ~AudioCapture() {
		mAudioIO->remove(this);
		stop();
	}
	
	void writeToOpenSoundFile(gam::SoundFile& sf) {
		// cached, because it may be being used in a different thread
		sampletime r = mReadIndex;
		sampletime w = mWriteIndex;
		
		unsigned length = mRing.size();
		
		// how much is there to write?
		sampletime ahead = w - r;	// how much writer is ahead of reader
		if (ahead > length) {
			// buffer is over-full!
			printf("overflow\n");
			mOverflows++;
			
			ahead = length;
			r = w - length;
		}
		
		// copy this many samples into the soundfile:
		if (ahead > 0) {
			unsigned written = 0;
			// copy in two passes, to handle ringbuffer boundary.
			unsigned ru = (unsigned)(r % length);
			unsigned wu = (unsigned)(w % length);
			
			if (ru > wu) {
                // read to end of ring buffer, then let the next block capture the remainder
                int frames = (length - ru)/mChans;
                int copied = mChans * sf.write(&mRing[ru], frames);
                ru = (ru + copied) % length;
				written += copied;
            }				
            if (ru < wu) {
                // let read head catch up to write head:
                int frames = (wu - ru)/mChans;
                int copied = mChans * sf.write(&mRing[ru], frames);
                ru = (ru + copied) % length;
				written += copied;
            }
			
			// update read index:
			mReadIndex = r + written;
		}
	}
	
	virtual void onAudioCB(AudioIOData& io) {
		if (mRecording) {
			// cached, because it may be being used in a different thread
			sampletime r = mReadIndex;
			sampletime w = mWriteIndex;
			
			unsigned channels = io.channelsOut();
			if (channels > mChans) channels = mChans;
			
			unsigned numFrames = io.framesPerBuffer();
			unsigned numSamples = numFrames * channels;
			unsigned length = mRing.size();
			
			sampletime ahead = w - r;	// how much writer is ahead of reader
			if (ahead > (length - numSamples)) {
				// not enough space left in ringbuffer!
				printf("underflow\n");
				mUnderflows++;
				return;
			}
			
			sampletime wnext = w + numSamples;
			unsigned wu = (unsigned)(w % length);
			
			//printf("writing %u samples to %llu\n", numSamples, wnext);
			
			for (unsigned c=0; c < channels; c++) {
				float * src = io.outBuffer(c);
				float * dst = &mRing[0];
				
				unsigned wc = wu + c;
				unsigned srcIdx = 0;
				
				while (srcIdx < numFrames) {
					dst[wc] = src[srcIdx];
					srcIdx++;
					wc += mChans;
					if (wc >= length) wc -= length;
				}
			}
		
			// update write head position
			mWriteIndex = wnext;
		}
	}
		
	void start() {
		if (!mRecording) {
			mRecording = true;
			mThread.start(audioRecordingThreadFunc, this);
		}
	}
	
	// waits for thread to stop.
	void stop() {
		if (mRecording) {
			mRecording = false;
			mThread.join();
		}
	}
	
	static void * audioRecordingThreadFunc(void * userData) {
		AudioCapture * self = (AudioCapture *)userData;
		if (self) {
			return (void *)self->recordingThreadFunc();
		}
		return NULL;
	}
	
	int recordingThreadFunc() {
		printf("start recording to %s\n", mFileName.c_str());
		// create & open soundfile:
		gam::SoundFile sf(mFileName);
		sf.format(gam::SoundFile::WAV);
		sf.encoding(gam::SoundFile::FLOAT);
		sf.channels(mChans);
		sf.frameRate(mSR);
		sf.openWrite();
		
		printf("started recording\n");
		while (mRecording) {		
			writeToOpenSoundFile(sf);
			al_sleep(mSleep);
		}
        
        // write any remaining samples!
        writeToOpenSoundFile(sf);
		sf.close();
		printf("finished recording; with %u overflows\n", mOverflows);
		return 0;
	}
	
	AudioIO * mAudioIO;
	std::string mFileName;
	unsigned mChans;
	double mSR;
	unsigned mFrames;
	sampletime mReadIndex, mWriteIndex;
	double mSleep;
	unsigned mOverflows, mUnderflows;
	bool mRecording;
	Thread mThread;
	
	std::vector<float> mRing;
};

void audioCB(AudioIOData& io){
	while(io()){
		float s0 = io.in(0);
		float s1 = io.in(1);
		
		//io.out(0) = s0;
		//io.out(1) = s1;
	}
}

int main (int argc, char * argv[]){
	
	SearchPaths paths(argc, argv);
	
	printf("outpath %s\n", paths.appPath().c_str());
	
	win.add(new StandardWindowKeyControls);
	win.add((WindowEventHandler *)&testscene);
	win.create(Window::Dim(720, 480));
	
	
	
	std::string audioFileName = paths.appPath() + "recording.wav";
	
	AudioIO audioIO(256, 44100, audioCB, NULL, 2, 2);
	AudioCapture capture(&audioIO, audioFileName, 2, 44100);
	
	
	//capture.start();
	audioIO.start();
	
	MainLoop::start();
	
	return 0;
}
