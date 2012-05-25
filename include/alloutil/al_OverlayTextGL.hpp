#ifndef INC_AL_OVERLAY_TEXT_OPENGL_HPP
#define INC_AL_OVERLAY_TEXT_OPENGL_HPP

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
	A printf for OpenGL windows:

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Font.hpp"

/*!
	A printf for OpenGL windows:
 */

namespace al {

class OverlayText {
public:
	
	OverlayText(int maxlines = 20)
	: mMaxlines(maxlines) {}
	
	void printf(std::string line) {
		lines.push_back(line);
	}
	void printf(const char * fmt, ...) {
		static char line[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(line, 1024, fmt, args);
		va_end(args);
		lines.push_back(line);
	}
	
	void draw(Font& font) {
		while (lines.size() > mMaxlines) lines.pop_front();
		gl.pushMatrix();
		// because OpenGL/font is upside-down:
		for (std::list<std::string>::iterator it = lines.begin(); it != lines.end(); it++) {
			font.render(gl, *it);
			gl.translate(0, -font.size(), 0);
		}
		gl.popMatrix();
	}
	
	void clear() {
		lines.clear();
	}
	
	std::list<std::string> lines;
	Graphics gl;
	int mMaxlines;
};

} // al::

#endif
