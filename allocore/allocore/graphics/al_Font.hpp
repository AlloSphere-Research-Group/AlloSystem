#ifndef INCLUDE_AL_FONT_HPP
#define INCLUDE_AL_FONT_HPP

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
	Font loader and renderer

	File author(s):
	Lance Putnam, 2024
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <string>
#include "allocore/system/al_Pimpl.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Texture.hpp"

#ifndef AL_FONT_ASCII_SIZE
#define AL_FONT_ASCII_SIZE 128	// number of characters to use
#endif

namespace al{

class Graphics;

/// Interface for loading fonts and rendering text
///
/// @ingroup allocore
class Font {
public:

	/// Metrics of a single glyph
	struct GlyphMetric{
		float xadvance;			///< effective width (can include padding)
		float xoffset, yoffset;	///< position offsets
		float u0, v0, u1, v1;	///< texture (UV) coords
	};


	Font();

	/// \param[in] filename		path to font file
	/// \param[in] fontSize		size of font, in pixels
	/// \param[in] antialias	whether to apply antialiasing
	Font(const std::string& filename, int fontSize=10, bool antialias=true);


	/// Load font from file

	/// \param[in] filename		path to font file
	/// \param[in] fontSize		size of font, in pixels
	/// \param[in] antialias	whether to apply antialiasing
	/// \returns whether font loaded successfully
	bool load(const std::string& filename, int fontSize=10, bool antialias=true);


	/// Get metrics of a particular character
	const GlyphMetric& glyphMetric(int c) const;

	/// Returns the bounds of a text string
	void bounds(float& w, float& h, const std::string& text) const;

	/// Returns the width of a character
	float width(int c) const;

	/// Returns the x-height (height of lowercase x)
	float size() const { return mFontSize; }

	/// Returns the "above-line" height
	float ascender() const { return mAscender; }

	/// Returns the "below-line" height of the font
	float descender() const { return mDescender; }


	/// Set alignment of rendered text strings

	/// \param[in] xfrac	Fraction along text width to render at x=0;
	///						0 is left-aligned, 0.5 is centered, 1 is right-aligned
	/// \param[in] yfrac	Fraction along text height to render at y=0
	Font& align(float xfrac, float yfrac);

	/// Set spacing between lines of text as multiple of x-height
	Font& lineSpacing(float v){ mLineSpacing=v; return *this; }
	float lineSpacing() const { return mLineSpacing; }

	/*! Write text geometry to mesh

		Example usage:
		<pre>
			Mesh mesh;
			font.write(mesh, "allocore");

			font.texture().bind();
			gl.draw(mesh);
			font.texture().unbind();
		</pre>
	*/
	void write(Mesh& mesh, const std::string& text);

	/*!
		Renders using an internal mesh (reset for each render() call)
		For rendering large volumes of text, use write() instead.
	*/
	void render(Graphics& g, const std::string& text);
	void renderf(Graphics& g, const char * fmt, ...);

	// Accessor so that the font texture can be bound separately
	Texture& texture() { return mTex; }

protected:
	class Impl;
	Pimpl<Impl> mImpl;

	static constexpr int glyphBegin = 32;
	static constexpr int glyphEnd = AL_FONT_ASCII_SIZE;
	static constexpr int glyphCount = glyphEnd - glyphBegin;
	static_assert(glyphEnd > glyphBegin, "AL_FONT_ASCII_SIZE must be greater than glyphBegin");

	Texture mTex; // bitmap of the font's ASCII characters in a 16x16 grid
	Mesh mMesh;
	GlyphMetric mMetrics[glyphCount];
	float mFontSize = 12.f; // x-height in pixels
	float mAscender = 0.f;
	float mDescender = 0.f;
	float mLineSpacing = 1.25f;
	float mAlign[2] = {0.f, 0.f};
	bool mAntiAliased = true;

	GlyphMetric& glyphMetric(int c);
};

} // al::

#endif	/* include guard */
