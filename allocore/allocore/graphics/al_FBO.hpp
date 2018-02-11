#ifndef INCLUDE_AL_GRAPHICS_FBO_HPP
#define INCLUDE_AL_GRAPHICS_FBO_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Render and frame buffer object abstractions

	File author(s):
	Lance Putnam, 2012, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{


/// Render buffer object

/// Render buffer objects are used for offscreen rendering. They are a single
/// image of a renderable internal format, such as color, depth, or stencil.
///
/// @ingroup allocore
class RBO : public GPUObject{
public:

	/// @param[in] format	internal format of buffer
	RBO(Graphics::Format format = Graphics::DEPTH_COMPONENT);

	/// Get internal pixel format
	Graphics::Format format() const;

	/// Set internal pixel format
	RBO& format(Graphics::Format v);

	/// Bind object
	void bind();

	/// Unbind object
	void unbind();

	/// Set dimensions, in pixels

	/// @param[in] width	width, in pixels
	/// @param[in] height	height, in pixels
	/// \returns whether the resize was successful
	bool resize(unsigned width, unsigned height);


	/// Get maximum buffer size
	static unsigned maxSize();

	static void bind(unsigned id);
	static bool resize(Graphics::Format format, unsigned width, unsigned height);

protected:
	Graphics::Format mFormat;

	virtual void onCreate();
	virtual void onDestroy();

public:
	/// \deprecated
	void begin(){ bind(); }
	/// \deprecated
	static void end(){ bind(0); }
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
	FBO& attachRBO(const RBO& rbo, Attachment attach);

	/// Detach RBO at specified attachment point
	FBO& detachRBO(Attachment attach);

	/// Attach a texture

	/// @param[in] texID	texture ID
	/// @param[in] attach	Attachment type
	/// @param[in] level	mipmap level of texture
	FBO& attachTexture2D(unsigned texID, Attachment attach=COLOR_ATTACHMENT0, int level=0);

	/// Detach texture at a specified attachment point and mipmap level
	FBO& detachTexture2D(Attachment attach, int level=0);

	/// Bind object (start rendering to attached objects)
	void bind();

	/// Unbind object
	void unbind();

	/// Get status of frame buffer object
	GLenum status();
	const char * statusString();
	const char * statusString(GLenum stat);

	static void bind(unsigned fboID);
	static void renderBuffer(unsigned rboID, Attachment attach);
	static void texture2D(unsigned texID, Attachment attach=COLOR_ATTACHMENT0, int level=0);

protected:
	virtual void onCreate();
	virtual void onDestroy();

public:
	/// \deprecated
	void begin(){ bind(); }
	/// \deprecated
	static void end();
};

} // al::
#endif
