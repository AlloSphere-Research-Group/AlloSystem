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

RBO& RBO::samples(unsigned n){ mSamples=n; return *this; }

void RBO::bind(){ validate(); bind(id()); }

void RBO::unbind(){ bind(0); }

bool RBO::resize(unsigned w, unsigned h){
	bind();
	bool r = resize(mFormat, w, h, mSamples);
	unbind();
	return r;
}

/*static*/ unsigned RBO::maxSize(){
	return Graphics::paramInt(GL_MAX_RENDERBUFFER_SIZE);
}

/*static*/ void RBO::bind(unsigned id){
	glBindRenderbuffer(GL_RENDERBUFFER, id);
	AL_GRAPHICS_ERROR("RBO::bind (glBindRenderbuffer)", id);
}

/*static*/ bool RBO::resize(Graphics::Format format, unsigned w, unsigned h, unsigned samples){
	unsigned mx = maxSize();
	if(w > mx || h > mx) return false;
	AL_GRAPHICS_ERROR("before RBO::resize", -1);
	if(0 == samples){
		glRenderbufferStorage(GL_RENDERBUFFER, format, w, h);
		AL_GRAPHICS_ERROR("RBO::resize (glRenderbufferStorage)", -1);
	} else {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, w,h);
		AL_GRAPHICS_ERROR("RBO::resize (glRenderbufferStorageMultisample)", -1);
	}
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
	return scope([&](){
		renderBuffer(rbo.id(), att);
	});
}

FBO& FBO::detachRBO(Attachment att){
	return scope([&](){
		renderBuffer(0, att);
	});
}

FBO& FBO::attachTexture2D(unsigned texID, Attachment att, int level){
	return scope([&](){
		texture2D(texID, att, level);
	});
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

FBO& FBO::copyTo(FBO& dst,
	int srcX0, int srcY0, int srcX1, int srcY1,
	int dstX0, int dstY0, int dstX1, int dstY1,
	Graphics::AttributeBit mask, bool nicest
){
	#ifdef AL_GRAPHICS_SUPPORTS_FBO_RW_BIND
	// Scissor test affects blit operation!
	bool scissorTest = Graphics::paramBool(GL_SCISSOR_TEST);
	if(scissorTest) glDisable(GL_SCISSOR_TEST);

	scope(GL_READ_FRAMEBUFFER, [&](){
		AL_GRAPHICS_ERROR("FBO::copyTo bind source", id());
		dst.scope(GL_DRAW_FRAMEBUFFER, [&](){
			AL_GRAPHICS_ERROR("FBO::copyTo bind dest", dst.id());
			glBlitFramebuffer(srcX0,srcY0,srcX1,srcY1, dstX0,dstY0,dstX1,dstY1, mask, nicest ? GL_LINEAR : GL_NEAREST);
			AL_GRAPHICS_ERROR("FBO::copyTo (glBlitFramebuffer)", id());
		});
	});

	if(scissorTest) glEnable(GL_SCISSOR_TEST);
	#endif
	return *this;
}

FBO& FBO::copyTo(FBO& dst,
	int srcW, int srcH,
	int dstW, int dstH,
	Graphics::AttributeBit mask, bool nicest
){
	return copyTo(dst, 0,0,srcW,srcH, 0,0,dstW,dstH, mask, nicest);
}

GLenum FBO::status(){
	bind();
	auto r = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	unbind();
	return r;
}

const char * FBO::statusString(){ return statusString(status()); }

/*static*/ const char * FBO::statusString(GLenum stat){
	#define CS(v) case v: return #v;
	switch(stat){
	CS(GL_FRAMEBUFFER_COMPLETE)
	CS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
	CS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
	CS(GL_FRAMEBUFFER_UNSUPPORTED)
	/*#ifdef AL_GRAPHICS_SUPPORTS_FBO_RW_BIND
		CS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		CS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
	#endif*/
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
	AL_GRAPHICS_ERROR("FBO::renderBuffer (glFramebufferRenderbuffer)", rboID);
}

/*static*/ void FBO::texture2D(GLuint texID, Attachment att, int level){
	glFramebufferTexture2D(GL_FRAMEBUFFER, att, GL_TEXTURE_2D, texID, level);
	AL_GRAPHICS_ERROR("FBO::texture2D (glFramebufferTexture2D)", texID);
}

} // al::

#endif //AL_GRAPHICS_SUPPORTS_FBO
