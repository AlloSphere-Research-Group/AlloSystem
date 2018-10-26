#include <fstream>
#include <vector>
#include "allocore/system/al_Time.hpp"
#include "allocore/graphics/al_OpenGL.hpp"
#include "allocore/graphics/al_Screenshot.hpp"

namespace al{

bool Screenshot::save(unsigned w, unsigned h, const std::string& filePath){
	auto derivedFilePath = filePath;
	if(derivedFilePath.empty()){
		derivedFilePath = mPath + "screenshot_" + timecodeNow("D_HMS") + "." + mExt;
	}

	std::ofstream fs(derivedFilePath, std::ofstream::out | std::ofstream::binary);

	if(!fs.is_open()) return false;

	std::vector<unsigned char> pix(w*h*3, 0);
	glReadPixels(0,0, w,h, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)&pix[0]);
	//printf("%d %d %d\n", pix[0], pix[1], pix[2]);

	auto lsB = [](unsigned x){ return (unsigned char)(x&255); };
	auto msB = [](unsigned x){ return (unsigned char)((x>>8)&255); };

	// TGA (simplest format, widely supported)
	// http://www.paulbourke.net/dataformats/tga/
	unsigned char hdr[18] = {
		0,			// image ID length
		0,			// color map
		2,			// uncompressed true-color
		0,0,0,0,0,	// color map
		0,0, 0,0,	// image origin
		lsB(w), msB(w), lsB(h), msB(h), // width,height in pixels
		24,			// bits/pixel
		0			// descriptor: 00vhaaaa - v/h vert/hor flip, a alpha bits
	};
	fs.write((char*)hdr, sizeof(hdr));
	// Annoyingly, TGA is little endian, so we have to write BGR
	for(unsigned i=0; i<pix.size(); i+=3) std::swap(pix[i], pix[i+2]);
	fs.write((char*)&pix[0], pix.size());

	// Other potential formats to support in future...

	// PNG (really complicated)
	//unsigned char hdr[] = {137,'P','N','G',13,10,26,10};

	// PBM (simple, but ASCII format so big files)

	/* TIFF (a bit complicated)
	// http://paulbourke.net/dataformats/tiff/
	// http://www.fileformat.info/format/tiff/corion.htm
	fs.open(RUN_MAIN_SOURCE_PATH "screenshot.tif.txt");
	unsigned char hdr[] = {
		'M','M',0,42,0,0,0,8, // big endian
		0x01,0x00, 0,3, 0,0,0,1, msB(w), lsB(w), 0,0), // width
		0x01,0x01, 0,3, 0,0,0,1, msB(h), lsB(h), 0,0), // height
		0x01,0x02, 0,3, 0,0,0,1, 0, 24, 0,0, // bits/sample
		0x01,0x03, 0,3, 0,0,0,1, 0,  1, 0,0, // compression
		0x01,0x06, 0,3, 0,0,0,1, 0,  2, 0,0, // RGB
	};
	//fs.write((char*)hdr,8);
	//*/

	return true;
}

} // al::
