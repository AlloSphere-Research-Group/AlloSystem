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

static int initializedFreeImage = 0;

class FreeImageImpl : public Image::Impl {
public:
	FreeImageImpl()
	:	mImage(NULL)
	{
		if (!initializedFreeImage) {
			FreeImage_Initialise();
			initializedFreeImage = 1;
		}
	}
	virtual ~FreeImageImpl() {
		destroy();
	}
	
	virtual bool load(const std::string& filename, Array &lat) {
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
		lat.format(planes, ty, w, h);
		
		Image::Format format = Image::getFormat(planes);
		switch(format) {
			case Image::LUMINANCE: {
				char *o_pix = (char *)(lat.data.ptr);
				int rowstride = lat.header.stride[1];
				for(unsigned j = 0; j < lat.header.dim[1]; ++j) {
					char *pix = (char *)FreeImage_GetScanLine(mImage, j);
					memcpy(o_pix, pix, rowstride);
					o_pix += rowstride;
				}
			}
			break;
			
			case Image::RGB: {
				switch(lat.header.type) {
					case AlloUInt8Ty: {
						char *bp = (char *)(lat.data.ptr);
						int rowstride = lat.header.stride[1];

						for(unsigned j = 0; j < lat.header.dim[1]; ++j) {
							RGBTRIPLE * pix = (RGBTRIPLE *)FreeImage_GetScanLine(mImage, j);
							Image::RGBPix<uint8_t> *o_pix = (Image::RGBPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < lat.header.dim[0]; ++i) {
								o_pix->r = pix->rgbtRed;
								o_pix->g = pix->rgbtGreen;
								o_pix->b = pix->rgbtBlue;
								++pix;
								++o_pix;
							}
						}
					}
					break;

					case AlloFloat32Ty: {
						char *o_pix = (char *)(lat.data.ptr);
						int rowstride = lat.header.stride[1];

						for(unsigned j = 0; j < lat.header.dim[1]; ++j) {
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
				switch(lat.header.type) {
					case AlloUInt8Ty: {
						char *bp = (char *)(lat.data.ptr);
						int rowstride = lat.header.stride[1];
						for(unsigned j = 0; j < lat.header.dim[1]; ++j) {
							RGBQUAD *pix = (RGBQUAD *)FreeImage_GetScanLine(mImage, j);
							Image::RGBAPix<uint8_t> *o_pix = (Image::RGBAPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < lat.header.dim[0]; ++i) {
								o_pix->r = pix->rgbRed;
								o_pix->g = pix->rgbGreen;
								o_pix->b = pix->rgbBlue;
								o_pix->a = pix->rgbReserved;
								++pix;
								++o_pix;
							}
						}
					}
					break;

					case AlloFloat32Ty: {
						char *o_pix = (char *)(lat.data.ptr);
						int rowstride = lat.header.stride[1];
						for(unsigned j = 0; j < lat.header.dim[1]; ++j) {
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


	virtual bool save(const std::string& filename, const Array& lat, int compressFlags) {
	
		// check existing image type
		FREE_IMAGE_FORMAT type = FreeImage_GetFIFFromFilename(filename.c_str());
		
		if(type == FIF_UNKNOWN) {
			AL_WARN("image format not recognized: %s", filename.c_str());
			return false;
		}
		if(!FreeImage_FIFSupportsWriting(type)) {
			AL_WARN("image format not supported: %s", filename.c_str());
			return false;
		}

		const AlloArrayHeader& header = lat.header;
		unsigned w = header.dim[0];
		unsigned h = (header.dimcount > 1) ? header.dim[1] : 1;
		Image::Format format = Image::getFormat(header.components);
		int bpp = header.stride[0]*8;

		resize(w,h,bpp);

		/* Note that according to the FreeImage documentation, the byte ordering
		of the pixels returned from FreeImage_GetScanLine are not portable.
		This means we cannot simply do a memcpy of each scan line, but must
		rather copy the RGB(A) components one pixel at a time.
		*/
		switch(format) {
			case Image::RGB: {
				switch(header.type) {

					case AlloUInt8Ty: { //printf("FreeImageImpl: save uint8/RGB\n");
						char *bp = (char *)(lat.data.ptr);
						int rowstride = header.stride[1]; 

						for(unsigned j = 0; j < h; ++j) {
							
							// copy Array row to image buffer
							/*memcpy(
								FreeImage_GetScanLine(mImage, j),
								bp + j*rowstride,
								w*3
							);*/

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
						break;
				}
			}
			break;

			case Image::RGBA: {
				
				switch(header.type) {

					case AlloUInt8Ty: {						
						char *bp = (char *)(lat.data.ptr);
						int rowstride = header.stride[1]; 
						
						for(unsigned j = 0; j < h; ++j) {

							// copy Array row to image buffer
							/*memcpy(
								FreeImage_GetScanLine(mImage, j),
								bp + j*rowstride,
								w*4
							);*/
							
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
		
		
		if (mImage == NULL) {
			AL_WARN("image could not be understood");
			return false;
		}

		int flags;
		int compressAmt = compressFlags & 127;

		switch(type){
		case FIF_JPEG: // default: JPEG_QUALITYGOOD|JPEG_SUBSAMPLING_420
			if(compressAmt <= 25) flags = JPEG_QUALITYSUPERB;
			else if(compressAmt <= 50) flags = JPEG_QUALITYGOOD;
			else if(compressAmt <= 75) flags = JPEG_QUALITYAVERAGE;
			else flags = JPEG_QUALITYBAD;
			break;

		case FIF_PNG: // default: PNG_Z_DEFAULT_COMPRESSION (= 6)
			compressAmt = (compressAmt + 5) / 10;
			if(compressAmt != 0) flags = compressAmt>=9 ? 9 : compressAmt;
			else flags = PNG_Z_NO_COMPRESSION;
			break;

		default:
			flags = 0;
		}

		return FreeImage_Save(type, mImage, filename.c_str(), flags);
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
