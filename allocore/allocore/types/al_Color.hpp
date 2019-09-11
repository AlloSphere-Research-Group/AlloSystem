#ifndef INCLUDE_AL_COLOR_HPP
#define INCLUDE_AL_COLOR_HPP

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
	RGBA, HSV, CIEXYZ, Lab, HCLab, Luv, and HCLuv color classes

	Color conversion code for CIEXYZ, Lab, HCLab, Luv, HCLuv was adapted from
	psuedo-code and formulas found at http://www.easyrgb.com/ and http://brucelindbloom.com/

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Owen Campbell, 2014, owen.campbell@gmail.com
*/

#include "allocore/system/al_Config.h"

namespace al{

struct RGB;
struct HSV;
struct Color;
struct Colori;

struct CIEXYZ;
struct Lab;
struct HCLab;
struct Luv;
struct HCLuv;


namespace{
	template <class T> T clampValue(const T& v, const T& max){
		return v<T(0) ? T(0) : (v>max ? max : v);
	}
}


/// Color represented by red, green, blue, and alpha components
///
/// @ingroup allocore
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

	/// @param[in] rgba			4-vector of RGBA components
	template<class T>
	Color(const T * rgba)
	:	Color(rgba[0], rgba[1], rgba[2], rgba[3]){}

	/// @param[in] gray			red/green/blue components
	/// @param[in] a			alpha component
	Color(float gray=1.f, float a=1.f)
	:	Color(gray, gray, gray, a){}

	/// @param[in] c	RGBA color to convert from
	Color(const Colori& c){ *this = c; }

	/// @param[in] hsv			HSV value
	/// @param[in] a			alpha component
	Color(const HSV& hsv, float a=1.f)
	:	a(a)
	{	*this = hsv; }

	/// @param[in] rgb			RGB value
	/// @param[in] a			alpha component
	Color(const RGB& rgb, float a=1.f)
	:	a(a)
	{	*this = rgb; }

	/// @param[in] xyz			CIEXYZ value
	/// @param[in] a			alpha component
	Color(const CIEXYZ& xyz, float a =1.f)
	:	a(a)
	{	*this = xyz; }

	/// @param[in] lab			Lab value
	/// @param[in] a			alpha component
	Color(const Lab& lab, float a =1.f)
	:	a(a)
	{	*this = lab; }

	/// @param[in] hclab		HCLab value
	/// @param[in] a			alpha component
	Color(const HCLab& hclab, float a =1.f)
	:	a(a)
	{	*this = hclab; }

	/// @param[in] luv			Luv value
	/// @param[in] a			alpha component
	Color(const Luv& luv, float a =1.f)
	:	a(a)
	{	*this = luv; }

	/// @param[in] hcluv		HCLuv value
	/// @param[in] a			alpha component
	Color(const HCLuv& hcluv, float a =1.f)
	:	a(a)
	{	*this = hcluv; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }

	RGB& rgb(){ return *(RGB*)(components); }
	const RGB& rgb() const { return *(const RGB*)(components); }


	/// Set RGB from another color and alpha from argument
	Color& set(const Color& c, float al){ a=al; return set(c.r,c.g,c.b); }

	/// Set RGBA components
	Color& set(float re, float gr, float bl, float al){ a=al; return set(re,gr,bl); }

	/// Set RGB components
	Color& set(float re, float gr, float bl){ r=re; g=gr; b=bl; return *this; }

	/// Set from gray value
	Color& set(float v){ return set(v,v,v); }

	/// Set from gray value and alpha
	Color& set(float v, float al){ return set(v,v,v,al); }

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

	/// Set RGB components from RGB
	Color& operator= (const RGB& v);

	/// Set RGB components from CIEXYZ
	Color& operator= (const CIEXYZ& v);

	/// Set RGB components from Lab
	Color& operator= (const Lab& v);

	/// Set RGB components from HCLab
	Color& operator= (const HCLab& v);

	/// Set RGB components from Luv
	Color& operator= (const Luv& v);

	/// Set RGB components from HCLuv
	Color& operator= (const HCLuv& v);

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

	/// Clamp all components into [0,max] range
	Color& clamp(float max=1.f){
		for(auto& c : components) c = clampValue(c,max);
		return *this;
	}

	/// Returns inverted color
	Color inverse() const { return Color(*this).invert(); }

	/// Invert RGB components
	Color& invert();

	/// Returns luminance value
	float luminance() const;

	/// Returns self linearly mixed with another color (0 = none)
	Color mix(const Color& c, float amt=0.5f) const {
		return (c-*this)*amt + *this;
	}

private:
	float tof(uint8_t v){ return float(v)*(1.f/255.f); }
};



/// Color represented by red, green, blue, and alpha components packed into 32-bit integer

/// The component accessor methods operate exclusively with integer types. To
/// convert to and from floating point values in the interval [0, 1], use the
/// overloaded assignment (=) operators.
///
/// @ingroup allocore
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

	/// @param[in] rgba			4-vector of RGBA components
	template<class T>
	Colori(const T * rgba)
	:	Colori(rgba[0], rgba[1], rgba[2], rgba[3]){}

	/// @param[in] gray			red/green/blue components
	/// @param[in] a			alpha component
	Colori(uint8_t gray=255, uint8_t a=255)
	:	Colori(gray, gray, gray, a){}

	/// @param[in] c			RGBA color to convert from
	Colori(const Color& c){ *this = c; }

	/// @param[in] hsv			HSV color
	/// @param[in] a			alpha component
	Colori(const HSV& hsv, uint8_t a=255)
	:	a(a)
	{	*this = hsv; }

	/// @param[in] rgb			RGB color
	/// @param[in] a			alpha component
	Colori(const RGB& rgb, uint8_t a=255)
	:	a(a)
	{	*this = rgb; }

	/// @param[in] xyz			CIEXYZ color
	/// @param[in] a			alpha component
	Colori(const CIEXYZ& xyz, float a =1.f)
	:	a(a)
	{	*this = xyz; }

	/// @param[in] lab			Lab color
	/// @param[in] a			alpha component
	Colori(const Lab& lab, float a =1.f)
	:	a(a)
	{	*this = lab; }

	/// @param[in] hclab		HCLab value
	/// @param[in] a			alpha component
	Colori(const HCLab& hclab, float a =1.f)
	:	a(a)
	{	*this = hclab; }

	/// @param[in] luv			Luv value
	/// @param[in] a			alpha component
	Colori(const Luv& luv, float a =1.f)
	:	a(a)
	{	*this = luv; }

	/// @param[in] hcluv		HCLuv value
	/// @param[in] a			alpha component
	Colori(const HCLuv& hcluv, float a =1.f)
	:	a(a)
	{	*this = hcluv; }

	/// Set color component at index with no bounds checking
	uint8_t& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const uint8_t& operator[](int i) const { return components[i]; }

	/// Set from floating-point color
	Colori& operator= (const Color& v){
		return set(toi(v.r), toi(v.g), toi(v.b), toi(v.a)); }

	/// Set RGB components from HSV
	Colori& operator= (const HSV& v);

	/// Set RGB components from RGB
	Colori& operator= (const RGB& v);

	/// Set RGB components from CIEXYZ
	Colori& operator= (const CIEXYZ& v);

	/// Set RGB components from Lab
	Colori& operator= (const Lab& v);

	/// Set RGB components from HCLab
	Colori& operator= (const HCLab& v);

	/// Set RGB components from Luv
	Colori& operator= (const Luv& v);

	/// Set RGB components from HCLuv
	Colori& operator= (const HCLuv& v);

	/// Return true if all components are equal, false otherwise
	bool operator ==(const Colori& v) const { return v.rgba==rgba; }

	/// Return true if components are not equal, false otherwise
	bool operator !=(const Colori& v) const { return !(*this == v); }

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


	/// Returns inverted color
	Colori inverse() const { return Colori(*this).invert(); }

	/// Invert RGB components
	Colori& invert(){ return set(255-r, 255-g, 255-b); }

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


	/// @param[in] h			hue
	/// @param[in] s			saturation
	/// @param[in] v			value
	HSV(float h=0, float s=1, float v=1)
	:	h(h), s(s), v(v){}

	/// @param[in] hsv			3-vector of HSV components
	template<class T>
	HSV(const T * hsv)
	:	HSV(hsv[0], hsv[1], hsv[2]){}

	/// @param[in] v			RGB color to convert from
	HSV(const Color& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	HSV(const Colori& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	HSV(const RGB& v){ *this = v; }

	/// @param[in] xyz			CIEXYZ color to convert from
	HSV(const CIEXYZ& xyz){ *this = xyz; }

	/// @param[in] lab			Lab color to convert from
	HSV(const Lab& lab){ *this = lab; }

	/// @param[in] hclab		HCLab color to convert from
	HSV(const HCLab& hclab){ *this = hclab; }

	/// @param[in] luv			Luv color to convert from
	HSV(const Luv& luv){ *this = luv; }

	/// @param[in] hcluv		HCLuv color to convert from
	HSV(const HCLuv& hcluv){ *this = hcluv; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }


	/// Set from RGBA color
	HSV& operator= (const Color& v){ return *this = v.rgb(); }

	/// Set from RGBA color
	HSV& operator= (const Colori& v){ return *this = Color(v); }

	/// Set from RGB color
	HSV& operator= (const RGB& v);

	/// Set from CIEXYZ color
	HSV& operator= (const CIEXYZ& v);

	/// Set from Lab color
	HSV& operator= (const Lab& v);

	/// Set from HCLab color
	HSV& operator= (const HCLab& v);

	/// Set from Luv color
	HSV& operator= (const Luv& v);

	/// Set from HCLuv color
	HSV& operator= (const HCLuv& v);

	/// Get new HSV with value component multiplied by a scalar
	HSV  operator* (float a) const { return HSV(*this)*=a; }

	/// Multiply value component by a scalar
	HSV& operator*=(float a){ v*=a; return *this; }


	/// Rotate hue in interval [0, 1)
	HSV& rotateHue(float dh){ h += dh; return wrapHue(); }

	/// Wrap hue value into valid interval [0, 1)
	HSV& wrapHue(){
		if(h>1){ h -= int(h); }
		else if(h<0){ h -= int(h)-1; }
		return *this;
	}
};



/// Color represented by red, green, and blue components

/// This offers an alternative to the Color class where the alpha component
/// is not required for computation.
#ifdef RGB	/* Windows related fix */
#undef RGB
#endif
struct RGB{
	union{
		struct{
			float r;			///< Red component in [0, 1]
			float g;			///< Green component in [0, 1]
			float b;			///< Blue component in [0, 1]
		};
		float components[3];	///< RGB component vector
	};


	/// @param[in] r			red component
	/// @param[in] g			green component
	/// @param[in] b			blue component
	RGB(float r, float g, float b)
	:	r(r), g(g), b(b){}

	/// @param[in] rgb			3-vector of RGB components
	template<class T>
	RGB(const T * rgb)
	:	RGB(rgb[0], rgb[1], rgb[2]){}

	/// @param[in] gray			red/green/blue components
	RGB(float gray=1.f)
	:	RGB(gray, gray, gray){}

	/// @param[in] v			RGB color to convert from
	RGB(const Color& v){ *this = v; }

	/// @param[in] v			Colori to convert from
	RGB(const Colori& v){ *this = v; }

	/// @param[in] hsv			HSV value
	RGB(const HSV& hsv){ *this = hsv; }

	/// @param[in] xyz			CIEXYZ value
	RGB(const CIEXYZ& xyz){ *this = xyz; }

	/// @param[in] lab			Lab value
	RGB(const Lab& lab){ *this = lab; }

	/// @param[in] hclab		HCLab value
	RGB(const HCLab& hclab){ *this = hclab; }

	/// @param[in] luv			Luv value
	RGB(const Luv& luv){ *this = luv; }

	/// @param[in] hcluv		HCLuv value
	RGB(const HCLuv& hcluv){ *this = hcluv; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }


	/// Set from another RGB
	RGB& set(const RGB& v){ return set(v.r,v.g,v.b); }

	/// Set from RGB components
	RGB& set(float re, float gr, float bl){ r=re; g=gr; b=bl; return *this; }

	/// Set from gray value
	RGB& set(float v){ return set(v,v,v); }

	/// Set from an array of RGB components
	template <class T>
	RGB& set(const T* rgb){ return set(rgb[0],rgb[1],rgb[2]); }

	/// Set components from tightly packed RGB array
	template <class Array3>
	RGB& operator= (const Array3& v){ return set(v[0],v[1],v[2]); }

	/// Set from gray value
	RGB& operator= (float v){ return set(v); }
	RGB& operator= (double v){ return set(v); }

	/// Set RGB components from HSV
	RGB& operator= (const HSV& v);

	/// Set RGB components from Color
	RGB& operator= (const Color& v){ return set(v.rgb()); }

	/// Set RGB components from Colori
	RGB& operator= (const Colori& v);

	/// Set RGB components from CIEXYZ
	RGB& operator= (const CIEXYZ& v);

	/// Set RGB components from Lab
	RGB& operator= (const Lab& v);

	/// Set RGB components from HCLab
	RGB& operator= (const HCLab& v);

	/// Set RGB components from Luv
	RGB& operator= (const Luv& v);

	/// Set RGB components from HCLuv
	RGB& operator= (const HCLuv& v);

	/// Return true if all components are equal, false otherwise
	bool operator ==(const RGB& v) const { return v.r==r && v.g==g && v.b==b; }

	/// Return true if components are not equal, false otherwise
	bool operator !=(const RGB& v) const { return !(*this == v); }

	RGB& operator+= (const RGB& v){ return set(r+v.r, g+v.g, b+v.b); }
	RGB& operator-= (const RGB& v){ return set(r-v.r, g-v.g, b-v.b); }
	RGB& operator*= (const RGB& v){ return set(r*v.r, g*v.g, b*v.b); }
	RGB& operator/= (const RGB& v){ return set(r/v.r, g/v.g, b/v.b); }
	RGB& operator+= (float v){ return set(r+v, g+v, b+v); }
	RGB& operator-= (float v){ return set(r-v, g-v, b-v); }
	RGB& operator*= (float v){ return set(r*v, g*v, b*v); }
	RGB& operator/= (float v){ return set(r/v, g/v, b/v); }

	RGB operator- () const { return RGB(-r,-g,-b); }
	RGB operator+ (const RGB& v) const { return RGB(*this)+=v; }
	RGB operator- (const RGB& v) const { return RGB(*this)-=v; }
	RGB operator* (const RGB& v) const { return RGB(*this)*=v; }
	RGB operator/ (const RGB& v) const { return RGB(*this)/=v; }
	RGB operator+ (float v) const { return RGB(*this)+=v; }
	RGB operator- (float v) const { return RGB(*this)-=v; }
	RGB operator* (float v) const { return RGB(*this)*=v; }
	RGB operator/ (float v) const { return RGB(*this)/=v; }


	/// Clamp all components into [0,max] range
	RGB& clamp(float max=1.f){
		for(auto& c : components) c = clampValue(c,max);
		return *this;
	}

	/// Returns inverted color
	RGB inverse() const { return RGB(*this).invert(); }

	/// Invert RGB components
	RGB& invert(){ return set(1.f-r, 1.f-g, 1.f-b); }

	/// Returns complementary color
	RGB complementary() const { return RGB(*this).complement(); }

	/// Rotate hue halfway around color wheel
	RGB& complement();

	/// Returns luminance value (following ITU-R BT.601)
	float luminance() const { return r*0.299f + g*0.587f + b*0.114f; }

	/// Returns self linearly mixed with another color (0 = none)
	RGB mix(const RGB& v, float amt=0.5f) const {
		return (v-*this)*amt + *this;
	}

	/// Set value of color in HSV space (leaving hue and saturation unchanged)
	RGB& value(float v){
		auto mx = r>g ? (r>b?r:b) : (g>b?g:b);
		return mx > 0. ? *this *= v/mx : *this = v;
	}
};


/// Color represented in CIE 1931 XYZ color space
struct CIEXYZ{
	union{
		struct{
			float x;			///< CIE X component in [0, 1]
			float y;			///< CIE Y component in [0, 1]
			float z;			///< CIE Z component in [0, 1]
		};
		float components[3];	///< CIE XYZ component vector
	};

	/// @param[in] x			CIE X
	/// @param[in] y			CIE Y
	/// @param[in] z			CIE Z
	CIEXYZ(float x=0, float y=1, float z=1): x(x), y(y), z(z){}

	/// @param[in] xyz			3-vector of CIEXYZ components
	template<class T>
	CIEXYZ(const T * xyz): CIEXYZ(xyz[0], xyz[1], xyz[2]){}

	/// @param[in] v			RGB color to convert from
	CIEXYZ(const Color& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	CIEXYZ(const Colori& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	CIEXYZ(const RGB& v){ *this = v; }

	/// @param[in] v			HSV color to convert from
	CIEXYZ(const HSV& v){ *this = v; }

	/// @param[in] v			Lab color to convert from
	CIEXYZ(const Lab& v){ *this = v; }

	/// @param[in] v			Luv color to convert from
	CIEXYZ(const Luv& v){ *this = v; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }


	/// Set from RGBA color
	CIEXYZ& operator= (const Color& v){ return *this = v.rgb(); }

	/// Set from RGBA color
	CIEXYZ& operator= (const Colori& v){ return *this = Color(v); }

	/// Set from RGB color
	CIEXYZ& operator= (const RGB& v);

	/// Set from HSV color
	CIEXYZ& operator= (const HSV& v){ return *this = RGB(v); }

	/// Set from Lab color
	CIEXYZ& operator= (const Lab& v);

	/// Set from Luv color
	CIEXYZ& operator= (const Luv& v);
};



/// Color represented by L* (lightness), a*, b*
///
/// @ingroup allocore
struct Lab{
	union{
		struct{
			float l; ///	< Lightness component in [0, 100]
			float a; ///	< red-green axis (red is positive, green is negative)
					 ///	  range in [-85.9293, 97.9631] (8-bit rgb gamut)
			float b; ///	< yellow-blue axis (yellow is positive, blue is
					 ///	  negative) range in [-107.544, 94.2025]
					 ///	range determined empirically using
					 ///	16million RGB color image from http://brucelindbloom.com
			/// Note: color is neutral gray when  a and b are both 0
		};
		float components[3];/// < Lab component vector
	};


	/// @param[in] l			Lightness
	/// @param[in] a			a
	/// @param[in] b			b
	Lab(float l=1, float a=1, float b=1): l(l), a(a), b(b){}

	/// @param[in] lab			3-vector of Lab components
	template<class T>
	Lab(const T * lab): Lab(lab[0], lab[1], lab[2]){}

	/// @param[in] v			RGB color to convert from
	Lab(const Color& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	Lab(const Colori& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	Lab(const RGB& v){ *this = v; }

	/// @param[in] v			HSV color to convert from
	Lab(const HSV& v){ *this = v; }

	/// @param[in] v			CIEXYZ color to convert from
	Lab(const CIEXYZ& v){ *this = v; }

	/// @param[in] v			HCLab color to convert from
	Lab(const HCLab& v){ *this = v; }




	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }


	/// Set from RGBA color
	Lab& operator= (const Color& v){ return *this = v.rgb(); }

	/// Set from RGBA color
	Lab& operator= (const Colori& v){ return *this = Color(v); }

	/// Set from CIEXYZ color
	Lab& operator= (const CIEXYZ& v);

	/// Set from RGB color
	Lab& operator= (const RGB& v){ return *this = CIEXYZ(v); }

	/// Set from HSV color
	Lab& operator= (const HSV& v){ return *this = CIEXYZ(v); }

	Lab& operator=(const HCLab& v);

	/// Get new Lab with value component multiplied by a scalar
	Lab  operator* (float c) const { return Lab(*this)*=c; }

	/// Multiply lightness component by a scalar
	Lab& operator*=(float c){ l*=c; return *this; }
};



/// Color represented by hue, chroma, luminance(ab)
///
/// @ingroup allocore
struct HCLab{
	union{
		struct{
			/// ranges normalized to 8-bit RGB gamut using
			/// 16million RGB color image from http://brucelindbloom.com
			float h;	///		< hue component in [0, 1]
			float c;	///		< chroma component
						///		  range in [0, 1]
			float l;	///		< luminance(ab) component
						///		  range in [0, 1]
		};
		float components[3];/// < HCLab component vector
	};

	/// @param[in] h			hue
	/// @param[in] c			chroma
	/// @param[in] l			luminance(ab)
	HCLab(float h=1, float c=1, float l=1): h(h), c(c), l(l){}

	/// @param[in] hcl			3-vector of HCLab components
	template<class T>
	HCLab(const T * hcl): HCLab(hcl[0], hcl[1], hcl[2]){}

	/// @param[in] v			RGB color to convert from
	HCLab(const Color& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	HCLab(const Colori& v){ *this = v; }

	/// @param[in] v			RGB color to convert from
	HCLab(const RGB& v){ *this = v; }

	/// @param[in] v			HSV color to convert from
	HCLab(const HSV& v){ *this = v; }

	/// @param[in] v			CIEXYZ color to convert from
	HCLab(const CIEXYZ& v){ *this = v; }

	/// @param[in] v			Lab color to convert from
	HCLab(const Lab& v){ *this = v; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }


	/// Set from RGBA color
	HCLab& operator= (const Color& v){ return *this = v.rgb(); }

	/// Set from RGBA color
	HCLab& operator= (const Colori& v){ return *this = Color(v); }

	/// Set from RGB color
	HCLab& operator= (const RGB& v){ return *this = Lab(v); }

	/// Set from HSV color
	HCLab& operator= (const HSV& v){ return *this = Lab(v); }

	/// Set from CIEXYZ color
	HCLab& operator= (const CIEXYZ& v){ return *this = Lab(v); }

	/// Set from Lab color
	HCLab& operator= (const Lab& v);

	/// Get new HCLab with value component multiplied by a scalar
	HCLab  operator* (float a) const { return HCLab(*this)*=a; }

	/// Multiply luminance component by a scalar
	HCLab& operator*=(float a){ l*=a; return *this; }

	/// Rotate hue in interval [0, 1)
	HCLab& rotateHue(float dh){ h += dh; return wrapHue(); }

	/// Wrap hue value into valid interval [0, 1)
	HCLab& wrapHue(){
		if(h>1){ h -= int(h); }
		else if(h<0){ h -= int(h)-1; }
		return *this;
	}
};



/// Color represented by L* (lightness), u*, v*
///
/// @ingroup allocore
struct Luv{
	union{
		struct{
			float l; ///	< Lightness component in [0, 100]
			float u; ///	< red-green axis (red is positive, green is negative)
					 ///	  range in [-82.7886, 174.378] (8-bit rgb gamut)
			float v; ///	< yellow-blue axis (yellow is positive, blue is
					 ///	  negative) range in [-133.556, 107.025]
					 ///	range determined empirically using
					 ///	16million RGB color image from http://brucelindbloom.com
			/// Note: color is neutral gray when  a and b are both 0
		};
		float components[3];///	< Luv component vector
	};


	/// @param[in] l			Lightness
	/// @param[in] u			u
	/// @param[in] v			v
	Luv(float l=1, float u=1, float v=1): l(l), u(u), v(v){}

	/// @param[in] luv			3-vector of Luv components
	template<class T>
	Luv(const T * luv): Luv(luv[0], luv[1], luv[2]){}

	/// @param[in] w			RGB color to convert from
	Luv(const Color& w){ *this = w; }

	/// @param[in] w			RGB color to convert from
	Luv(const Colori& w){ *this = w; }

	/// @param[in] w			RGB color to convert from
	Luv(const RGB& w){ *this = w; }

	/// @param[in] w			HSV color to convert from
	Luv(const HSV& w){ *this = w; }

	/// @param[in] w			CIEXYZ color to convert from
	Luv(const CIEXYZ& w){ *this = w; }

	/// @param[in] w			HCLuv color to convert from
	Luv(const HCLuv& w){ *this = w; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }


	/// Set from RGBA color
	Luv& operator= (const Color& w){ return *this = w.rgb(); }

	/// Set from RGBA color
	Luv& operator= (const Colori& w){ return *this = Color(w); }

	/// Set from CIEXYZ color
	Luv& operator= (const CIEXYZ& w);

	/// Set from HCLuv color
	Luv& operator= (const HCLuv& w);

	/// Set from RGB color
	Luv& operator= (const RGB& w){ return *this = CIEXYZ(w); }

	/// Set from HSV color
	Luv& operator= (const HSV& w){ return *this = CIEXYZ(w); }

	/// Get new Luv with value component multiplied by a scalar
	Luv  operator* (float a) const { return Luv(*this)*=a; }

	/// Multiply lightness component by a scalar
	Luv& operator*=(float a){ l*=a; return *this; }
};



/// Color represented by hue, chroma, luminance(uv)
///
/// @ingroup allocore
struct HCLuv{

	union{
		struct{
			//ranges normalized to 8-bit RGB gamut using
			//16million RGB color image from http://brucelindbloom.com
			float h; ///	< hue component in [0, 1]
			float c; ///	< chroma component
					 ///	  range in [0, 1]
			float l; ///	< luminance(uv) component
					 ///	  range in [0, 1]
		};
		float components[3];/// < HCLuv component vector
	};


	/// @param[in] h			hue
	/// @param[in] c			chroma
	/// @param[in] l			luminance(uv)
	HCLuv(float h=1, float c=1, float l=1): h(h), c(c), l(l){}

	/// @param[in] hcl			3-vector of HCLuv components
	template<class T>
	HCLuv(const T * hcl): HCLuv(hcl[0], hcl[1], hcl[2]){}

	/// @param[in] w			RGB color to convert from
	HCLuv(const Color& w){ *this = w; }

	/// @param[in] w			RGB color to convert from
	HCLuv(const Colori& w){ *this = w; }

	/// @param[in] w			RGB color to convert from
	HCLuv(const RGB& w){ *this = w; }

	/// @param[in] w			HSV color to convert from
	HCLuv(const HSV& w){ *this = w; }

	/// @param[in] v			CIEXYZ color to convert from
	HCLuv(const CIEXYZ& w){ *this = w; }

	/// @param[in] w			Luv color to convert from
	HCLuv(const Luv& w){ *this = w; }


	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }

	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }

	/// Set from RGBA color
	HCLuv& operator= (const Color& w){ return *this = w.rgb(); }

	/// Set from RGBA color
	HCLuv& operator= (const Colori& w){ return *this = Color(w); }

	/// Set from RGB color
	HCLuv& operator= (const RGB& w){ return *this = Luv(w); }

	/// Set from HSV color
	HCLuv& operator= (const HSV& w){ return *this = Luv(w); }

	/// Set from CIEXYZ color
	HCLuv& operator= (const CIEXYZ& w){ return *this = Luv(w); }

	/// Set from Luv color
	HCLuv& operator= (const Luv& w);

	/// Get new HCLuv with value component multiplied by a scalar
	HCLuv  operator* (float a) const { return HCLuv(*this)*=a; }

	/// Multiply luminance component by a scalar
	HCLuv& operator*=(float a){ l*=a; return *this; }

	/// Rotate hue in interval [0, 1)
	HCLuv& rotateHue(float dh){ h += dh; return wrapHue(); }

	/// Wrap hue value into valid interval [0, 1)
	HCLuv& wrapHue(){
		if(h>1){ h -= int(h); }
		else if(h<0){ h -= int(h)-1; }
		return *this;
	}
};




// Implementation --------------------------------------------------------------

inline RGB operator + (float s, const RGB& c){ return  c+s; }
inline RGB operator - (float s, const RGB& c){ return -c+s; }
inline RGB operator * (float s, const RGB& c){ return  c*s; }
inline RGB operator / (float s, const RGB& c){ return RGB(s/c.r, s/c.g, s/c.b); }

inline RGB& RGB::operator= (const Colori& v){
	return set(float(v.r)/255.f, float(v.g)/255.f, float(v.b)/255.f);
}


inline Color operator + (float s, const Color& c){ return  c+s; }
inline Color operator - (float s, const Color& c){ return -c+s; }
inline Color operator * (float s, const Color& c){ return  c*s; }
inline Color operator / (float s, const Color& c){ return Color(s/c.r, s/c.g, s/c.b, s/c.a); }

inline Color& Color::operator= (const Colori& v){
	r=tof(v.r); g=tof(v.g); b=tof(v.b); a=tof(v.a); return *this; }

inline Color& Color::operator= (const HSV& v){ rgb()=v; return *this; }
inline Color& Color::operator= (const RGB& v){ rgb()=v; return *this; }

inline Color& Color::invert(){ rgb().invert(); return *this; }

inline float Color::luminance() const { return rgb().luminance(); }


inline Colori& Colori::operator= (const HSV& v){
	return *this = RGB(v);
}

inline Colori& Colori::operator= (const RGB& v){
	return set(toi(v.r), toi(v.g), toi(v.b), 255);
}


inline HSV operator * (float s, const HSV& c){ return  c*s; }

} // al::

#endif
