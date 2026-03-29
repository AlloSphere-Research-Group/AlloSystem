#ifndef INC_AL_OPENCV_HPP
#define INC_AL_OPENCV_HPP

/*	AlloSystem -- Multimedia / virtual environment application class library

	Description:
	al::Array <-> OpenCV Mat conversion

	Author(s):
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
