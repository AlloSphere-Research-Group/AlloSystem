#ifndef INCLUDE_AL_IMAGE_HPP
#define INCLUDE_AL_IMAGE_HPP

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
	Loads and saves images
	
	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <string>
#include "allocore/types/al_Array.hpp"

namespace al{

/*!
	\class Image

	Loads and saves images.  
	
	Default implementation uses the FreeImage library. Supported formats include:

		bmp, chead, cut, dcx, dds, doom, doomFlat, exif, gif, hdr, ico, jasc_pal, jpg,
		lbm, lif, mdl, pcd, pcx, pic, pix, png, pnm, psd, psp, pxr, raw, sgi, tgo, tif,
		wal, xpm
		
	FreeImage is used under the FreeImage Public License (FIPL) v1.0
	See the /licenses folder in the source tree, or
	http://freeimage.sourceforge.net/freeimage-license.txt
*/
class Image {
public:
	/*!
		Image data formats.
	*/
	enum Format {
		LUMINANCE = 0,	//!< luminance (1-plane)
		LUMALPHA,		//!< lumalpha (2-plane)
		RGB,			//!< rgb (3-plane)
		RGBA,			//!< rgba (4-plane)
		UNKNOWN_FORMAT
	};
	
	template<typename T>
	struct RGBPix { T r, g, b; };

	template<typename T>
	struct RGBAPix { T r, g, b, a; };


	Image();

	/// @param[in] filename		file name of image; loaded automatically
	Image(const std::string& filename);

	~Image();


	/// Load image with file name. Image type determined by file extension.

        // return true for success or print error message and return false
	bool load(const std::string& filename);
	
	/// Save image with file name. Image type determined by file extension. 

        // return true for success or print error message and return false
	bool save(const std::string& filename);
	
	/// File path to image
	const std::string& filepath() const { return mFilename; }
	
	/// Whether an image was loaded from file
	bool loaded() const { return mLoaded; }


	/// Get pixels as an Array
	Array& array(){ return mArray; }

	/// Get pixels as an Array (read-only)
	const Array& array() const { return mArray; }

	/// Get pointer to pixels
	template <typename T>
	T * pixels(){ return (T*)(mArray.data.ptr); }

	/// Get pointer to pixels (read-only)
	template <typename T>
	const T * pixels() const { return (const T*)(mArray.data.ptr); }


	/// Get number of bytes per pixel
	unsigned bytesPerPixel() const { return allo_type_size(array().type()) * array().components(); }

	/// Get pixel format
	Format format() const;

	/// Get width, in pixels
	unsigned width() const { return array().width(); }
	
	/// Get height, in pixels
	unsigned height() const { return array().height(); }


	/// Write a pixel to an Image

	/// Warning: doesn't check that Pix has matching type/component count
	/// Warning: no bounds checking performed on x and y
	template<typename Pix>
	void write(const Pix& pix, unsigned x, unsigned y) {
		array().write(&pix.r, x, y); 
	}

	/// Quick image writing function
	
	/// @param[in] filePath		file path
	/// @param[in] pixels		pixel data
	/// @param[in] nx			number of pixels along the x dimension
	/// @param[in] ny			number of pixels along the y dimension
	/// @param[in] fmt			pixel format
	template <class T>
	static bool write(const std::string& filePath, const T * pixels, int nx, int ny, Format fmt);

	/// Read a pixel from an Image

	/// Warning: doesn't check that Pix has matching type/component count
	/// Warning: no bounds checking performed on x and y
	template<typename Pix>
	void read(Pix& pix, unsigned x, unsigned y) const {
		array().read(&pix.r, x, y); 
	}

	/// Resize internal pixel buffer. Erases any existing data.
	
	/// @param[in] dimX		number of pixels in x direction
	/// @param[in] dimY		number of pixels in y direction
	/// @param[in] format	pixel color format
	/// \returns True on success; false otherwise.
	template <typename T>
	bool resize(int dimX, int dimY, Format format){
		mArray.formatAligned(components(format), Array::type<T>(), dimX, dimY, 1);
		return true;
	}


	/// Get number of components per pixel element
	static int components(Format v);
	
	static Format getFormat(int planes);

	class Impl {
	public:
		virtual ~Impl() {};
		virtual bool load(const std::string& filename, Array& lat) = 0;
		virtual bool save(const std::string& filename, const Array& lat) = 0;
	};

protected:
	Array mArray;			// pixel data
	Impl * mImpl;			// library implementation
	std::string mFilename;
	bool mLoaded;			// true after image data is loaded
};




// Implementation ______________________________________________________________
inline int Image::components(Format v){
	switch(v){
	case LUMINANCE:	return 1;
	case LUMALPHA:	return 2;
	case RGB:		return 3;
	case RGBA:		return 4;
	default:;
	}
	return 0;
}

template <class T>
bool Image::write(const std::string& filePath, const T * pixels, int nx, int ny, Format fmt){
	Image img;
	Array& a = img.array();
	a.data.ptr			= (char *)const_cast<T *>(pixels);
	a.header.type		= Array::type<T>();
	a.header.components	= Image::components(fmt);
	allo_array_setdim2d(&a.header, nx, ny);
	allo_array_setstride(&a.header, 1);
	bool res = img.save(filePath);
	a.data.ptr = NULL; // prevent ~Array from deleting data
	return res;
}


} // al::

#endif
