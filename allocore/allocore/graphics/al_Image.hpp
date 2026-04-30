#ifndef INC_AL_IMAGE_HPP
#define INC_AL_IMAGE_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Loads and saves images

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <vector>
#include <string>
#include "allocore/types/al_Array.hpp"

namespace al{

/// Loads and saves images.
///
/// \ingroup allocore
class Image {
public:
	/*!
		Image data formats.
	*/
	enum Format {
		FORMAT_INVALID = 0,
		LUMINANCE,		//!< Luminance (1-plane)
		LUMALPHA,		//!< Luminance/alpha (2-plane)
		RGB,			//!< RGB (3-plane)
		RGBA			//!< RGBA (4-plane)
	};

	enum FileType {
		FILE_TYPE_INVALID = 0,
		PNG,
		TGA,
		BMP,
		JPG,
		FILE_TYPE_UNKNOWN
	};

	template<typename T>
	struct RGBPix { T r, g, b; };

	template<typename T>
	struct RGBAPix { T r, g, b, a; };

	using ByteArray = std::vector<unsigned char>;


	Image();

	/// \param[in] filePath		Image file to load
	Image(const std::string& filePath);

	~Image();


	/// Load image from disk

	/// \param[in] filePath		File to load. Image type determined by file 
	///							extension.
    /// \returns detected file type
	FileType load(const std::string& filePath);

	/// Load image from memory source

	/// \param[in] src		Source array
	/// \param[in] len		Number of bytes
    /// \returns detected file type
	FileType load(const unsigned char * src, int len);
	FileType load(const ByteArray& src){ return load(&src[0], src.size()); }


	/// Save image to disk

	/// \param[in] filePath		File to save. Image type determined by file 
	///							extension.
    /// \returns true for success or print error message and return false
	bool save(const std::string& filePath);

	/// Save image file data to raw byte array
	bool save(FileType t, ByteArray& dst);

	/// Save pixel data to disk

	/// \param[in] filePath		File to save. Image type determined by file 
	///							extension.
	/// \param[in] src			source array containing pixel data
	/// \param[in] compressFlags level of compression in [0,100] and other flags
	/// \param[in] paletteSize	number of colors in palette, in [2,256]
	static bool save(const std::string& filePath, const Array& src, int compressFlags=50, int paletteSize=-1);

	/// Save pixel data to memory
	static bool save(FileType t, ByteArray& dst, const Array& src, int compressFlags=50, int paletteSize=-1);

	/// Save pixel data to disk

	/// \param[in] filePath		File to save. Image type determined by file 
	///							extension.
	/// \param[in] pixels		pixel data
	/// \param[in] nx			number of pixels along the x dimension
	/// \param[in] ny			number of pixels along the y dimension
	/// \param[in] nc			number of components
	/// \param[in] compressFlags level of compression in [0,100] and other flags
	/// \param[in] paletteSize	number of colors in palette, in [2,256]
	template <class T>
	static bool save(const std::string& filePath, const T * pixels, int nx, int ny, int nc, int compressFlags=50, int paletteSize=-1);

	/// Save pixel data to memory
	template <class T>
	static bool save(FileType t, ByteArray& dst, const T * pixels, int nx, int ny, int nc, int compressFlags=50, int paletteSize=-1);


	/// File path to image
	const std::string& filepath() const { return mFilename; }

	/// Whether image was loaded from file
	bool loaded() const { return mLoaded; }


	/// Get pixels as an Array
	Array& array(){ return mArray; }

	/// Get pixels as an Array (read-only)
	const Array& array() const { return mArray; }

	/// Get pointer to pixel data
	template <typename T = void>
	T * data(){ return (T*)(mArray.data.ptr); }

	/// Get pointer to pixel data (read-only)
	template <typename T = void>
	const T * data() const { return const_cast<Image*>(this)->data<T>(); }


	/// Get number of bytes per pixel
	unsigned bytesPerPixel() const { return allo_type_size(mArray.type()) * mArray.components(); }

	/// Get pixel format
	Format format() const;

	/// Get width, in pixels
	unsigned width() const { return mArray.width(); }

	/// Get height, in pixels
	unsigned height() const { return mArray.height(); }

	/// Get number of pixel color components
	unsigned components() const { return mArray.components(); }

	/// Get total size of pixel data, in bytes
	unsigned sizeBytes() const { return mArray.size(); }

	/// Returns whether the internal pixel buffer is empty
	bool empty() const { return mArray.empty(); }


	/// Get compression flags for saving
	int compression() const { return mCompression; }

	/// Set compression flags for saving

	/// The flags consist of a bitwise-or of the level of compression in [0,100]
	/// and other flags which may be specific to the image format.
	Image& compression(int flags){ mCompression=flags; return *this; }

	/// Get color palette size
	int paletteSize() const { return mPaletteSize; }

	/// Set color palette size

	/// \param[in] numColors	number of colors in palette, in [2,256]
	///
	Image& paletteSize(int numColors){ mPaletteSize=numColors; return *this; }

	/// Get mutable reference to a pixel
	template<typename Pix>
	Pix& at(unsigned x, unsigned y){
		return *mArray.cell<Pix>(x, y);
	}

	/// Get read-only reference to a pixel
	template<typename Pix>
	const Pix& at(unsigned x, unsigned y) const {
		return const_cast<Image*>(this)->at<Pix>(x,y);
	}

	/// Resize internal pixel buffer. Erases any existing data.

	/// \tparam T			component type
	/// \param[in] dimX		number of pixels in x direction
	/// \param[in] dimY		number of pixels in y direction
	/// \param[in] format	pixel color format
	/// \returns True on success; false otherwise.
	template <typename T>
	bool resize(int dimX, int dimY, Format format){
		mArray.formatAligned(components(format), Array::type<T>(), dimX, dimY, 1);
		return true;
	}

	/// Clear internal pixel buffer
	Image& clear(){ mArray.dataFree(); return *this; }


	/// Get number of components per pixel element
	static int components(Format v);

	static Format getFormat(int planes);

protected:
	class Impl;
	Impl * mImpl = NULL;	// backend implementation
	Array mArray;			// pixel data
	std::string mFilename;
	int mCompression = 50;
	int mPaletteSize = -1;	// number of colors in palette
	bool mLoaded = false;	// true after image data is loaded

	template <class T, class OnArray>
	static bool arrayScope(const T * pixels, int nx, int ny, int nc, const OnArray& onArray){
		if(!pixels) return false;
		if(nx<=0 || ny<=0) return false;
		if(nc<=0 || nc>=5) return false;
		Array a;
		a.ref(const_cast<T *>(pixels), nc, nx, ny);
		return onArray(a);
	}
};




// Implementation ______________________________________________________________

template <class T>
/*static*/ bool Image::save(
	const std::string& filePath, const T * pixels, int nx, int ny, int nc, int compress, int paletteSize
){
	return arrayScope(pixels, nx,ny,nc, [&](auto& a){
		return save(filePath, a, compress, paletteSize);
	});
}

template <class T>
/*static*/ bool Image::save(
	FileType t, ByteArray& dst, const T * pixels, int nx, int ny, int nc, int compress, int paletteSize
){
	return arrayScope(pixels, nx,ny,nc, [&](auto& a){
		return save(t, dst, a, compress, paletteSize);
	});
}

} // al::
#endif
