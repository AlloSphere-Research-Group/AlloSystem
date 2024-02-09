#ifndef INCLUDE_AL_VIEWPORT_HPP
#define INCLUDE_AL_VIEWPORT_HPP

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
	Viewport class for describing a region of the screen

	File author(s):
	Lance Putnam, 2024, putnam.lance@gmail.com
*/

namespace al {

/// A framed area on a display screen
/// @ingroup allocore
struct Viewport {
	float l, b, w, h;	///< left, bottom, width, height

	/// @param[in] w_	width
	/// @param[in] h_	height
	Viewport(float w_=800, float h_=600)
	:	Viewport(0.f,0.f,w_,h_)
	{}

	/// @param[in] l_	left edge coordinate
	/// @param[in] b_	bottom edge coordinate
	/// @param[in] w_	width
	/// @param[in] h_	height
	Viewport(float l_, float b_, float w_, float h_)
	:	l(l_), b(b_), w(w_), h(h_)
	{}


	/// Get aspect ratio (width divided by height)
	float aspect() const { return (h!=0.f && w!=0.f) ? w/h : 1.f; }

	/// Set dimensions
	Viewport& set(float l_, float b_, float w_, float h_){
		l=l_; b=b_; w=w_; h=h_;
		return *this;
	}
};

} // al::

#endif
