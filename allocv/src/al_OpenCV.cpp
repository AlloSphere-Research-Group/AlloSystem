#include "allocv/al_OpenCV.hpp"

namespace al{

void fromCV(AlloArrayHeader& hdr, const cv::Mat& mat, int rowByteAlign){
	fromCV(hdr, mat.type());
	cv::Size size = mat.size();
	int w = size.width;
	int h = size.height;
	allo_array_setdim2d(&hdr, w,h);
	allo_array_setstride(&hdr, rowByteAlign);
}

void fromCV(Array& arr, const cv::Mat& mat, int copyPolicy, int rowByteAlign){

	AlloArrayHeader hdr;
	fromCV(hdr, mat, rowByteAlign);
	arr.format(hdr);	// allocates memory (when necessary)

	//printf("%d x %d; %d %d\n", mat.size[1], mat.size[0], mat.step[1], mat.step[0]);
	//arr.print();

	// Only copy as many bytes as is necessary (ignoring alignment, padding, etc.)
	int widthBytes = arr.width() * arr.components() * allo_type_size(arr.type());

	// Note: copies must be done row-by-row due to cv::Mat row padding
	switch(copyPolicy){
	case  1:	// Copy rows in same order
		for(unsigned j=0; j<arr.height(); ++j){
			memcpy(
				arr.cell<char>(0,j),
				mat.ptr(j,0), // cv convention is row,col
				widthBytes
			);
		}
		break;

	case -1:	// Copy with rows flipped
		for(unsigned j=0; j<arr.height(); ++j){
			memcpy(
				arr.cell<char>(0,arr.height()-j-1),
				mat.ptr(j,0), // cv convention is row,col
				widthBytes
			);
		}
		break;

	//case 2:	// Copy data byte for byte
			//memcpy(arr.data.ptr, (const char *)mat.ptr(), arr.size());

	//case 0:		// Reference data?
				// Probably not a good idea since mat is const...
		//arr.data.ptr = (char *)mat.ptr();
		break;

	default:;
	}
}

void fromCV(AlloArrayHeader& hdr, int CV_TYPE) {
	switch (CV_TYPE) {
		case CV_8UC1: hdr.type=AlloUInt8Ty;		hdr.components = 1;	break;
		case CV_8UC2: hdr.type=AlloUInt8Ty;		hdr.components = 2;	break;
		case CV_8UC3: hdr.type=AlloUInt8Ty;		hdr.components = 3;	break;
		case CV_8UC4: hdr.type=AlloUInt8Ty;		hdr.components = 4;	break;

		case CV_16UC1: hdr.type=AlloUInt16Ty;	hdr.components = 1;	break;
		case CV_16UC2: hdr.type=AlloUInt16Ty;	hdr.components = 2;	break;
		case CV_16UC3: hdr.type=AlloUInt16Ty;	hdr.components = 3;	break;
		case CV_16UC4: hdr.type=AlloUInt16Ty;	hdr.components = 4;	break;

		case CV_8SC1: hdr.type=AlloSInt8Ty;		hdr.components = 1;	break;
		case CV_8SC2: hdr.type=AlloSInt8Ty;		hdr.components = 2;	break;
		case CV_8SC3: hdr.type=AlloSInt8Ty;		hdr.components = 3;	break;
		case CV_8SC4: hdr.type=AlloSInt8Ty;		hdr.components = 4;	break;

		case CV_16SC1: hdr.type=AlloSInt16Ty;	hdr.components = 1;	break;
		case CV_16SC2: hdr.type=AlloSInt16Ty;	hdr.components = 2;	break;
		case CV_16SC3: hdr.type=AlloSInt16Ty;	hdr.components = 3;	break;
		case CV_16SC4: hdr.type=AlloSInt16Ty;	hdr.components = 4;	break;

		case CV_32SC1: hdr.type=AlloSInt32Ty;	hdr.components = 1;	break;
		case CV_32SC2: hdr.type=AlloSInt32Ty;	hdr.components = 2;	break;
		case CV_32SC3: hdr.type=AlloSInt32Ty;	hdr.components = 3;	break;
		case CV_32SC4: hdr.type=AlloSInt32Ty;	hdr.components = 4;	break;

		case CV_32FC1: hdr.type=AlloFloat32Ty;	hdr.components = 1;	break;
		case CV_32FC2: hdr.type=AlloFloat32Ty;	hdr.components = 2;	break;
		case CV_32FC3: hdr.type=AlloFloat32Ty;	hdr.components = 3;	break;
		case CV_32FC4: hdr.type=AlloFloat32Ty;	hdr.components = 4;	break;

		case CV_64FC1: hdr.type=AlloFloat64Ty;	hdr.components = 1;	break;
		case CV_64FC2: hdr.type=AlloFloat64Ty;	hdr.components = 2;	break;
		case CV_64FC3: hdr.type=AlloFloat64Ty;	hdr.components = 3;	break;
		case CV_64FC4: hdr.type=AlloFloat64Ty;	hdr.components = 4;	break;

		default:;
	}
}

int toCV(AlloTy type, unsigned char components){
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
		case AlloUInt8Ty:
			switch(components) {
				case 1: return CV_8UC1;
				case 2: return CV_8UC2;
				case 3: return CV_8UC3;
				case 4: return CV_8UC4;
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
		//case AlloUInt32Ty: // no support for 32-bit unsigned ints
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
		default:; // unhandled type
	}
	return 0;
}

cv::Mat toCV(const Array& arr) {
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
	return cv::Mat(arr.dim(1), arr.dim(0), type, (uchar *)arr.data.ptr, arr.stride(1));
}

} // al::
