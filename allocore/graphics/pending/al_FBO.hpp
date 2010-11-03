#ifndef GLW_FBO_H_INC
#define GLW_FBO_H_INC

#include "GPUObject.h"

namespace glw{


/// Render buffer object
class RBO : public GPUObject{
public:

	enum PixelFormat{
		Depth = GL_DEPTH_COMPONENT
	};

	RBO(GLenum format);
	virtual ~RBO();

	static void bind(GLuint id);
	static void storage(GLenum internalformat, GLsizei width, GLsizei height);

	void bind() const { bind(id()); }
	void begin() const { bind(); }
	static void end() { bind(0); }
	GLenum format() const { return mFormat; }

	void storage(GLsizei width, GLsizei height);

protected:
	GLenum mFormat;
	
	virtual void onCreate();
	virtual void onDestroy();
};


/// Frame buffer object

/// All attachments must have the same dimensions
/// You can only render to RGB, RGBA, and depth textures
class FBO : public GPUObject {
public:

	FBO(){}
	virtual ~FBO();

	static void bind(GLuint id);
	static void renderBuffer(const RBO& rbo);
	static void texture2D(GLuint texID, int attach=0, int level=0);
	
	FBO& attach(const RBO& rbo);
	FBO& attachTexture2D(GLuint texID, int attach=0, int level=0);
	FBO& detachTexture2D(int attach=0, int level=0);

	/// Start rendering to attached objects
	void begin() const { bind(id()); }

	/// Stop rendering to attached objects
	static void end(){ bind(0); }

	GLenum status() const;
	const char * statusString() const;
	const char * statusString(GLenum stat) const;

protected:

	virtual void onCreate();
	virtual void onDestroy();
};





/*
#include "WWOpenGL.h"

using namespace WW;

OpenGLFrameBuffer::OpenGLFrameBuffer()
{
	m_nGLFrameBufferObjectHandle = m_nGLTextureHandle = m_nGLDepthBufferHandle = m_nGLStencilBufferHandle = 0;

	m_nWidth = m_nHeight = -1;
}

OpenGLFrameBuffer::OpenGLFrameBuffer(const OpenGLFrameBuffer &from)
{
	m_nGLFrameBufferObjectHandle = m_nGLTextureHandle = m_nGLDepthBufferHandle = m_nGLStencilBufferHandle = 0;

	m_nTextureTarget = GL_TEXTURE_2D;

	m_nWidth = m_nHeight = -1;

	*this = from;
}

OpenGLFrameBuffer::~OpenGLFrameBuffer()
{
	Reset();
}

OpenGLFrameBuffer &OpenGLFrameBuffer::operator=(const OpenGLFrameBuffer &from)
{
	Reset();

	m_nGLFrameBufferObjectHandle = from.m_nGLFrameBufferObjectHandle;
	m_nGLTextureHandle = from.m_nGLTextureHandle;
	m_nGLDepthBufferHandle = from.m_nGLDepthBufferHandle;
	m_nGLStencilBufferHandle = from.m_nGLStencilBufferHandle;

	m_nTextureTarget = from.m_nTextureTarget;

	m_nWidth = from.m_nWidth;
	m_nHeight = from.m_nHeight;

	m_sLastError = from.m_sLastError;

	return(*this);
}

void OpenGLFrameBuffer::Reset()
{
    Delete();
	DeleteTexture();
	DeleteDepth();
	DeleteStencil();

	m_nWidth = m_nHeight = -1;
}

// initialize the frame buffer object
bool OpenGLFrameBuffer::Init(int width, int height)
{
	Delete();

	glGenFramebuffersEXT(1, &m_nGLFrameBufferObjectHandle);
	
	//if(GetGLError())
	//	return(false);

	//if(!glIsFramebufferEXT(m_nGLFrameBufferObjectHandle))
	//{
	//	m_nGLFrameBufferObjectHandle = 0;
	//	m_sLastError = "Could not create frame buffer object.";
	//	return(false);
	//}

	m_nWidth = width;
	m_nHeight = height;

	if(!BindAsTarget())
		return(false);

	return(true);
}


void OpenGLFrameBuffer::Delete()
{
	if(m_nGLFrameBufferObjectHandle != 0)
		glDeleteFramebuffersEXT(1, &m_nGLFrameBufferObjectHandle);	
}

// add a texture object as the render target of the frame buffer
bool OpenGLFrameBuffer::AddTexture(GLenum target, GLint internalFormat, GLenum format)
{
	glGenTextures(1, &m_nGLTextureHandle);

	//if(GetGLError())
	//	return(false);

	glBindTexture(target, m_nGLTextureHandle);

	glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(target, 0, internalFormat, m_nWidth, m_nHeight, 0, format, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target, m_nGLTextureHandle, 0);

	//if(GetGLError())
	//	return(false);

	m_nTextureTarget = target;

	return(true);
}

void OpenGLFrameBuffer::DeleteTexture()
{
	if(m_nGLTextureHandle != 0)
	{
		glDeleteTextures(1, &m_nGLTextureHandle);
		m_nGLTextureHandle = 0;
	}
}

GLuint OpenGLFrameBuffer::GetTextureGLHandle()
{
	return(m_nGLTextureHandle);
}

// add a depth buffer
bool OpenGLFrameBuffer::AddDepth()
{
	glGenRenderbuffersEXT(1, &m_nGLDepthBufferHandle);

	//if(GetGLError())
	//	return(false);

	//if(!glIsRenderbufferEXT(m_nGLDepthBufferHandle))
	//{
	//	m_nGLDepthBufferHandle = 0;
	//	m_sLastError = "Could not create render buffer object for depth.";
	//	return(false);
	//}

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_nGLDepthBufferHandle);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, m_nWidth, m_nHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_nGLDepthBufferHandle);

	//if(GetGLError())
	//	return(false);

	return(true);
}

void OpenGLFrameBuffer::DeleteDepth()
{
	if(m_nGLDepthBufferHandle != 0)
	{
		glDeleteRenderbuffersEXT(1, &m_nGLDepthBufferHandle);
		m_nGLDepthBufferHandle = 0;
	}
}

// add a stecil buffer
bool OpenGLFrameBuffer::AddStencil()
{
	glGenRenderbuffersEXT(1, &m_nGLStencilBufferHandle);

	//if(GetGLError())
	//	return(false);

	//if(!glIsRenderbufferEXT(m_nGLStencilBufferHandle))
	//{
	//	m_nGLStencilBufferHandle = 0;
	//	m_sLastError = "Could not create render buffer object for stencil.";
	//	return(false);
	//}

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_nGLStencilBufferHandle);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_STENCIL_INDEX, m_nWidth, m_nHeight);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_nGLStencilBufferHandle);

	//if(GetGLError())
	//	return(false);

	return(true);
}

void OpenGLFrameBuffer::DeleteStencil()
{
	if(m_nGLStencilBufferHandle != 0)
	{
		glDeleteRenderbuffersEXT(1, &m_nGLStencilBufferHandle);
		m_nGLDepthBufferHandle = 0;
	}
}

// returns true if the frame buffer object is valid and can be set as target
GLenum OpenGLFrameBuffer::Completeness()
{
    return(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
}

bool OpenGLFrameBuffer::Complete()
{
	return(Completeness() == GL_FRAMEBUFFER_COMPLETE_EXT);
}

bool OpenGLFrameBuffer::IsValid()
{
	bool isOK = false;

	GLenum status;    

	BindAsTarget();

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	switch(status) 
	{                                          
	case GL_FRAMEBUFFER_COMPLETE_EXT: // Everything's OK
		isOK = true;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		m_sLastError = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		m_sLastError = "GL_FRAMEBUFFER_UNSUPPORTED_EXT";
		isOK = false;
		break;
	case GL_FRAMEBUFFER_STATUS_ERROR_EXT:
		m_sLastError = "GL_FRAMEBUFFER_STATUS_ERROR_EXT\n";
		isOK = false;
		break;
	default:
		m_sLastError = "Unknown ERROR";
		isOK = false;
	};

	return(isOK);
}

// sets as current render target
bool OpenGLFrameBuffer::BindAsTarget()
{
	//glBindTexture(m_nTextureTarget, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_nGLFrameBufferObjectHandle);

	return(true);
}

// sets as texture for active texture unit
bool OpenGLFrameBuffer::BindAsTexture()
{
	glBindTexture(m_nTextureTarget, m_nGLTextureHandle);

	return(true);
}

// return last error message generated
std::string OpenGLFrameBuffer::GetLastError()
{
	return(m_sLastError);
}

bool OpenGLFrameBuffer::GetGLError()
{
	GLenum err = glGetError();
    
	if(err != GL_NO_ERROR)
	{
		m_sLastError = (char *)gluErrorString(err);
		return(true);
	}

	return(false);
}
*/

}

#endif

