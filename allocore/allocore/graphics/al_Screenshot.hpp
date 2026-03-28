#ifndef INC_AL_SCREENSHOT_HPP
#define INC_AL_SCREENSHOT_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Screenshot utility (with minimal dependencies)

	Author(s):
	Lance Putnam, 2018, putnam.lance@gmail.com
*/

#include <string>

namespace al{

/// Save screen pixels to an image file
class Screenshot{
public:

	/// Set directory for output file (must include trailing slash)
	Screenshot& dir(const std::string& v){ mDir=v; return *this; }

	/// Get output directory
	const std::string& dir() const { return mDir; }

	/// Get output file extension
	const std::string& fileExt() const { return mExt; }

	/// Set format of image file
	//Screenshot& imageFormat(const std::string& ext){ mExt=ext; return *this; }

	/// Save screen pixels to an image file

	/// @param[in] w			width of capture, in pixels
	/// @param[in] h			height of capture, in pixels
	/// @param[in] filePath		if not empty, the file path to the saved image;
	///							if empty, an automatically generated file under dir()
	/// \returns whether the save was successful
	bool save(unsigned w, unsigned h, const std::string& filePath="");

	/// Save screen pixels to an image file

	/// @param[in] rect			dimensions of capture;
	///							any object with width() and height() member functions
	/// @param[in] filePath		if not empty, the file path to the saved image;
	///							if empty, an automatically generated file under dir()
	/// \returns whether the save was successful
	template <class Rect>
	bool save(const Rect& rect, const std::string& filePath=""){
		return save(rect.width(), rect.height(), filePath);
	}

	/// Save screen pixels in curent viewport to an image file

	/// @param[in] filePath		if not empty, the file path to the saved image;
	///							if empty, an automatically generated file under dir()
	/// \returns whether the save was successful
	bool save(const std::string& filePath="");

private:
	std::string mExt = "tga";
	std::string mDir = "";
};

} // al::
#endif
