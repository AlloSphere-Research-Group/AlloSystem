#ifndef INCLUDE_AL_GRAPHICS_DISPLAYLIST_HPP
#define INCLUDE_AL_GRAPHICS_DISPLAYLIST_HPP

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
	GPU display list for drawing static geometry

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/graphics/al_OpenGL.hpp"
#include "allocore/graphics/al_GPUObject.hpp"

namespace al{

#ifdef AL_GRAPHICS_USE_OPENGL

/// Display list for drawing static geometry
///
/// @ingroup allocore
class DisplayList : public GPUObject {
public:

	/// Begin the display list's rendering commands
	void begin(){ validate(); glNewList(id(), GL_COMPILE); }

	/// End the display list's rendering commands
	void end(){ glEndList(); }

	/// Draw the display list
	void draw(){ glCallList(id()); }

protected:
	void onCreate() override {
		mID = glGenLists(1);
	}

	void onDestroy() override {
		glDeleteLists(mID, 1);
	}
};

#endif

} // al::

#endif
