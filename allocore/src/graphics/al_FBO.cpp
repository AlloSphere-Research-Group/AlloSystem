#include "allocore/graphics/al_FBO.hpp"
#include <stdio.h>

#ifdef AL_GRAPHICS_SUPPORTS_FBO

namespace al{

RBO::RBO(Graphics::Format format)
:	mFormat(format)
{}

void RBO::onCreate(){
	GLuint i;
	glGenRenderbuffers(1,&i);
	mID=i;
}

void RBO::onDestroy(){
	GLuint i=id();
	glDeleteRenderbuffers(1,&i);
}

Graphics::Format RBO::format() const { return mFormat; }

RBO& RBO::format(Graphics::Format v){ mFormat=v; return *this; }

void RBO::bind(){ validate(); bind(id()); }

void RBO::unbind(){ bind(0); }

bool RBO::resize(unsigned w, unsigned h){
	bind();
	bool r = resize(format(), w, h);
	unbind();
	return r;
}

// static functions
unsigned RBO::maxSize(){
	int s;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &s);
	return s;
}

void RBO::bind(unsigned id){ glBindRenderbuffer(GL_RENDERBUFFER, id); }

bool RBO::resize(Graphics::Format format, unsigned w, unsigned h){
	unsigned mx = maxSize();
	if(w > mx || h > mx) return false;
	glRenderbufferStorage(GL_RENDERBUFFER, format, w, h);
	return true;
}




void FBO::onCreate(){
	GLuint i;
	glGenFramebuffers(1,&i);
	mID=i;
}

void FBO::onDestroy(){
	GLuint i=id();
	glDeleteFramebuffers(1,&i);
}

FBO& FBO::attachRBO(const RBO& rbo, Attachment att){
	bind();
	renderBuffer(rbo.id(), att);
	unbind();
	return *this;
}

FBO& FBO::detachRBO(Attachment att){
	bind();
	renderBuffer(0, att);
	unbind();
	return *this;
}

FBO& FBO::attachTexture2D(unsigned texID, Attachment att, int level){
	bind();
	texture2D(texID, att, level);
	unbind();
	return *this;
}

FBO& FBO::detachTexture2D(Attachment att, int level){
	attachTexture2D(0,att,level);
	return *this;
}

void FBO::bind(){ validate(); bind(id()); }

void FBO::unbind(){ bind(0); }

GLenum FBO::status(){
	bind();
	int r=glCheckFramebufferStatus(GL_FRAMEBUFFER);
	unbind();
	return r;
}

const char * FBO::statusString(){ return statusString(status()); }

const char * FBO::statusString(GLenum stat){
	#define CS(v) case v: return #v;
	switch(stat){
	CS(GL_FRAMEBUFFER_COMPLETE)
	CS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
	CS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
	CS(GL_FRAMEBUFFER_UNSUPPORTED)
	#ifdef AL_GRAPHICS_SUPPORTS_SET_RW_BUFFER
		CS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		CS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
	#endif
	#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
		CS(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
	#endif
	default: return "Unknown status";
	};
}


/*static*/ void FBO::bind(unsigned fboID){
	AL_GRAPHICS_ERROR("(before FBO::bind)", fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	AL_GRAPHICS_ERROR("binding FBO", fboID);
}

/*static*/ void FBO::renderBuffer(unsigned rboID, Attachment att){
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, att, GL_RENDERBUFFER, rboID);
}

/*static*/ void FBO::texture2D(GLuint texID, Attachment att, int level){
	glFramebufferTexture2D(GL_FRAMEBUFFER, att, GL_TEXTURE_2D, texID, level);
}

} // al::

#endif //AL_GRAPHICS_SUPPORTS_FBO
