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
	RGBA and HSV color classes

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/system/al_Config.h"

namespace al{

struct RGB;
struct HSV;
struct Color;
struct Colori;


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

	/// @param[in] rgb			RGB value
	/// @param[in] a			alpha component
	Color(const RGB& rgb, float a=1.f)
	:	a(a)
	{	*this = rgb; }


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
	Color& set(float v, float al){ return set(v,v,v,a); }

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

	/// Clamp all components into [0,1] range
	Color& clamp(){
		for(int i=0; i<4; ++i){
			float& v = components[i];
			v<0.f ? v=0.f : (v>1.f ? v=1.f : 0);
		}
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
	Colori(const HSV& hsv, uint8_t a=255)
	:	a(a)
	{	*this = hsv; }

	/// @param[in] hsv			HSV value
	/// @param[in] a			alpha component
	Colori(const RGB& rgb, uint8_t a=255)
	:	a(a)
	{	*this = rgb; }

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

	/// @param[in] v	RGB color to convert from
	HSV(const Color& v){ *this = v; }

	/// @param[in] v	RGB color to convert from
	HSV(const Colori& v){ *this = v; }

	/// @param[in] v	RGB color to convert from
	HSV(const RGB& v){ *this = v; }

	/// @param[in] hsv	3-vector of HSV components
	template<class T>
	HSV(T * hsv): h(hsv[0]), s(hsv[1]), v(hsv[2]){}


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


	/// @param[in] r		red component
	/// @param[in] g		green component
	/// @param[in] b		blue component
	RGB(float r, float g, float b)
	:	r(r), g(g), b(b){}

	/// @param[in] gray		red/green/blue components
	RGB(float gray=1.f)
	:	r(gray), g(gray), b(gray){}

	/// @param[in] v		RGB color to convert from
	RGB(const Color& v){ *this = v; }

	/// @param[in] hsv			HSV value
	RGB(const HSV& hsv)
	{	*this = hsv; }


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


	/// Clamp all components into [0,1] range
	RGB& clamp(){
		for(int i=0; i<3; ++i){
			float& v = components[i];
			v<0.f ? v=0.f : (v>1.f ? v=1.f : 0);
		}
		return *this;
	}

	/// Returns inverted color
	RGB inverse() const { return RGB(*this).invert(); }

	/// Invert RGB components
	RGB& invert(){ return set(1.f-r, 1.f-g, 1.f-b); }

	/// Returns luminance value
	float luminance() const { return r*0.3f + g*0.59f + b*0.11f; }

	/// Returns self linearly mixed with another color (0 = none)
	RGB mix(const RGB& v, float amt=0.5f) const {
		return (v-*this)*amt + *this;
	}
};



// Implementation --------------------------------------------------------------

inline RGB operator + (float s, const RGB& c){ return  c+s; }
inline RGB operator - (float s, const RGB& c){ return -c+s; }
inline RGB operator * (float s, const RGB& c){ return  c*s; }
inline RGB operator / (float s, const RGB& c){ return RGB(s/c.r, s/c.g, s/c.b); }



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
