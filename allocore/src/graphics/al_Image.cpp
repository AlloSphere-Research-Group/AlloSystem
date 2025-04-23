#include "allocore/graphics/al_Image.hpp"
#include "allocore/system/al_Printing.hpp" // AL_WARN

#if defined(AL_IMAGE_DUMMY)

namespace al{

class Image::Impl {
public:
	Impl(){}
	~Impl(){}
	bool load(const std::string& filename, Array &arr){
		return false;
	}
	bool save(const std::string& filename, const Array& arr, int compressFlags, int paletteSize){
		return false;
	}
};

} // al::


#elif defined(AL_IMAGE_FREEIMAGE)

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

class Image::Impl {
public:
	Impl(){
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

	~Impl(){
		destroy();
	}

	bool load(const std::string& filename, Array& arr){
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
		mFIBitmap = FreeImage_Load(type, filename.c_str(), 0);
		if (mFIBitmap == NULL) {
			AL_WARN("image failed to load: %s", filename.c_str());
			return false;
		}

		FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(mFIBitmap);
		switch(colorType) {
			case FIC_MINISBLACK:
			case FIC_MINISWHITE: {
					FIBITMAP *res = FreeImage_ConvertToGreyscale(mFIBitmap);
					FreeImage_Unload(mFIBitmap);
					mFIBitmap = res;
				}
				break;

			case FIC_PALETTE: {
					if(FreeImage_IsTransparent(mFIBitmap)) {
						FIBITMAP *res = FreeImage_ConvertTo32Bits(mFIBitmap);
						FreeImage_Unload(mFIBitmap);
						mFIBitmap = res;
					}
					else {
						FIBITMAP *res = FreeImage_ConvertTo24Bits(mFIBitmap);
						FreeImage_Unload(mFIBitmap);
						mFIBitmap = res;
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
		//FreeImage_FlipVertical(mFIBitmap);

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
					char *pix = (char *)FreeImage_GetScanLine(mFIBitmap, j);
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
							RGBTRIPLE * pix = (RGBTRIPLE *)FreeImage_GetScanLine(mFIBitmap, j);
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
							char *pix = (char *)FreeImage_GetScanLine(mFIBitmap, j);
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
							RGBQUAD *pix = (RGBQUAD *)FreeImage_GetScanLine(mFIBitmap, j);
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
							char *pix = (char *)FreeImage_GetScanLine(mFIBitmap, j);
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

	// TODO
	bool load(const unsigned char * src, int len, Array& arr){
		return false;
	}


	void resize(int w, int h, int bpp){
		if(NULL != mFIBitmap){ // already allocated image?
			int oldw = FreeImage_GetWidth(mFIBitmap);
			int oldh = FreeImage_GetHeight(mFIBitmap);
			int oldbpp = FreeImage_GetBPP(mFIBitmap);
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
			mFIBitmap = FreeImage_AllocateT(FIT_BITMAP, w, h, bpp);
		}
	}


	bool save(const std::string& filename, const Array& arr, int compressFlags, int paletteSize){

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

		if (mFIBitmap == NULL) {
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
								FreeImage_GetScanLine(mFIBitmap, j),
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
								FreeImage_GetScanLine(mFIBitmap, j),
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
							RGBTRIPLE * dst = (RGBTRIPLE *)FreeImage_GetScanLine(mFIBitmap, j);
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
							RGBQUAD * dst = (RGBQUAD *)FreeImage_GetScanLine(mFIBitmap, j);
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

		auto * outputBitmap = mFIBitmap;

		if(paletteSize > 1){
			outputBitmap = FreeImage_ColorQuantizeEx(mFIBitmap, FIQ_WUQUANT, paletteSize);
		}

		auto result = FreeImage_Save(fileType, outputBitmap, filename.c_str(), flags);
		if(outputBitmap != mFIBitmap) FreeImage_Unload(outputBitmap);
		return result;
	}


	void getDim(int &w, int &h){
		if(mFIBitmap) {
			w = FreeImage_GetWidth(mFIBitmap);
			h = FreeImage_GetHeight(mFIBitmap);
		}
		else {
			w = h = 0;
		}
	}

	int getPlanes() const {
		AlloTy type = getDataType();
		BITMAPINFOHEADER * hdr = FreeImage_GetInfoHeader(mFIBitmap);
		return (hdr->biBitCount)/(8*allo_type_size(type));
	}

	AlloTy getDataType() const {
		FREE_IMAGE_TYPE type = FreeImage_GetImageType(mFIBitmap);
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
	void destroy(){
		if (mFIBitmap) FreeImage_Unload(mFIBitmap);
		mFIBitmap = NULL;
	}

	FIBITMAP * mFIBitmap = NULL;
};

} // al::
// End FreeImage implementation


#else

#include <cctype> // tolower
#include <fstream> // ofstream
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

namespace al{

class Image::Impl {
public:

	// Reader suppports: JPG PNG BMP TGA GIF HDR PSD PIC PNM
	// Writer suppports: JPG PNG BMP TGA GIF HDR

	template <class Func, class... Args>
	bool loadData(Array& arr, Func loadFunc, Args... args){
		stbi_set_flip_vertically_on_load(1); // rows go bottom to top
		int w,h,n;
		// components always uint8_t
		unsigned char * data = loadFunc(args..., &w, &h, &n, 0);
		if(data){
			arr.formatAligned(n, AlloUInt8Ty, w,h, 1);
			memcpy(arr.data.ptr, data, w*h*n);
			stbi_image_free(data);
			return true;
		}
		return false;
	}

	bool load(const std::string& filePath, Array& arr){
		return loadData(arr, stbi_load, filePath.c_str());
	}

	bool load(const unsigned char * src, int len, Array& arr){
		return loadData(arr, stbi_load_from_memory, src, len);
	}

	bool save(const std::string& filePath, const Array& pix, int compressFlags, int paletteSize){
		auto extPos = filePath.find_last_of('.');
		if(extPos == std::string::npos){
			AL_WARN("No file extension found");
			return false;
		}

		auto ext = filePath.substr(extPos+1);
		for(auto& c:ext) c=std::tolower(c);

		FileType ftype;
		if("png" == ext) ftype = PNG;
		else if("jpg" == ext || "jpeg" == ext) ftype = JPG;
		else if("tga" == ext) ftype = TGA;
		else if("bmp" == ext) ftype = BMP;
		else {
			AL_WARN("Cannot match extension to a supported file type");
			return false;
		}

		ByteArray bytes;
		save(ftype, bytes, pix, compressFlags, paletteSize);

		std::ofstream fs(filePath, std::ios::binary);
		if(!fs.good()){
			AL_WARN("Cannot open file for writing");
			return false;
		}
		fs.write((char *)bytes.data(), bytes.size());

		return true;
	}

	bool save(FileType fileType, ByteArray& dst, const Array& pix, int compressFlags, int paletteSize){
		auto compressAmt = compressFlags & 127;
		if(compressAmt > 100) compressAmt=100;
		auto w = pix.dim(0);
		auto h = (pix.dimcount() > 1) ? pix.dim(1) : 1;
		auto n = pix.components();
		auto s = pix.stride(0);
		const void * data = pix.data.ptr;
		std::vector<unsigned char> packui8;
		if(pix.type() != AlloUInt8Ty || s != w*n){
			switch(pix.type()){
				#define CS(T, op)\
				case Array::type<T>():\
				for(unsigned j=0; j<h; ++j){\
				for(unsigned i=0; i<w; ++i){\
				for(unsigned c=0; c<n; ++c){\
					packui8.push_back(pix.elem<T>(c,i,j) op);\
				}}} break;
				CS(uint8_t,);
				CS(uint16_t, >>16);
				CS(uint32_t, >>24);
				CS(uint64_t, >>56);
				CS(float, *255.f);
				CS(double, *255.);
				default:
					AL_WARN("Data type not supported");
					return false;
			}
			data = &packui8[0];
		}

		stbi_flip_vertically_on_write(1); // rows go bottom to top

		dst.clear();

		auto writeFunc = [](void * ctx, void * buf, int size){
			auto * bytes = (ByteArray::value_type *)buf;
			auto& dst = *(ByteArray *)ctx;
			dst.insert(dst.end(), bytes, bytes+size);
		};

		switch(fileType){
		case PNG:
			stbi_write_png_compression_level = compressAmt;
			return stbi_write_png_to_func(writeFunc, &dst, w,h,n, data, w*n);
		case JPG:
			return stbi_write_jpg_to_func(writeFunc, &dst, w,h,n, data, 100-compressAmt);
		case TGA:
			stbi_write_tga_with_rle = compressAmt >= 50;
			return stbi_write_tga_to_func(writeFunc, &dst, w,h,n, data);
		case BMP:
			return stbi_write_bmp_to_func(writeFunc, &dst, w,h,n, data);
		default:;
		}

		return false;
	}
};

} // al::

#endif


namespace al{

Image::Image(){}

Image::Image(const std::string& filename){
	load(filename);
}

Image::~Image() {
	if(mImpl) delete mImpl;
}

bool Image::load(const std::string& filename){
	if(!mImpl) mImpl = new Impl();
	mLoaded = mImpl->load(filename, mArray);
	if(mLoaded) mFilename = filename;
	return mLoaded;
}

bool Image::load(const unsigned char * src, int len){
	if(!mImpl) mImpl = new Impl();
	mLoaded = mImpl->load(src, len, mArray);
	return mLoaded;
}

bool Image::save(const std::string& filename){
	if(!mImpl) mImpl = new Impl();
	mLoaded = mImpl->save(filename, mArray, mCompression, mPaletteSize);
	if(mLoaded) mFilename = filename;
	return mLoaded;
}

bool Image::save(FileType fileType, ByteArray& buffer){
	if(!mImpl) mImpl = new Impl();
	return mImpl->save(fileType, buffer, mArray, mCompression, mPaletteSize);
}

template <class OnSave>
bool saveFromArray(const Array& src, int compress, int paletteSize, const OnSave& onSave){
	if(src.empty()) AL_WARN("Source al::Array is empty");
	Image img;
	Array& a = img.array();
	a.configure(src.header); // copy over header information
	a.data.ptr = src.data.ptr;
	img.compression(compress);
	img.paletteSize(paletteSize);
	bool res = onSave(img);
	a.data.ptr = nullptr; // prevent ~Array from deleting data
	return res;
}

/*static*/ bool Image::save(
	const std::string& filePath, const Array& src, int compress, int paletteSize
){
	/*return save(
		filePath,
		src.data.ptr, src.width(), src.height(), getFormat(src.components()),
		compress
	);*/

	/*Image img;
	Array& a = img.array();
	a.configure(src.header); // copy over header information
	a.data.ptr = src.data.ptr;
	img.compression(compress);
	img.paletteSize(paletteSize);
	bool res = img.save(filePath);
	a.data.ptr = NULL; // prevent ~Array from deleting data
	return res;*/

	return saveFromArray(src, compress, paletteSize, [&](auto& img){
		return img.save(filePath);
	});
}

/*static*/ bool Image::save(
	FileType t, ByteArray& dst, const Array& src, int compress, int paletteSize
){
	return saveFromArray(src, compress, paletteSize, [&](auto& img){
		return img.save(t, dst);
	});
}


Image::Format Image::format() const {
	return getFormat(array().components());
}

Image::Format Image::getFormat(int planes){
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
