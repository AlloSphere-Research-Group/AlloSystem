#ifndef INCLUDE_AL_COLOR_HPP
#define INCLUDE_AL_COLOR_HPP

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

#include "allocore/system/pstdint.h"

namespace al{

struct Color;
struct Colori;
struct HSV;


/// Color represented by red, green, blue, and alpha components
struct Color{

	union{
		struct{
			float r;			///< Red component in [0, 1]
			float g;			///< Green component in [0, 1]
			float b;			///< Blue component in [0, 1]
			float a;			///< Alpha component in [0, 1]
		};
		float components[4];	///< RGBA component vector
	};


	/// @param[in] r			red component
	/// @param[in] g			green component
	/// @param[in] b			blue component
	/// @param[in] a			alpha component
	Color(float r, float g, float b, float a=1.f)
	:	r(r), g(g), b(b), a(a){}

	/// @param[in] gray			red/green/blue components
	/// @param[in] a			alpha component
	Color(float gray=1.f, float a=1.f)
	:	r(gray), g(gray), b(gray), a(a){}

	/// @param[in] c	RGBA color to convert from
	Color(const Colori& c){ *this = c; }

	/// @param[in] hsv			HSV value
	/// @param[in] a			alpha component
	Color(const HSV& hsv, float a=1.f)
	:	a(a)
	{	*this = hsv; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }

	/// Set RGB from another color and alpha from argument
	Color& set(const Color& c, float al){ a=al; return set(c.r,c.g,c.b); }

	/// Set RGBA components
	Color& set(float re, float gr, float bl, float al){ a=al; return set(re,gr,bl); }

	/// Set RGB components
	Color& set(float re, float gr, float bl){ r=re; g=gr; b=bl; return *this; }

	/// Set from gray value
	Color& set(float v){ return set(v,v,v); }

	/// Set from gray value and alpha
	Color& set(float v, float al){ return set(v,al); }

	/// Set from an array of RGBA components
	template <class T>
	Color& set(const T* rgba){ return set(rgba[0],rgba[1],rgba[2],rgba[3]); }

	/// Set components from tightly packed RGBA array
	template <class Array4>
	Color& operator= (const Array4& v){ return set(v[0], v[1], v[2], v[3]); }

	/// Set from gray value
	Color& operator= (float v){ return set(v); }
	Color& operator= (double v){ return set(v); }

	/// Set components from integer color
	Color& operator= (const Colori& v);

	/// Set RGB components from HSV
	Color& operator= (const HSV& v);

	/// Return true if all components are equal, false otherwise
	bool operator ==(const Color& v) const { return v.r==r && v.g==g && v.b==b && v.a==a; }

	/// Return true if components are not equal, false otherwise
	bool operator !=(const Color& v) const { return !(*this == v); }

	Color& operator+= (const Color& v){ return set(r+v.r, g+v.g, b+v.b, a+v.a); }
	Color& operator-= (const Color& v){ return set(r-v.r, g-v.g, b-v.b, a-v.a); }
	Color& operator*= (const Color& v){ return set(r*v.r, g*v.g, b*v.b, a*v.a); }
	Color& operator/= (const Color& v){ return set(r/v.r, g/v.g, b/v.b, a/v.a); }
	Color& operator+= (float v){ return set(r+v, g+v, b+v, a+v); }
	Color& operator-= (float v){ return set(r-v, g-v, b-v, a-v); }
	Color& operator*= (float v){ return set(r*v, g*v, b*v, a*v); }
	Color& operator/= (float v){ return set(r/v, g/v, b/v, a/v); }

	Color operator- () const { return Color(-r,-g,-b,-a); }
	Color operator+ (const Color& v) const { return Color(*this)+=v; }
	Color operator- (const Color& v) const { return Color(*this)-=v; }
	Color operator* (const Color& v) const { return Color(*this)*=v; }
	Color operator/ (const Color& v) const { return Color(*this)/=v; }
	Color operator+ (float v) const { return Color(*this)+=v; }
	Color operator- (float v) const { return Color(*this)-=v; }
	Color operator* (float v) const { return Color(*this)*=v; }
	Color operator/ (float v) const { return Color(*this)/=v; }

	/// Returns nearest black or white color
	Color blackAndWhite() const { return Color(luminance()>0.5f?1.f:0.f); }

	/// Clamp all components into [0,1] range
	Color& clamp(){
		r<0.f ? r=0.f : (r>1.f ? r=1.f : 0);
		g<0.f ? g=0.f : (g>1.f ? g=1.f : 0);
		b<0.f ? b=0.f : (b>1.f ? b=1.f : 0);
		a<0.f ? a=0.f : (a>1.f ? a=1.f : 0);
		return *this;
	}

	/// Returns inverted color
	Color inverse() const { return Color(1.f-r, 1.f-g, 1.f-b, a); }

	/// Invert RGB components
	Color& invert(){ return set(1.f-r, 1.f-g, 1.f-b); }

	/// Returns luminance value
	float luminance() const { return r*0.3f+g*0.59f+b*0.11f; }

	/// Returns self linearly mixed with another color (0 = none)
	Color mix(const Color& c, float amt=0.5f) const {
		return (c-*this)*amt + *this;
	}

private:
	float tof(uint8_t v){ return float(v)/255.f; }
};



/// Color represented by red, green, blue, and alpha components packed into 32-bit integer

/// The component accessor methods operate exclusively with integer types. To
/// convert to and from floating point values in the interval [0, 1], use the
/// overloaded assignment (=) operators.
struct Colori {
	
	union{
		struct{
			uint8_t r;			///< Red component in [0, 255]
			uint8_t g;			///< Green component in [0, 255]
			uint8_t b;			///< Blue component in [0, 255]
			uint8_t a;			///< Alpha component in [0, 255]
		};
		uint8_t components[4];	///< RGBA component vector
		uint32_t rgba;			///< RGBA components packed into 32-bit integer
	};


	/// @param[in] r			red component
	/// @param[in] g			green component
	/// @param[in] b			blue component
	/// @param[in] a			alpha component
	Colori(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255)
	:	r(r), g(g), b(b), a(a){}

	/// @param[in] gray			red/green/blue components
	/// @param[in] a			alpha component
	Colori(uint8_t gray=255, uint8_t a=255)
	:	r(gray), g(gray), b(gray), a(a){}

	/// @param[in] c			RGBA color to convert from
	Colori(const Color& c){ *this = c; }

	/// @param[in] hsv			HSV value
	/// @param[in] a			alpha component
	Colori(const HSV& hsv, float a=1.f)
	:	a(toi(a))
	{	*this = hsv; }



	/// Set color component at index with no bounds checking
	uint8_t& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const uint8_t& operator[](int i) const { return components[i]; }

	/// Set from floating-point color
	Colori& operator= (const Color& c){
		r=toi(c.r); g=toi(c.g); b=toi(c.b); a=toi(c.a); return *this; }

	/// Set RGB components from HSV
	Colori& operator= (const HSV& v){ *this = Color(v); return *this; }

	/// Set RGB components
	Colori& set(uint8_t re, uint8_t gr, uint8_t bl){
		r=re; g=gr; b=bl; return *this; }

	/// Set RGBA components
	Colori& set(uint8_t re, uint8_t gr, uint8_t bl, uint8_t al){
		a=al; return set(re,gr,bl); }

	/// Set from gray value
	Colori& set(uint8_t v){ return set(v,v,v); }

	/// Set from gray value and alpha
	Colori& set(uint8_t v, uint8_t al){ return set(v,al); }

private:
	uint8_t toi(float v){ return uint8_t(v*255.f); }
};



/// Color represented by hue, saturation, and value
struct HSV{

	union{
		struct{
			float h;			///< Hue component in [0, 1]
			float s;			///< Saturation component in [0, 1]
			float v;			///< Value component in [0, 1]
		};
		float components[3];	///< HSV component vector
	};


	/// @param[in] h	hue
	/// @param[in] s	saturation
	/// @param[in] v	value
	HSV(float h=0, float s=1, float v=1): h(h), s(s), v(v){}

	/// @param[in] c	RGB color to convert from
	HSV(const Color& c){ *this = c; }

	/// @param[in] c	RGB color to convert from
	HSV(const Colori& c){ *this = c; }

	/// @param[in] hsv		3-vector of hsv components
	template<class T>
	HSV(T * hsv): h(hsv[0]), s(hsv[1]), v(hsv[2]){}


	/// Set from RGB color
	HSV& operator= (const Color& c);

	/// Set from RGB color
	HSV& operator= (const Colori& c){ return *this = Color(c); }

	/// Rotate hue in interval [0, 1)
	HSV& rotateHue(float dh){ h += dh; return wrapHue(); }

	/// Wrap hue value into valid interval [0, 1)
	HSV& wrapHue(){
		if(h>1){ h -= int(h); }
		else if(h<0){ h -= int(h)-1; }
		return *this;
	}
};




// Implementation --------------------------------------------------------------

inline Color operator + (float s, const Color& c){ return  c+s; }
inline Color operator - (float s, const Color& c){ return -c+s; }
inline Color operator * (float s, const Color& c){ return  c*s; }
inline Color operator / (float s, const Color& c){ return Color(s/c.r, s/c.g, s/c.b, s/c.a); }

inline Color& Color::operator= (const Colori& v){
		r=tof(v.r); g=tof(v.g); b=tof(v.b); a=tof(v.a); return *this; }


} // al::

#endif
