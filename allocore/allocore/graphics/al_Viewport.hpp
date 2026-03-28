#ifndef INC_AL_VIEWPORT_HPP
#define INC_AL_VIEWPORT_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Viewport class for describing a region of the screen

	Author(s):
	Lance Putnam, 2024, putnam.lance@gmail.com
*/

namespace al {

/// A framed area on a display screen
/// @ingroup allocore
struct Viewport {
	float l, b, w, h;	///< left, bottom, width, height

	/// @param[in] w_	width
	/// @param[in] h_	height
	Viewport(float w_=800, float h_=600)
	:	Viewport(0.f,0.f,w_,h_)
	{}

	/// @param[in] l_	left edge coordinate
	/// @param[in] b_	bottom edge coordinate
	/// @param[in] w_	width
	/// @param[in] h_	height
	Viewport(float l_, float b_, float w_, float h_)
	:	l(l_), b(b_), w(w_), h(h_)
	{}


	/// Get aspect ratio (width divided by height)
	float aspect() const { return (h!=0.f && w!=0.f) ? w/h : 1.f; }

	/// Set dimensions
	Viewport& set(float l_, float b_, float w_, float h_){
		l=l_; b=b_; w=w_; h=h_;
		return *this;
	}
};

} // al::
#endif
