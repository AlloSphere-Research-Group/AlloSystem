#ifndef INCLUDE_AL_FONT_HPP
#define INCLUDE_AL_FONT_HPP

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS).
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include <string>
#include <map>
#include <stdio.h>
#include <stdarg.h>

#include "allocore/types/al_Array.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Graphics.hpp"

#define ASCII_SIZE 256	// number of characters to use

namespace al{

class Font {
public:
	
	struct FontCharacter{
		FontCharacter() : width(10), y_offset(0) {}
		int width;
		int y_offset;
	};

	Font(Graphics& g, std::string filename, int font_size=10, bool anti_aliased=true);
	~Font();

	// Get metrics of a particular character (idx 0..255)
	const FontCharacter & character(int idx) { return mChars[idx & 255]; }

	// returns the width of a text string in pixels
	float width(std::string text);
	float width(unsigned char c) { return mChars[c].width; }

	// returns the "above-line" height of the font in pixels
	float ascender();
	
	// returns the "below-line" height of the font in pixels
	float descender();

	// returns the total height of the font in pixels
	float size() const { return mFontSize; }
	
	
	/*! Render text geometry
		Render text into geometry for drawing a string of text using the bitmap 
		returned by ascii_chars.  Render expects the vertex and texcoord buffers 
		to be at least as big as the text length * 4 since each character is 
		rendered by a quad.
		
		Example usage:
		<pre>		
			Mesh mesh;
			font.render("allocore", mesh);
		
			font.texture().bind();
			gl.draw(mesh);
			font.texture().unbind();
		</pre>
		
	*/
	void write(Mesh& mesh, std::string text);
	
	/*
		Renders using an internal mesh (reset for each render() call)
		For rendering large volumes of text, use write() instead.
	*/
	void render(Graphics& g, std::string text);
	void renderf(Graphics& g, const char * fmt, ...);
	
	// TODO:
	//int outline(int idx, Array *vertex, Array *index);
	
	// accessor so that the font texture can be bound separately:
	Texture& texture() { return mTex; }

protected:

	// makes sure that the texture has been filled with data:
	void ensureTexture(Graphics& g);

	class Impl; 
	Impl * mImpl;

	FontCharacter mChars[ASCII_SIZE];
	unsigned int mFontSize;
	bool mAntiAliased;
	
	//The a bitmap of the font's ASCII characters in a 16x16 grid
	Texture mTex;
	Mesh mMesh;
};

inline void Font :: renderf(Graphics& g, const char * fmt, ...) {
	static char line[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(line, 1024, fmt, args);
	va_end(args);
	render(g, line);
}

inline void Font :: render(Graphics& g, std::string text) {
	write(mMesh, text);
	mTex.bind(0);
	g.draw(mMesh);
	mTex.unbind(0);
}

inline float Font :: width(std::string text) {
	float total = 0.f;
	for (unsigned i=0; i < text.size(); i++) {
		total += mChars[text[i]].width;
	}
	return total;
}

} // al::

#endif	/* include guard */

