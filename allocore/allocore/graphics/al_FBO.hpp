#ifndef INC_AL_FBO_HPP
#define INC_AL_FBO_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Render and frame buffer object abstractions

	Author(s):
	Lance Putnam, 2012, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{

#ifdef AL_GRAPHICS_SUPPORTS_FBO

/// Render buffer object

/// Render buffer objects are used for offscreen rendering. They are a single
/// image of a renderable internal format, such as color, depth, or stencil.
/// Since render buffers are write-only, they can have better performance than
/// textures (read-write buffers).
///
/// @ingroup allocore
class RBO : public GPUObject{
public:

	/// @param[in] format	internal format of buffer
	RBO(Graphics::Format format = Graphics::DEPTH_COMPONENT_OFFSCREEN);


	/// Set dimensions, in pixels

	/// @param[in] width	width, in pixels
	/// @param[in] height	height, in pixels
	/// \returns whether the resize was successful
	RBO& resize(unsigned width, unsigned height);

	/// Get width in pixels
	unsigned width() const;
	/// Get height in pixels
	unsigned height() const;

	/// Set internal pixel format
	RBO& format(Graphics::Format v);

	/// Get internal pixel format
	Graphics::Format format() const;

	/// Set number of samples for multisampling (0 for no multisampling)

	/// All RBOs attached to an FBO must have the same number of samples
	/// regardless of their format---this is left up to the user to ensure.
	RBO& samples(unsigned n);

	/// Get number of samples for multisampling
	unsigned samples() const;

	/// Bind object
	void bind();

	/// Unbind object
	void unbind();

	/// Synchronize local with remote state
	void sync();


	/// Get maximum buffer size
	static unsigned maxSize();

	static void bind(unsigned id);
	static bool resize(Graphics::Format format, unsigned width, unsigned height, unsigned samples=0);

protected:
	unsigned mWidth=0, mHeight=0;
	Graphics::Format mFormat;
	unsigned mSamples = 0;
	bool mNeedsSync = true;

	template <class T>
	RBO& sync(T& dst, const T& src){
		if(dst != src) dst=src, mNeedsSync=true;
		return *this;
	}

	void onSync();

	void onCreate() override;
	void onDestroy() override;
};



/// Frame buffer object

/// A frame buffer object is an application-created frame buffer that is
/// non-displayable. A single FBO can have multiple rendering destinations
/// (attachments) for color, depth, and stencil information. If the attachment
/// is a texture, then the FBO does "render-to-texture". If the attachment is
/// a render buffer object, then the FBO does "offscreen rendering".
/// A single FBO can have multiple color attachments, but only one depth and
/// one stencil attachment. Switching the attachments (attach/detach calls) is
/// much faster than using multiple FBOs.
/// All attachments must have the same dimensions and all color attachments
/// must have the same pixel format. These are standard requirements of an FBO,
/// not an implementation-imposed limitation.
///
/// @ingroup allocore
class FBO : public GPUObject {
public:

	/// Attachment type
	enum Attachment{
		COLOR_ATTACHMENT0			= GL_COLOR_ATTACHMENT0,
		#ifdef AL_GRAPHICS_USE_OPENGL
		COLOR_ATTACHMENT1			= GL_COLOR_ATTACHMENT1,
		COLOR_ATTACHMENT2			= GL_COLOR_ATTACHMENT2,
		COLOR_ATTACHMENT3			= GL_COLOR_ATTACHMENT3,
		#endif
		DEPTH_ATTACHMENT			= GL_DEPTH_ATTACHMENT,
		STENCIL_ATTACHMENT			= GL_STENCIL_ATTACHMENT
	};


	/// Attach RBO at specified attachment point
	FBO& attachRBO(RBO& rbo, Attachment attach);

	/// Detach RBO at specified attachment point
	FBO& detachRBO(Attachment attach);

	/// Attach a texture

	/// @param[in] texID	texture ID
	/// @param[in] attach	Attachment type
	/// @param[in] level	mipmap level of texture
	FBO& attachTexture2D(unsigned texID, Attachment attach=COLOR_ATTACHMENT0, int level=0);

	template <class Texture2D>
	FBO& attachTexture2D(Texture2D& tex, Attachment attach=COLOR_ATTACHMENT0, int level=0){
		return attachTexture2D(tex.id(), attach, level);
	}

	/// Detach texture at a specified attachment point and mipmap level
	FBO& detachTexture2D(Attachment attach, int level=0);

	/// Bind object (start rendering to attached objects)
	void bind();
	void bind(int target);

	/// Unbind object
	void unbind();

	/// Call function between bind and unbind calls
	template <class Func>
	FBO& scope(const Func& f){ bind(); f(); unbind(); return *this; }
	template <class Func>
	FBO& scope(int target, const Func& f){ bind(target); f(); unbind(); return *this; }

	/// Copy data to another FBO
	FBO& copyTo(FBO& dst,
		int srcX0, int srcY0, int srcX1, int srcY1,
		int dstX0, int dstY0, int dstX1, int dstY1,
		Graphics::AttributeBit mask = Graphics::COLOR_BUFFER_BIT, bool nicest=false
	);
	FBO& copyTo(FBO& dst,
		int srcW, int srcH,
		int dstW, int dstH,
		Graphics::AttributeBit mask = Graphics::COLOR_BUFFER_BIT, bool nicest=false
	);
	FBO& copyTo(FBO& dst,
		int w, int h,
		Graphics::AttributeBit mask = Graphics::COLOR_BUFFER_BIT, bool nicest=false
	);

	/// Get status of frame buffer object
	GLenum status();
	const char * statusString();

	static const char * statusString(GLenum stat);
	static void bind(unsigned fboID, int target);
	static void renderBuffer(unsigned rboID, Attachment attach);
	static void texture2D(unsigned texID, Attachment attach=COLOR_ATTACHMENT0, int level=0);

protected:
	int mTarget;
	void onCreate() override;
	void onDestroy() override;
};

#endif

} // al::
#endif
