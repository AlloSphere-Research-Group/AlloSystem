#ifndef INCLUDE_AL_OPENCV_HPP
#define INCLUDE_AL_OPENCV_HPP

/*	AlloSystem --
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
	al::Array <-> OpenCV Mat conversion

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2013, putnam.lance@gmail.com
*/

#include <opencv2/opencv.hpp>
#include "allocore/types/al_Array.hpp"

namespace al{

/// Create an Array from an OpenCV Mat

/// @param[in] arr			Destination array
/// @param[in] mat			Source OpenCV matrix
/// @param[in] copyPolicy	Policy on copying data:
///							 1 = copy data with same row order (default),
///							-1 = copy data with rows flipped.
/// @param[in] rowByteAlign	row byte alignment (1, 2, 4, or 8)
void fromCV(Array& arr, const cv::Mat& mat, int copyPolicy=1, int rowByteAlign=1);

/// Fill in an Array header from an OpenCV Mat

/// @param[in] hdr			Destination array header
/// @param[in] mat			Source OpenCV matrix
/// @param[in] rowByteAlign	row byte alignment (1, 2, 4, or 8)
void fromCV(AlloArrayHeader& hdr, const cv::Mat& mat, int rowByteAlign=1);


// Sets the AlloTy and components fields in an AlloArrayHeader
void fromCV(AlloArrayHeader& hdr, int CV_TYPE);


/// Converts an AlloTy type and component count into an OpenCV type flag:
int toCV(AlloTy type, unsigned char components);

/// Creates a cv::Mat which directly uses an Array's data (no copying)

/// This implies that no memory management will be performed by OpenCV.
/// To copy the data (to persist beyond the Array), call clone() on the result.
cv::Mat toCV(const Array& arr);

} // al::

#endif
