#include "allocore/graphics/al_FBO.hpp"
#include <stdio.h>
namespace al{

RBO::RBO(PixelFormat format): mFormat(format){}

void RBO::bind(GLuint id){ glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, id); }

void RBO::storage(GLenum internalformat, GLsizei width, GLsizei height){
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalformat, width, height);
}

void RBO::onCreate(){ GLuint i; glGenRenderbuffersEXT(1,&i); mID=i; }
void RBO::onDestroy(){ GLuint i=id(); glDeleteRenderbuffersEXT(1, &i); }

void RBO::storage(GLsizei width, GLsizei height){
	begin(); storage(format(), width, height);
}


//	static void renderBufferDepth(GLuint rbID, int attach=0, int level=0){
//		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rbID);
//	}


FBO& FBO::attachTexture2D(GLuint texID, Attachment att, int level){
	begin(); texture2D(texID, att, level); end(); return *this;
}

FBO& FBO::attach(const RBO& rbo){
	begin(); renderBuffer(rbo); end(); return *this;
}

void FBO::bind(GLuint id){ glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id); }

FBO& FBO::detachTexture2D(Attachment att, int level){
	attachTexture2D(0,att,level);
	return *this;		
}

void FBO::onCreate(){ GLuint i; glGenFramebuffersEXT(1,&i); mID=i; }
void FBO::onDestroy(){ GLuint i=id(); glDeleteFramebuffersEXT(1, &i); }


void FBO::renderBuffer(const RBO& rbo){
	GLenum a=0;
	switch(rbo.format()){
		case RBO::DEPTH: a=GL_DEPTH_ATTACHMENT_EXT; break;
		default:;
	}
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, a, GL_RENDERBUFFER_EXT, rbo.id());
}

GLenum FBO::status(){
	begin();
	int r=glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	end();
	return r;
}

const char * FBO::statusString(){ return statusString(status()); }

const char * FBO::statusString(GLenum stat){
	#define CS(v) case v: return #v;
	switch(stat){
	CS(GL_FRAMEBUFFER_COMPLETE_EXT)							CS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT)
	CS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT)	CS(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT)
	CS(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)				CS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT)
	CS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT)			CS(GL_FRAMEBUFFER_UNSUPPORTED_EXT)
	default: return "GL_FRAMEBUFFER_UNKNOWN";
	};
}

void FBO::texture2D(GLuint texID, Attachment att, int level){
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, att, GL_TEXTURE_2D, texID, level);
}

}
