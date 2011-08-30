#ifndef INCLUDE_AL_IMAGE_HPP
#define INCLUDE_AL_IMAGE_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


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
		RGBA			//!< rgba (4-plane)
	};
	
	template<typename T>
	struct RGBPix { T r, g, b; };
	template<typename T>
	struct RGBAPix { T r, g, b, a; };

	Image();
	~Image();

	/// @param[in] filename		file name of image; loaded automatically
	Image(const std::string& filename);


	/// Load image with file name. Image type determined by file extension.
	bool load(const std::string& filename);
	
	/// Save image with file name. Image type determined by file extension.
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


	/// Resize internal pixel buffer. Erases any existing data.
	
	/// @param[in] dimX		number of pixels in x direction
	/// @param[in] dimY		number of pixels in y direction
	/// @param[in] format	pixel color format
	/// \returns True on success; false otherwise.
	template <typename T>
	bool resize(int dimX, int dimY, Format format){
		mArray.formatAligned(components(format), Array::type<T>(), dimX, dimY, 0);
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


} // al::

#endif
