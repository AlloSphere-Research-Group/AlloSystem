#ifndef INCLUDE_AL_GRAPHICS_SCREENSHOT_HPP
#define INCLUDE_AL_GRAPHICS_SCREENSHOT_HPP

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
	Screenshot utility (with minimal dependencies)

	File author(s):
	Lance Putnam, 2018, putnam.lance@gmail.com
*/

#include <string>

namespace al{

/// Save screen pixels to an image file
class Screenshot{
public:

	/// Set directory for output file (must include trailing slash)
	Screenshot& path(const std::string& v){ mPath=v; return *this; }

	/// Set format of image file
	//Screenshot& imageFormat(const std::string& ext){ mExt=ext; return *this; }

	/// Save screen pixels to an image file

	/// @param[in] w			width of capture, in pixels
	/// @param[in] h			height of capture, in pixels
	/// @param[in] filePath		if not empty, the file path to the saved image
	/// \returns whether the save was successful
	bool save(unsigned w, unsigned h, const std::string& filePath="");

	/// Save screen pixels to an image file

	/// @param[in] rect			dimensions of capture;
	///							any object with width() and height() member functions
	/// @param[in] filePath		if not empty, the file path to the saved image
	/// \returns whether the save was successful
	template <class Rect>
	bool save(const Rect& rect, const std::string& filePath=""){
		return save(rect.width(), rect.height(), filePath);
	}

	/// Save screen pixels in curent viewport to an image file

	/// @param[in] filePath		if not empty, the file path to the saved image
	/// \returns whether the save was successful
	bool save(const std::string& filePath="");

private:
	std::string mExt = "tga";
	std::string mPath = "";
};

} // al::
#endif
