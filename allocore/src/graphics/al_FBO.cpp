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

void RBO::bind(){ validate(); bind(id()); onSync(); }

void RBO::unbind(){ bind(0); }

void RBO::sync(){ bind(); unbind(); }

RBO& RBO::resize(unsigned w, unsigned h){ return sync(mWidth,w).sync(mHeight,h); }

unsigned RBO::width() const { return mWidth; }
unsigned RBO::height() const { return mHeight; }

Graphics::Format RBO::format() const { return mFormat; }

RBO& RBO::format(Graphics::Format v){ return sync(mFormat,v); }

unsigned RBO::samples() const { return mSamples; }

RBO& RBO::samples(unsigned n){ return sync(mSamples,n); }

void RBO::onSync(){
	if(mNeedsSync){
		mNeedsSync = false;
		resize(mFormat, mWidth, mHeight, mSamples);
	}
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

	// Determine a suitable internal format which may differ from the passed in
	// format. Note that this logic will likely fail if the format was 
	// explicitly casted from some random GL format.
	int ifmt;
	if(Graphics::isColor(format)){
		switch(Graphics::numComponents(format)){
			default:ifmt = AL_GRAPHICS_COLOR4_OFFSCREEN; break;
			case 3: ifmt = AL_GRAPHICS_COLOR3_OFFSCREEN; break;
			case 2: ifmt = AL_GRAPHICS_COLOR2_OFFSCREEN; break;
			case 1: ifmt = AL_GRAPHICS_COLOR1_OFFSCREEN; break;
		}
	} else { //assuming depth
		ifmt = Graphics::DEPTH_COMPONENT_OFFSCREEN;
	}

	#ifdef AL_GRAPHICS_SUPPORTS_RBO_MS
		#define AL_RBO_STORAGE_MS glRenderbufferStorageMultisample
	#else
		#define AL_RBO_STORAGE_MS(t,s,f,w,h)\
			AL_WARN_ONCE("RBO multisampling not supported. Falling back to non-multisampled RBO.");\
			glRenderbufferStorage(t,f,w,h)
	#endif

	if(0 == samples){
		glRenderbufferStorage(GL_RENDERBUFFER, ifmt, w,h);
		AL_GRAPHICS_ERROR("RBO::resize (glRenderbufferStorage)", -1);
	} else {
		AL_RBO_STORAGE_MS(GL_RENDERBUFFER, samples, ifmt, w,h);
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

FBO& FBO::attachRBO(RBO& rbo, Attachment att){
	return scope([&](){
		rbo.sync(); // must have valid remote configuration before attaching
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

#else
	AL_WARN_ONCE("FBO blitting not supported (in FBO::copyTo).");
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

FBO& FBO::copyTo(FBO& dst,
	int w, int h,
	Graphics::AttributeBit mask, bool nicest
){
	return copyTo(dst, w,h, w,h, mask, nicest);
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
