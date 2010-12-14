#include <stdio.h>

#include "allocore/graphics/al_Image.hpp"
#include "allocore/system/al_Config.h"

#include "FreeImage.h"
/*!
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
	FreeImageImpl() : mImage(NULL) {
		if (!initializedFreeImage) {
			FreeImage_Initialise();
			initializedFreeImage = 1;
		}
	}
	virtual ~FreeImageImpl() {
	}
	
	virtual bool load(std::string filename, Array &lat) {
		FREE_IMAGE_FORMAT type = FreeImage_GetFIFFromFilename(filename.data());
		if(type == FIF_UNKNOWN) {
			printf("image format not recognized: %s\n", filename.data());
			return false;
		}
		if(!FreeImage_FIFSupportsReading(type)) {
			printf("image format not supported: %s\n", filename.data());
			return false;
		}
		
		destroy();
		mImage = FreeImage_Load(type, filename.data(), 0);
		if (mImage == NULL) {
			printf("image failed to load: %s\n", filename.data());
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
					printf("CMYK images currently not supported\n");
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
								#if (defined(AL_LINUX) || defined(AL_OSX))
								o_pix->r = pix->rgbtRed;
								o_pix->g = pix->rgbtGreen;
								o_pix->b = pix->rgbtBlue;
								#else
								o_pix->r = pix->rgbtBlue;
								o_pix->g = pix->rgbtGreen;
								o_pix->b = pix->rgbtRed;
								#endif
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
								#if (defined(AL_LINUX) || defined(AL_OSX))
								o_pix->r = pix->rgbRed;
								o_pix->g = pix->rgbGreen;
								o_pix->b = pix->rgbBlue;
								o_pix->a = pix->rgbReserved;
								#else
								o_pix->r = pix->rgbBlue;
								o_pix->g = pix->rgbGreen;
								o_pix->b = pix->rgbRed;
								o_pix->a = pix->rgbReserved;
								#endif

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
				printf("image data not understood\n");
				destroy();
				return false;
		}
		return true;
	}
	
	virtual bool save(std::string filename, Array &lat) {
	
		// check existing image type
		FREE_IMAGE_FORMAT type = FreeImage_GetFIFFromFilename(filename.data());
		
		if(type == FIF_UNKNOWN) {
			printf("image format not recognized: %s\n", filename.data());
			return false;
		}
		if(!FreeImage_FIFSupportsWriting(type)) {
			printf("image format not supported: %s\n", filename.data());
			return false;
		}
		
		destroy();
		
		AlloArrayHeader& header = lat.header;
		int w = header.dim[0];
		int h = (header.dimcount > 1) ? header.dim[1] : 1;
		Image::Format format = Image::getFormat(header.components);

		switch(format) {
			case Image::RGB: {
				switch(header.type) {

					case AlloUInt8Ty: {
						int bpp = header.stride[0]; 
						mImage = FreeImage_AllocateT(FIT_BITMAP, w, h, bpp*8);
						char *bp = (char *)(lat.data.ptr);
						int rowstride = header.stride[1]; 

						for(unsigned j = 0; j < header.dim[1]; ++j) {
							RGBTRIPLE *pix = (RGBTRIPLE *)FreeImage_GetScanLine(mImage, j);
							Image::RGBPix<uint8_t> *o_pix = (Image::RGBPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < header.dim[0]; ++i) {
								pix->rgbtRed = o_pix->r;
								pix->rgbtGreen = o_pix->g;
								pix->rgbtBlue = o_pix->b;

								++pix;
								++o_pix;
							}
						}
					}
					break;

					default:
						printf("Input Matrix type not supported\n");
						break;
				}
			}
			break;

			case Image::RGBA: {
				switch(header.type) {

					case AlloUInt8Ty: {
						int bpp = header.stride[0]; 
						mImage = FreeImage_AllocateT(FIT_BITMAP, w, h, bpp*8);
						char *bp = (char *)(lat.data.ptr);
						int rowstride = header.stride[1]; 

						for(unsigned j = 0; j < header.dim[1]; ++j) {
							RGBQUAD *pix = (RGBQUAD *)FreeImage_GetScanLine(mImage, j);
							Image::RGBAPix<uint8_t> *o_pix = (Image::RGBAPix<uint8_t> *)(bp + j*rowstride);
							for(unsigned i=0; i < header.dim[0]; ++i) {
								pix->rgbRed = o_pix->b;
								pix->rgbGreen = o_pix->g;
								pix->rgbBlue = o_pix->r;
								pix->rgbReserved = o_pix->a;

								++pix;
								++o_pix;
							}
						}
					}
					break;

					default:
						printf("Input Matrix type not supported\n");
						return false;
				}
			}
			break;

			default: {
				printf("Input Matrix format not supported\n");
				return false;
			}
		}
		
		if (mImage == NULL) {
			printf("Image could not be understood\n");
			return false;
		}
		
		return FreeImage_Save(type, mImage, filename.data(), 0);
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
	
	int getPlanes() {
		AlloTy type = getDataType();
		BITMAPINFOHEADER * hdr = FreeImage_GetInfoHeader(mImage);
		return (hdr->biBitCount)/(8*allo_type_size(type));
	}
	
	AlloTy getDataType() {
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
: mImpl(0), mLoaded(false)
{}

Image :: Image(std::string filename) 
: mImpl(0), mFilename(filename), mLoaded(false) {
	load(filename);
}

Image :: ~Image() {
	if (mImpl) delete mImpl;
}
	
bool Image :: load(std::string filename) {
	if (!mImpl) mImpl = new FreeImageImpl();
//	// TODO: if we add other image formats/libraries, 
//	// detect by file extension & redirect to appropriate implementation here:
//	if (mImpl) delete mImpl;
//	mImpl = new FreeImageImpl();
	mLoaded = mImpl->load(filename, mArray);
	if (mLoaded) mFilename = filename;
	return mLoaded;
}

bool Image :: save(std::string filename) {
	if (!mImpl) mImpl = new FreeImageImpl();
//	// TODO: if we add other image formats/libraries, 
//	// detect by file extension & redirect to appropriate implementation here:
//	if (mImpl) delete mImpl;
//	mImpl = new FreeImageImpl();
	mLoaded = mImpl->save(filename, mArray);
	if (mLoaded) mFilename = filename;
	return mLoaded;
}


Image::Format Image :: getFormat(int planes) {
	switch(planes) {
		case 1:		return LUMINANCE;
		case 2:		return LUMALPHA;
		case 3:		return RGB;
		case 4:
		default:	return RGBA;
	}
}

} // al::
