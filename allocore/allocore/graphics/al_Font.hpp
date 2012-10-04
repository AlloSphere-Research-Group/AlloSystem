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

	Font(std::string filename, int font_size=10, bool anti_aliased=true);
	~Font();

	// Get metrics of a particular character (idx 0..255)
	const FontCharacter & character(int idx) { return mChars[idx & 255]; }

	// returns the width of a text string in pixels
	float width(std::string text);
	float width(unsigned char c) { return mChars[int(c)].width; }

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
			font.write("allocore", mesh);
		
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
		total += mChars[ (int)text[i] ].width;
	}
	return total;
}

} // al::

#endif	/* include guard */

