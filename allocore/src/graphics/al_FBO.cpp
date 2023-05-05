#include "allocore/graphics/al_FBO.hpp"
#include <cstdio>

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

/*static*/ unsigned RBO::maxSize(){
	int s;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &s);
	return s;
}

/*static*/ void RBO::bind(unsigned id){
	glBindRenderbuffer(GL_RENDERBUFFER, id);
}

/*static*/ bool RBO::resize(Graphics::Format format, unsigned w, unsigned h){
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

void FBO::bind(){ bind(GL_FRAMEBUFFER); }

void FBO::bind(int target){
	validate();
	mTarget = target;
	bind(id(), mTarget);
}

void FBO::unbind(){ bind(0, mTarget); }

void FBO::copyTo(FBO& dst,
	int srcX0, int srcY0, int srcX1, int srcY1,
	int dstX0, int dstY0, int dstX1, int dstY1,
	Graphics::AttributeBit mask, bool nicest
){
	#ifdef AL_GRAPHICS_SUPPORTS_SET_RW_BUFFERS
	// Scissor test affects blit operation!
	GLboolean scissorTest;
	glGetBooleanv(GL_SCISSOR_TEST, &scissorTest);
	if(scissorTest) glDisable(GL_SCISSOR_TEST);

	bind(GL_READ_FRAMEBUFFER);
	dst.bind(GL_DRAW_FRAMEBUFFER);
	
	glBlitFramebuffer(srcX0,srcY0,srcX1,srcY1, dstX0,dstY0,dstX1,dstY1, mask, nicest ? GL_LINEAR : GL_NEAREST);

	unbind();
	dst.unbind();

	if(scissorTest) glEnable(GL_SCISSOR_TEST);
	#endif
}

GLenum FBO::status(){
	bind();
	int r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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

/*static*/ void FBO::bind(unsigned fboID, int target){
	AL_GRAPHICS_ERROR("(before FBO::bind)", fboID);
	glBindFramebuffer(target, fboID);
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
