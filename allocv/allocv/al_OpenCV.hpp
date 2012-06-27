#ifndef INCLUDE_AL_OPENCV_HPP
#define INCLUDE_AL_OPENCV_HPP

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
	al::Array <-> OpenCV Mat conversion

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

// include OpenCV:
#include "opencv/cv.h"
#include "opencv/cv.hpp"

#include "allocore/types/al_Array.hpp"


namespace al{

/// Converts an AlloTy type and component count into an OpenCV type flag:
inline int toCV(AlloTy type, uint8_t components);

// sets the AlloTy and components fields in an AlloArrayHeader
inline void fromCV(AlloArrayHeader hdr, int CV_TYPE);

/// creates a cv::Mat which directly uses an Array's data (no copying)
/// This implies that no memory management will be performed by OpenCV.
/// To copy the data (to persist beyond the Array), call clone() on the result.
inline CvMat toCV(const Array& arr) {
	// This for OpenCV2:
//	size_t steps[ALLO_ARRAY_MAX_DIMS];
//	int sizes[ALLO_ARRAY_MAX_DIMS];
//	for (int i=0; i<arr.header.dimcount; i++) {
//		steps[i] = arr.header.stride[i];
//		sizes[i] = arr.header.dim[i];
//	}
//	return CvMat(
//		arr.header.dimcount,
//		sizes,
//		toCV(arr.header.type, arr.header.components),
//		arr.data.ptr,
//		steps
//	);

	int type = toCV(arr.header.type, arr.header.components);
	return cvMat(arr.dim(1), arr.dim(0), type, (uchar *)arr.data.ptr);
}


/// create an Array from an OpenCV Mat
/// default behavior is to copy the data
//inline void fromCV(Array& arr, const cv::Mat& mat, bool copyData=true) {
//	AlloArrayHeader hdr;
//	cv::Size size = mat.size();
//
//	fromCV(hdr, mat.type());
//
//	arr.format(hdr); // allocates.
//	arr.data.ptr = (char *)mat.ptr();
//}

/// Converts an AlloTy type and component count into an OpenCV type flag:
inline int toCV(AlloTy type, uint8_t components) {
	switch(type) {
		case AlloFloat32Ty:
			switch(components) {
				case 1:	return CV_32FC1;
				case 2:	return CV_32FC2;
				case 3:	return CV_32FC3;
				case 4:	return CV_32FC4;
			}
			break;
		case AlloFloat64Ty:
			switch(components) {
				case 1:	return CV_64FC1;
				case 2:	return CV_64FC2;
				case 3:	return CV_64FC3;
				case 4:	return CV_64FC4;
			}
			break;
		case AlloSInt8Ty:
			switch(components) {
				case 1:	return CV_8SC1;
				case 2:	return CV_8SC2;
				case 3:	return CV_8SC3;
				case 4:	return CV_8SC4;
			}
			break;
		case AlloSInt16Ty:
			switch(components) {
				case 1: return CV_16SC1;
				case 2: return CV_16SC2;
				case 3: return CV_16SC3;
				case 4: return CV_16SC4;
			}
			break;
		case AlloSInt32Ty:
			switch(components) {
				case 1: return CV_32SC1;
				case 2: return CV_32SC2;
				case 3: return CV_32SC3;
				case 4: return CV_32SC4;
			}
			break;
		case AlloUInt16Ty:
			switch(components) {
				case 1: return CV_16UC1;
				case 2: return CV_16UC2;
				case 3: return CV_16UC3;
				case 4: return CV_16UC4;
			}
			break;
		default:
			// unhandled type:
			return 0;
	}
	return 0;
}

// sets the AlloTy and components fields in an AlloArrayHeader
inline void fromCV(AlloArrayHeader hdr, int CV_TYPE) {
	switch (CV_TYPE) {
		case CV_8UC1: hdr.type = AlloUInt8Ty;	hdr.components = 1;	break;
		case CV_8UC2: hdr.type = AlloUInt8Ty;	hdr.components = 2;	break;
		case CV_8UC3: hdr.type = AlloUInt8Ty;	hdr.components = 3;	break;
		case CV_8UC4: hdr.type = AlloUInt8Ty;	hdr.components = 4;	break;

		case CV_16UC1: hdr.type = AlloUInt16Ty;	hdr.components = 1;	break;
		case CV_16UC2: hdr.type = AlloUInt16Ty;	hdr.components = 2;	break;
		case CV_16UC3: hdr.type = AlloUInt16Ty;	hdr.components = 3;	break;
		case CV_16UC4: hdr.type = AlloUInt16Ty;	hdr.components = 4;	break;

		case CV_8SC1: hdr.type = AlloSInt8Ty;	hdr.components = 1;	break;
		case CV_8SC2: hdr.type = AlloSInt8Ty;	hdr.components = 2;	break;
		case CV_8SC3: hdr.type = AlloSInt8Ty;	hdr.components = 3;	break;
		case CV_8SC4: hdr.type = AlloSInt8Ty;	hdr.components = 4;	break;

		case CV_16SC1: hdr.type = AlloSInt16Ty;	hdr.components = 1;	break;
		case CV_16SC2: hdr.type = AlloSInt16Ty;	hdr.components = 2;	break;
		case CV_16SC3: hdr.type = AlloSInt16Ty;	hdr.components = 3;	break;
		case CV_16SC4: hdr.type = AlloSInt16Ty;	hdr.components = 4;	break;

		case CV_32SC1: hdr.type = AlloSInt32Ty;	hdr.components = 1;	break;
		case CV_32SC2: hdr.type = AlloSInt32Ty;	hdr.components = 2;	break;
		case CV_32SC3: hdr.type = AlloSInt32Ty;	hdr.components = 3;	break;
		case CV_32SC4: hdr.type = AlloSInt32Ty;	hdr.components = 4;	break;

		case CV_32FC1: hdr.type = AlloFloat32Ty;	hdr.components = 1;	break;
		case CV_32FC2: hdr.type = AlloFloat32Ty;	hdr.components = 2;	break;
		case CV_32FC3: hdr.type = AlloFloat32Ty;	hdr.components = 3;	break;
		case CV_32FC4: hdr.type = AlloFloat32Ty;	hdr.components = 4;	break;

		case CV_64FC1: hdr.type = AlloFloat64Ty;	hdr.components = 1;	break;
		case CV_64FC2: hdr.type = AlloFloat64Ty;	hdr.components = 2;	break;
		case CV_64FC3: hdr.type = AlloFloat64Ty;	hdr.components = 3;	break;
		case CV_64FC4: hdr.type = AlloFloat64Ty;	hdr.components = 4;	break;

		default:
			break;
	}
}

} // al::

#endif

