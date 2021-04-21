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
	Abstraction over the FreeType library

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <string>
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Graphics.hpp"

#define ASCII_SIZE 256	// number of characters to use

namespace al{

/// Interface for loading fonts and rendering text
///
/// @ingroup allocore
class Font {
public:

	/// Metrics of a single font character
	struct FontCharacter{
		FontCharacter() : width(10), x_offset(0), y_offset(0) {}
		int width;
		int x_offset;
		int y_offset;
	};


	Font();

	/// \param[in] filename		path to font file
	/// \param[in] fontSize		size of font
	/// \param[in] antialias	whether to apply antialiasing
	Font(const std::string& filename, int fontSize=10, bool antialias=true);

	~Font();


	/// Load font from file

	/// \param[in] filename		path to font file
	/// \param[in] fontSize		size of font
	/// \param[in] antialias	whether to apply antialiasing
	/// \returns whether font loaded successfully
	bool load(const std::string& filename, int fontSize=10, bool antialias=true);


	/// Get metrics of a particular character (idx 0..255)
	const FontCharacter& character(int idx) const { return mChars[idx & 255]; }

	/// Returns the width of a text string, in pixels
	float width(const std::string& text) const;

	/// Returns the width of a character, in pixels
	float width(unsigned char c) const { return mChars[int(c)].width; }

	/// Returns the "above-line" height of the font, in pixels
	float ascender() const;

	/// Returns the "below-line" height of the font, in pixels
	float descender() const;

	/// Returns the total height of the font, in pixels
	float size() const { return mFontSize; }


	/// Set alignment of rendered text strings

	/// \param[in] xfrac	Fraction along text width to render at x=0;
	///						0 is left-aligned, 0.5 is centered, 1 is right-aligned
	/// \param[in] yfrac	Fraction along text height to render at y=0
	void align(float xfrac, float yfrac);


	/*! Render text geometry
		Render text into geometry for drawing a string of text using the bitmap
		returned by ascii_chars.  Render expects the vertex and texcoord buffers
		to be at least as big as the text length * 4 since each character is
		rendered by a quad.

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

	// accessor so that the font texture can be bound separately:
	Texture& texture() { return mTex; }

protected:
	// makes sure that the texture has been filled with data:
	void ensureTexture(Graphics& g);

	class Impl;
	Impl * mImpl;

	Texture mTex; //Bitmap of the font's ASCII characters in a 16x16 grid
	Mesh mMesh;
	FontCharacter mChars[ASCII_SIZE];
	unsigned int mFontSize;
	float mAlign[2];
	bool mAntiAliased;
};

} // al::

#endif	/* include guard */
