#include <stdio.h>

#include "allocore/graphics/al_Image.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp"

#include "FreeImage.h"
/*
	\class FreeImageImpl

	Loads and saves images using the FreeImage library.  Supported formats include:

		bmp, chead, cut, dcx, dds, doom, doomFlat, exif, gif, hdr, ico, jasc_pal, jpg,
		lbm, lif, mdl, pcd, pcx, pic, pix, png, pnm, psd, psp, pxr, raw, sgi, tgo, tif,
		wal, xpm

	FreeImage is used under the FreeImage Public License (FIPL) v1.0
	See the /licenses folder in the source tree, or
	http://freeimage.sourceforge.net/freeimage-license.txt
*/

namespace al{

class FreeImageImpl : public Image::Impl {
public:
	FreeImageImpl()
	:	mImage(NULL)
	{
		// Initialization singleton
		struct Init{
			Init(){ FreeImage_Initialise(); }
			~Init(){ FreeImage_DeInitialise(); }
			static Init& get(){
				static Init v;
				return v;
			}
		};

		Init::get();
	}

	virtual ~FreeImageImpl() {
		destroy();
	}

	virtual bool load(const std::string& filename, Array &arr) {
		FREE_IMAGE_FORMAT type = FreeImage_GetFIFFromFilename(filename.c_str());
		if(type == FIF_UNKNOWN) {
			AL_WARN("image format not recognized: %s", filename.c_str());
			return false;
		}
		if(!FreeImage_FIFSupportsReading(type)) {
			AL_WARN("image format not supported: %s", filename.c_str());
			return false;
		}

		destroy();
		mImage = FreeImage_Load(type, filename.c_str(), 0);
		if (mImage == NULL) {
			AL_WARN("image failed to load: %s", filename.c_str());
			return false;
		}

		FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(mImage);
		switch(colorType) {
			case FIC_MINISBLACK:
			case FIC_MINISWHITE: {
					FIBITMAP *res = FreeImage_ConvertToGreyscale(mImage);
					FreeImage_Unload(mImage);
					mImage = res;
				}
				break;

			case FIC_PALETTE: {
					if(FreeImage_IsTransparent(mImage)) {
						FIBITMAP *res = FreeImage_ConvertTo32Bits(mImage);
						FreeImage_Unload(mImage);
						mImage = res;
					}
					else {
						FIBITMAP *res = FreeImage_ConvertTo24Bits(mImage);
						FreeImage_Unload(mImage);
						mImage = res;
					}
				}
				break;

			case FIC_CMYK: {
					AL_WARN("CMYK images currently not supported");
					return false;
				}
				break;

			default:
				break;
		}

		// flip vertical for OpenGL:
		//FreeImage_FlipVertical(mImage);

		//Freeimage is not tightly packed, so we use
		//a custom method instead of one of the Matrix
		//utility methods
		int planes = getPlanes();
		AlloTy ty = getDataType();
		int w, h;
		getDim(w, h);
		arr.format(planes, ty, w, h);

		Image::Format format = Image::getFormat(planes);
		switch(format) {
			case Image::LUMINANCE: {
				char *o_pix = (char *)(arr.data.ptr);
				int rowstride = arr.stride(1);
				for(unsigned j = 0; j < arr.dim(1); ++j) {
					char *pix = (char *)FreeImage_GetScanLine(mImage, j);
					memcpy(o_pix, pix, rowstride);
					o_pix += rowstride;
				}
			}
			break;

			case Image::RGB: {
				switch(arr.type()) {
					case AlloUInt8Ty: {
						char *bp = (char *)(arr.data.ptr);
						int rowstride = arr.stride(1);

						for(unsigned j = 0; j < arr.dim(1); ++j) {
							RGBTRIPLE * pix = (RGBTRIPLE *)FreeImage_GetScanLine(mImage, j);
							Image::RGBPix<uint8_t> *o_pix = (Image::RGBPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < arr.dim(0); ++i) {
								o_pix[i].r = pix[i].rgbtRed;
								o_pix[i].g = pix[i].rgbtGreen;
								o_pix[i].b = pix[i].rgbtBlue;
							}
						}
					}
					break;

					case AlloFloat32Ty: {
						char *o_pix = (char *)(arr.data.ptr);
						int rowstride = arr.stride(1);

						for(unsigned j = 0; j < arr.dim(1); ++j) {
							char *pix = (char *)FreeImage_GetScanLine(mImage, j);
							memcpy(o_pix, pix, rowstride);
							o_pix += rowstride;
						}
					}
					break;

					default:
					break;

				}
			}
			break;

			case Image::RGBA: {
				switch(arr.type()) {
					case AlloUInt8Ty: {
						char *bp = (char *)(arr.data.ptr);
						int rowstride = arr.stride(1);
						for(unsigned j = 0; j < arr.dim(1); ++j) {
							RGBQUAD *pix = (RGBQUAD *)FreeImage_GetScanLine(mImage, j);
							Image::RGBAPix<uint8_t> *o_pix = (Image::RGBAPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < arr.dim(0); ++i) {
								o_pix[i].r = pix[i].rgbRed;
								o_pix[i].g = pix[i].rgbGreen;
								o_pix[i].b = pix[i].rgbBlue;
								o_pix[i].a = pix[i].rgbReserved;
							}
						}
					}
					break;

					case AlloFloat32Ty: {
						char *o_pix = (char *)(arr.data.ptr);
						int rowstride = arr.stride(1);
						for(unsigned j = 0; j < arr.dim(1); ++j) {
							char *pix = (char *)FreeImage_GetScanLine(mImage, j);
							memcpy(o_pix, pix, rowstride);
							o_pix += rowstride;
						}
					}
					break;

					default: break;
				}
			}
			break;

			default:
				AL_WARN("image data not understood");
				destroy();
				return false;
		}
		return true;
	}


	void resize(int w, int h, int bpp){
		if(NULL != mImage){ // already allocated image?
			int oldw = FreeImage_GetWidth(mImage);
			int oldh = FreeImage_GetHeight(mImage);
			int oldbpp = FreeImage_GetBPP(mImage);
			// re-allocate only if dimensions have changed
			if(oldw != w || oldh != h || oldbpp != bpp){
				destroy();
				resize(w,h,bpp);
			}
		}
		else{
			/*
			FreeImage_AllocateT(FREE_IMAGE_TYPE type, int width, int height, int bpp);
			bpp		Bit depth of the new Bitmap. Supported pixel depth:
					1-, 4-, 8-, 16-, 24-, 32-bit per pixel for standard bitmap
			*/
			//printf("FreeImage_AllocateT(%d, %d, %d)\n", w,h,bpp);
			mImage = FreeImage_AllocateT(FIT_BITMAP, w, h, bpp);
		}
	}


	virtual bool save(const std::string& filename, const Array& arr, int compressFlags) {

		// check existing image type
		FREE_IMAGE_FORMAT fileType = FreeImage_GetFIFFromFilename(filename.c_str());

		if(fileType == FIF_UNKNOWN) {
			AL_WARN("image format not recognized: %s", filename.c_str());
			return false;
		}
		if(!FreeImage_FIFSupportsWriting(fileType)) {
			AL_WARN("image format not supported: %s", filename.c_str());
			return false;
		}

		Image::Format format = Image::getFormat(arr.components());

		if(FIF_JPEG == fileType && (Image::RGBA == format || Image::LUMALPHA == format)){
			AL_WARN("cannot save JPEG with alpha channel; use 24-bit RGB or 8-bit grayscale/luminance");
			return false;
		}

		unsigned w = arr.dim(0);
		unsigned h = (arr.dimcount() > 1) ? arr.dim(1) : 1;
		int bpp = arr.stride(0)*8; // bits/pixel

		resize(w,h,bpp);

		if (mImage == NULL) {
			AL_WARN("image could not be understood");
			return false;
		}

		const int rowstride = arr.stride(1);

		//printf("w=%d, h=%d, bpp=%d, stride=%d\n", w,h,bpp,rowstride);

		switch(format) {

			case Image::LUMINANCE:
				switch(arr.type()) {

					case AlloUInt8Ty: {
						char *bp = (char *)(arr.data.ptr);
						for(unsigned j = 0; j < h; ++j) {
							memcpy(
								FreeImage_GetScanLine(mImage, j),
								bp + j*rowstride,
								w
							);
						}
					}
					break;

					default:
						AL_WARN("input Array component type not supported");
						return false;
				}
			break;

			// TODO: must save as RGBA
			/*case Image::LUMALPHA:
				switch(header.type) {

					case AlloUInt8Ty: {
						char *bp = (char *)(arr.data.ptr);
						for(unsigned j = 0; j < h; ++j) {
							memcpy(
								FreeImage_GetScanLine(mImage, j),
								bp + j*rowstride,
								w*2
							);
						}
					}
					break;

					default:
						AL_WARN("input Array component type not supported");
						return false;
				}
			break;*/

			/* According to the FreeImage documentation ("Pixel access functions"):
			"When accessing to individual color components of a 24- or 32-bit
			DIB, you should always use FreeImage macros or RGBTRIPLE / RGBQUAD
			structures in order to write OS independent code.
			*/
			case Image::RGB: {
				switch(arr.type()) {

					case AlloUInt8Ty: { //printf("FreeImageImpl: save uint8/RGB\n");
						char *bp = (char *)(arr.data.ptr);

						for(unsigned j = 0; j < h; ++j) {
							RGBTRIPLE * dst = (RGBTRIPLE *)FreeImage_GetScanLine(mImage, j);
							const Image::RGBPix<uint8_t> * src = (const Image::RGBPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < w; ++i) {
								dst[i].rgbtRed  = src[i].r;
								dst[i].rgbtGreen= src[i].g;
								dst[i].rgbtBlue = src[i].b;
							}
						}
					}
					break;

					default:
						AL_WARN("input Array component type not supported");
						return false;
				}
			}
			break;

			case Image::RGBA: {

				switch(arr.type()) {

					case AlloUInt8Ty: {
						char *bp = (char *)(arr.data.ptr);

						for(unsigned j = 0; j < h; ++j) {
							RGBQUAD * dst = (RGBQUAD *)FreeImage_GetScanLine(mImage, j);
							const Image::RGBAPix<uint8_t> * src = (const Image::RGBAPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < w; ++i) {
								dst[i].rgbRed     = src[i].r;
								dst[i].rgbGreen   = src[i].g;
								dst[i].rgbBlue    = src[i].b;
								dst[i].rgbReserved= src[i].a;
							}
						}
					}
					break;

					default:
						AL_WARN("input Array component type not supported");
						return false;
				}
			}
			break;

			default: {
				AL_WARN("input Array component format not supported");
				return false;
			}
		}


		int flags;
		int compressAmt = compressFlags & 127;
		int quality = 100-(compressAmt<=100?compressAmt:100);

		switch(fileType){

		case FIF_BMP:
			flags = compressAmt >= 50 ? BMP_SAVE_RLE : BMP_DEFAULT;
			break;

		case FIF_EXR:
			flags = compressAmt >= 50 ? EXR_DEFAULT : EXR_NONE;
			break;

		case FIF_JPEG: // default: JPEG_QUALITYGOOD|JPEG_SUBSAMPLING_420
			/*if(compressAmt <= 25) flags = JPEG_QUALITYSUPERB;
			else if(compressAmt <= 50) flags = JPEG_QUALITYGOOD;
			else if(compressAmt <= 75) flags = JPEG_QUALITYAVERAGE;
			else flags = JPEG_QUALITYBAD;*/
			flags = quality;
			break;

		case FIF_J2K:
		case FIF_JP2:
			flags = int(double(quality)*5.11 + 1); // convert [0,100] -> [1,512]
			break;

		case FIF_PNG: // default: PNG_Z_DEFAULT_COMPRESSION (= 6)
			compressAmt = (compressAmt + 5) / 10;
			if(compressAmt != 0) flags = compressAmt>=9 ? 9 : compressAmt;
			else flags = PNG_Z_NO_COMPRESSION;
			break;

		case FIF_TIFF:
			flags = compressAmt >= 50 ? TIFF_DEFAULT : TIFF_NONE;
			break;

		case FIF_TARGA:
			flags = compressAmt >= 50 ? TARGA_SAVE_RLE : TARGA_DEFAULT;
			break;

		default:
			flags = 0;
		}

		return FreeImage_Save(fileType, mImage, filename.c_str(), flags);
	}


	void getDim(int &w, int &h) {
		if(mImage) {
			w = FreeImage_GetWidth(mImage);
			h = FreeImage_GetHeight(mImage);
		}
		else {
			w = h = 0;
		}
	}

	int getPlanes() const {
		AlloTy type = getDataType();
		BITMAPINFOHEADER * hdr = FreeImage_GetInfoHeader(mImage);
		return (hdr->biBitCount)/(8*allo_type_size(type));
	}

	AlloTy getDataType() const {
		FREE_IMAGE_TYPE type = FreeImage_GetImageType(mImage);
		switch(type) {
			case FIT_UINT32: return AlloUInt32Ty;
			case FIT_INT32: return AlloSInt32Ty;

			case FIT_RGBF:
			case FIT_RGBAF:
			case FIT_FLOAT: return AlloFloat32Ty;
			case FIT_DOUBLE: return AlloFloat64Ty;

			case FIT_BITMAP:
			default: return AlloUInt8Ty;
		}
	}
protected:
	void destroy() {
		if (mImage) FreeImage_Unload(mImage);
		mImage = NULL;
	}

	FIBITMAP		*mImage;
};

Image :: Image()
: mImpl(0), mCompression(50), mLoaded(false)
{}

Image :: Image(const std::string& filename)
: mImpl(0), mFilename(filename), mCompression(50), mLoaded(false) {
	load(filename);
}

Image :: ~Image() {
	if (mImpl) delete mImpl;
}

bool Image :: load(const std::string& filename) {
	if (!mImpl) mImpl = new FreeImageImpl();
//	// TODO: if we add other image formats/libraries,
//	// detect by file extension & redirect to appropriate implementation here:
//	if (mImpl) delete mImpl;
//	mImpl = new FreeImageImpl();
	mLoaded = mImpl->load(filename, mArray);
	if (mLoaded) mFilename = filename;
	return mLoaded;
}

bool Image :: save(const std::string& filename) {
	if (!mImpl) mImpl = new FreeImageImpl();
//	// TODO: if we add other image formats/libraries,
//	// detect by file extension & redirect to appropriate implementation here:
//	if (mImpl) delete mImpl;
//	mImpl = new FreeImageImpl();
	mLoaded = mImpl->save(filename, mArray, mCompression);
	if (mLoaded) mFilename = filename;
	return mLoaded;
}

/*static*/ bool Image::save(
	const std::string& filePath, const Array& src, int compress
){
	/*return save(
		filePath,
		src.data.ptr, src.width(), src.height(), getFormat(src.components()),
		compress
	);*/

	Image img;
	Array& a = img.array();
	a.configure(src.header); // copy over header information
	a.data.ptr = src.data.ptr;
	img.compression(compress);
	bool res = img.save(filePath);
	a.data.ptr = NULL; // prevent ~Array from deleting data
	return res;
}

Image::Format Image::format() const {
	return getFormat(array().components());
}

Image::Format Image :: getFormat(int planes) {
	switch(planes) {
		case 1:		return LUMINANCE;
		case 2:		return LUMALPHA;
		case 3:		return RGB;
		case 4:		return RGBA;
		default:;
	}
	return UNKNOWN_FORMAT;
}

} // al::
