#ifndef INCLUDE_AL_IMAGE_HPP
#define INCLUDE_AL_IMAGE_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
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
*/

#include <string>
#include "allocore/types/al_Lattice.hpp"

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
	
	// shorthand for Image().load(filename)
	Image(std::string filename);
	
	// Image attempts to determine image type by file extension:
	bool load(std::string filename);
	bool save(std::string filename);
	
	bool loaded() const { return mLoaded; }
	const Lattice& lattice() { return mLattice; }
	
	static Format getFormat(int planes);

	class Impl {
	public:
		virtual ~Impl() {};
		virtual bool load(std::string filename, Lattice &lat) = 0;
		virtual bool save(std::string filename, Lattice &lat) = 0;
	};

protected: 
	Lattice mLattice;			// pixel data
	Impl * mImpl;				// library implementation
	std::string mFilename;
	bool mLoaded;				// true after image data is loaded

};

} // al::

#endif
