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
	RGBA, HSV, XYZ, Lab, HCLab, Luv, and HCLuv color classes

	Color conversion code for XYZ, Lab, HCLab, Luv, HCLuv was adapted from psuedo-code and formulas found at
	http://www.easyrgb.com/ and http://brucelindbloom.com/
	
	Note that converting from RGB/HSV to Lab/Luv and back will result in SLIGHTLY different values 


	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Owen Campbell, 2013, owen.campbell@gmail.com
*/

#include "allocore/system/al_Config.h"

namespace al{

  struct RGB;
  struct HSV;
  struct Color;
  struct Colori;
  struct XYZ;
  struct Lab;
  struct HCLab;
  struct Luv;
  struct HCLuv;

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
  /*
	/// @param[in] xyz			XYZ color to convert from
	Color(const XYZ& xyz)
	{	*this = xyz; }

	/// @param[in] lab			Lab color to convert from
	Color(const Lab& lab)
	{	*this = lab; }

	/// @param[in] hclab			HCLab color to convert from
	Color(const HCLab& hclab)
	{	*this = hclab; }

	/// @param[in] luv			Luv color to convert from
	Color(const Luv& luv)
	{	*this = luv; }

	/// @param[in] hcluv			HCLuv color to convert from
	Color(const HCLuv& hcluv)
	{	*this = hcluv; }

  */
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
  /*
        /// Set RGB components from XYZ
        Color& operator= (const XYZ& v);

        /// Set RGB components from Lab
        Color& operator= (const Lab& v);

        /// Set RGB components from HCLab
        Color& operator= (const HCLab& v);

        /// Set RGB components from Luv
        Color& operator= (const Luv& v);

        /// Set RGB components from HCLuv
        Color& operator= (const HCLuv& v);
  */
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

  /*
	/// @param[in] xyz			XYZ color to convert from
	Colori(const XYZ& xyz)
	{	*this = xyz; }

	/// @param[in] lab			Lab color to convert from
	Colori(const Lab& lab)
	{	*this = lab; }

	/// @param[in] hclab			HCLab color to convert from
	Colori(const HCLab& hclab)
	{	*this = hclab; }

	/// @param[in] luv			Luv color to convert from
	Colori(const Luv& luv)
	{	*this = luv; }

	/// @param[in] hcluv			HCLuv color to convert from
	Colori(const HCLuv& hcluv)
	{	*this = hcluv; }
*/

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

  /*
        /// Set RGB components from XYZ
        Color& operator= (const XYZ& v);

        /// Set RGB components from Lab
        Color& operator= (const Lab& v);

        /// Set RGB components from HCLab
        Color& operator= (const HCLab& v);

        /// Set RGB components from Luv
        Color& operator= (const Luv& v);

        /// Set RGB components from HCLuv
        Color& operator= (const HCLuv& v);
  */
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
  /*
	/// @param[in] xyz			XYZ color to convert from
	HSV(const XYZ& xyz)
	{	*this = xyz; }

	/// @param[in] lab			Lab color to convert from
	HSV(const Lab& lab)
	{	*this = lab; }

	/// @param[in] hclab			HCLab color to convert from
	HSV(const HCLab& hclab)
	{	*this = hclab; }

	/// @param[in] luv			Luv color to convert from
	HSV(const Luv& luv)
	{	*this = luv; }

	/// @param[in] hcluv			HCLuv color to convert from
	HSV(const HCLuv& hcluv)
	{	*this = hcluv; }
  */

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

	/// Set from XYZ color
	HSV& operator= (const XYZ& v);

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
  /*
	/// @param[in] xyz			XYZ value
	RGB(const XYZ& xyz)
	{	*this = xyz; }

	/// @param[in] lab			Lab value
	RGB(const Lab& lab)
	{	*this = lab; }

	/// @param[in] hclab			HCLab value
	RGB(const HCLab& hclab)
	{	*this = hclab; }

	/// @param[in] luv			Luv value
	RGB(const Luv& luv)
	{	*this = luv; }

	/// @param[in] hcluv			HCLuv value
	RGB(const HCLuv& hcluv)
	{	*this = hcluv; }
  */
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
  /*
  	/// Set RGB components from XYZ
	RGB& operator= (const XYZ& v);

  	/// Set RGB components from Lab
	RGB& operator= (const Lab& v);

	/// Set RGB components from HCLab
	RGB& operator= (const HCLab& v);

	/// Set RGB components from Luv
	RGB& operator= (const Luv& v);

	/// Set RGB components from HCLuv
	RGB& operator= (const HCLuv& v);
  */
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

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//begin xyz

struct XYZ{
  union{
    struct{
      float x;			///< red component in [0, 1]
      float y;			///< green component in [0, 1]
      float z;			///< blue component in [0, 1]
    };
    float components[3];	///< XYZ component vector
  };

  /// @param[in] x	CIE X
  /// @param[in] y	CIE Y
  /// @param[in] z	CIE Z
  XYZ(float x=0, float y=1, float z=1): x(x), y(y), z(z){}

  /// @param[in] v	RGB color to convert from
  XYZ(const Color& v){ *this = v; }

  /// @param[in] v	RGB color to convert from
  XYZ(const Colori& v){ *this = v; }

  /// @param[in] v	RGB color to convert from
  XYZ(const RGB& v){ *this = v; }

  /// @param[in] v	HSV color to convert from
  XYZ(const HSV& v){ *this = v; }

  /// @param[in] xyz	3-vector of XYZ components
  template<class T>
  XYZ(T * xyz): x(xyz[0]), y(xyz[1]), z(xyz[2]){}


  /// Set color component at index with no bounds checking
  float& operator[](int i){ return components[i]; }

  /// Get color component at index with no bounds checking
  const float& operator[](int i) const { return components[i]; }


  /// Set from RGBA color
  XYZ& operator= (const Color& v){ return *this = v.rgb(); }

  /// Set from RGBA color
  XYZ& operator= (const Colori& v){ return *this = Color(v); }

  /// Set from RGB color
  XYZ& operator= (const RGB& v){
    //using sRGB and reference white D65 
    Mat3f transformMatrix(0.4124f,  0.3576f,  0.1805f,
			  0.2126f, 0.7152f, 0.0722f,
			  0.0193f,  0.1192f,  0.9505f);
    float R = v.r, G = v.g, B = v.b;
    //convert vanilla RGB values to be linear with respect to energy
    R = (float)(R <= 0.04045)?R / 12.92:pow(((R + 0.055) / 1.055), 2.4);
    G = (float)(G <= 0.04045)?G / 12.92:pow(((G + 0.055) / 1.055), 2.4);
    B = (float)(B <= 0.04045)?B / 12.92:pow(((B + 0.055) / 1.055), 2.4);
    
    //convert rgb to CIE XYZ
    Vec3f rgb(R, G, B);
    Vec3f xyz = transformMatrix * rgb;
    x = xyz[0]; y = xyz[1]; z = xyz[2];

    //cout << "XYZ from RGB: {" << x << ", " << y << ", " << z << "}" << endl;
    return *this;
  };

  /// Set from HSV color
  XYZ& operator= (const HSV& v){
    return *this = RGB(v);
  }
};

//end xyz
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//begin lab

/// Color represented by L* (lightness), a*, b*
struct Lab{
  union{
    struct{
      float l; ///< Lightness component in [0, 100]
      float a; ///< red-green axis (red is positive, green is negative)
               ///  range in [-85.9293, 97.9631] (8-bit rgb gamut)
      float b; ///< yellow-blue axis (yellow is positive, blue is negative)
               ///  range in [-107.544, 94.2025]
      //range determined empirically using
      //16million RGB color image from http://brucelindbloom.com
      ///Note: color is neutral gray when  a and b are both 0
    };
    float components[3];	///< Lab component vector
  };


  /// @param[in] l	Lightness
  /// @param[in] a	a
  /// @param[in] b	b
  Lab(float l=1, float a=1, float b=1): l(l), a(a), b(b){}

  /// @param[in] v	RGB color to convert from
  Lab(const Color& v){ *this = v; }

  /// @param[in] v	RGB color to convert from
  Lab(const Colori& v){ *this = v; }

  /// @param[in] v	RGB color to convert from
  Lab(const RGB& v){ *this = v; }

  /// @param[in] v	HSV color to convert from
  Lab(const HSV& v){ *this = v; }

  /// @param[in] v	XYZ color to convert from
  Lab(const XYZ& v){ *this = v; }

  /// @param[in] hsv	3-vector of Lab components
  template<class T>
  Lab(T * Lab): l(Lab[0]), a(Lab[1]), b(Lab[2]){}


  /// Set color component at index with no bounds checking
  float& operator[](int i){ return components[i]; }

  /// Get color component at index with no bounds checking
  const float& operator[](int i) const { return components[i]; }


  /// Set from RGBA color
  Lab& operator= (const Color& v){ return *this = v.rgb(); }

  /// Set from RGBA color
  Lab& operator= (const Colori& v){ return *this = Color(v); }

  /// Set from XYZ color
  Lab& operator= (const XYZ& v){
    float fx, fy, fz, xr, yr, zr;
    float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
    // using reference white D65
    float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
    
    // convert XYZ to Lab
    xr = v.x / Xn; yr = v.y / Yn; zr = v.z / Zn;
    fx = (float)(xr > epsilon)?pow(xr, 1.0/3.0):((kappa * xr + 16.0) / 116.0);
    fy = (float)(yr > epsilon)?pow(yr, 1.0/3.0):((kappa * yr + 16.0) / 116.0);
    fz = (float)(zr > epsilon)?pow(zr, 1.0/3.0):((kappa * zr + 16.0) / 116.0);

    l = 116.0f * fy - 16.0f;
    a = 500.0f * (fx - fy);
    b = 200.0f * (fy - fz);
    //cout << "Lab: {" << l << ", " << a << ", " << b << "}" << endl;    
    return *this;
  }

  /// Set from RGB color
  Lab& operator= (const RGB& v){
    return *this = XYZ(v);
  }

  /// Set from HSV color
  Lab& operator= (const HSV& v){
    return *this = XYZ(v);
  }

  /// Get new Lab with value component multiplied by a scalar
  Lab  operator* (float a) const { return Lab(*this)*=a; }

  /// Multiply lightness component by a scalar
  Lab& operator*=(float a){ l*=a; return *this; }
  
  };

//end lab
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//begin hclab

/// Color represented by hue, chroma, luminance(ab)
struct HCLab{

  union{
    struct{
      //ranges normalized to 8-bit RGB gamut using
      //16million RGB color image from http://brucelindbloom.com
      float h; ///< hue component in [0, 1]
      float c; ///< chroma component
               ///  range in [0, 1]
      float l; ///< luminance(ab) component
               ///  range in [0, 1]
    };
    float components[3];	///< HCLab component vector
  };


  /// @param[in] h	hue
  /// @param[in] c	chroma
  /// @param[in] l	luminance(ab)
  HCLab(float h=1, float c=1, float l=1): h(h), c(c), l(l){}

  /// @param[in] v	RGB color to convert from
  HCLab(const Color& v){ *this = v; }

  /// @param[in] v	RGB color to convert from
  HCLab(const Colori& v){ *this = v; }

  /// @param[in] v	RGB color to convert from
  HCLab(const RGB& v){ *this = v; }

  /// @param[in] v	HSV color to convert from
  HCLab(const HSV& v){ *this = v; }

  /// @param[in] v	XYZ color to convert from
  HCLab(const XYZ& v){ *this = v; }

  /// @param[in] v	Lab color to convert from
  HCLab(const Lab& v){ *this = v; }\

  /// @param[in] hcl	3-vector of HCLab components
  template<class T>
  HCLab(T * HCLab): h(HCLab[0]), c(HCLab[1]), l(HCLab[2]){}


  /// Set color component at index with no bounds checking
  float& operator[](int i){ return components[i]; }

  /// Get color component at index with no bounds checking
  const float& operator[](int i) const { return components[i]; }


  /// Set from RGBA color
  HCLab& operator= (const Color& v){ return *this = v.rgb(); }

  /// Set from RGBA color
  HCLab& operator= (const Colori& v){ return *this = Color(v); }

  /// Set from RGB color
  HCLab& operator= (const RGB& v){
    return *this = Lab(v);
  }

  /// Set from HSV color
  HCLab& operator= (const HSV& v){
    return *this = Lab(v);
  }

  /// Set from XYZ color
  HCLab& operator= (const XYZ& v){
    return *this = Lab(v);
  }

  /// Set from Lab color
  HCLab& operator= (const Lab& v){
    static const float TAU = 2 * M_PI;
    float L = v.l, a = v.a, b = v.b;
    //calculate hue angle from 0 to 1
    h = (atan2(b, a) + M_PI) / TAU;
    //cout << "HCLab hue: " << h << endl;
    //wrap hue angle
    if(h < 0.f) h += 1.f;
    if(h >= 1.f) h -= 1.f;

    c = sqrt(a * a + b * b) / 133.419f; //range determined empirically using
                                        //16million RGB color image from http://brucelindbloom.com
    l = L / 100.0f;    
    //cout << "HCLab: {" << h << ", " << c << ", " << l << "}" << endl;    
    return *this;
  }

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




//end lab
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//begin luv


/// Color represented by L* (lightness), u*, v*
struct Luv{
  union{
    struct{
      float l; ///< Lightness component in [0, 100]
      float u; ///< red-green axis (red is positive, green is negative)
               ///  range in [-82.7886, 174.378] (8-bit rgb gamut)
      float v; ///< yellow-blue axis (yellow is positive, blue is negative)
               ///  range in [-133.556, 107.025]
      //range determined empirically using
      //16million RGB color image from http://brucelindbloom.com
      ///Note: color is neutral gray when  a and b are both 0
    };
    float components[3];	///< Luv component vector
  };


  /// @param[in] l	Lightness
  /// @param[in] u	u
  /// @param[in] v	v
  Luv(float l=1, float u=1, float v=1): l(l), u(u), v(v){}

  /// @param[in] w	RGB color to convert from
  Luv(const Color& w){ *this = w; }

  /// @param[in] w	RGB color to convert from
  Luv(const Colori& w){ *this = w; }

  /// @param[in] w	RGB color to convert from
  Luv(const RGB& w){ *this = w; }

  /// @param[in] w	HSV color to convert from
  Luv(const HSV& v){ *this = v; }

  /// @param[in] w	XYZ color to convert from
  Luv(const XYZ& v){ *this = v; }

  /// @param[in] hsv	3-vector of Luv components
  template<class T>
  Luv(T * Luv): l(Luv[0]), u(Luv[1]), v(Luv[2]){}


  /// Set color component at index with no bounds checking
  float& operator[](int i){ return components[i]; }

  /// Get color component at index with no bounds checking
  const float& operator[](int i) const { return components[i]; }


  /// Set from RGBA color
  Luv& operator= (const Color& w){ return *this = w.rgb(); }

  /// Set from RGBA color
  Luv& operator= (const Colori& w){ return *this = Color(w); }

  /// Set from XYZ color
  Luv& operator= (const XYZ& w){
    float up, vp, ur, yr, vr, x, y, z;
    float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
    // using reference white D65
    float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
    x = w.x; y = w.y; z = w.z;

    // convert XYZ to Luv
    ur = (4 * Xn) / (Xn + 15 * Yn + 3 * Zn); 
    yr = w.y / Yn; 
    vr = (9 * Yn) / (Xn + 15 * Yn + 3 * Zn);
    
    up = (4 * x) / (x + 15 * y + 3 * z);
    vp = (9 * y) / (x + 15 * y + 3 * z);

    l = (float)(yr > epsilon)?116.0f * pow(yr, 1.0/3.0) - 16.0f:kappa * yr;
    u = 13.0f * l * (up - ur);
    v = 13.0f * l * (vp - vr);
    //cout << "Luv: {" << l << ", " << u << ", " << v << "}" << endl;    
    return *this;
  }

  /// Set from RGB color
  Luv& operator= (const RGB& w){
    return *this = XYZ(w);
  }

  /// Set from HSV color
  Luv& operator= (const HSV& w){
    return *this = XYZ(w);
  }

  /// Get new Luv with value component multiplied by a scalar
  Luv  operator* (float a) const { return Luv(*this)*=a; }

  /// Multiply lightness component by a scalar
  Luv& operator*=(float a){ l*=a; return *this; }
  
  };


//end luv
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//begin hcluv

/// Color represented by hue, chroma, luminance(uv)
struct HCLuv{

  union{
    struct{
      //ranges normalized to 8-bit RGB gamut using
      //16million RGB color image from http://brucelindbloom.com
      float h; ///< hue component in [0, 1]
      float c; ///< chroma component
               ///  range in [0, 1]
      float l; ///< luminance(uv) component
               ///  range in [0, 1]
    };
    float components[3];	///< HCLuv component vector
  };


  /// @param[in] h	hue
  /// @param[in] c	chroma
  /// @param[in] l	luminance(uv)
  HCLuv(float h=1, float c=1, float l=1): h(h), c(c), l(l){}

  /// @param[in] w	RGB color to convert from
  HCLuv(const Color& w){ *this = w; }

  /// @param[in] w	RGB color to convert from
  HCLuv(const Colori& w){ *this = w; }

  /// @param[in] w	RGB color to convert from
  HCLuv(const RGB& w){ *this = w; }

  /// @param[in] w	HSV color to convert from
  HCLuv(const HSV& w){ *this = w; }

  /// @param[in] v	XYZ color to convert from
  HCLuv(const XYZ& w){ *this = w; }

  /// @param[in] w	Luv color to convert from
  HCLuv(const Luv& w){ *this = w; }

  /// @param[in] hcl	3-vector of HCLuv components
  template<class T>
  HCLuv(T * HCLuv): h(HCLuv[0]), c(HCLuv[1]), l(HCLuv[2]){}


  /// Set color component at index with no bounds checking
  float& operator[](int i){ return components[i]; }

  /// Get color component at index with no bounds checking
  const float& operator[](int i) const { return components[i]; }


  /// Set from RGBA color
  HCLuv& operator= (const Color& w){ return *this = w.rgb(); }

  /// Set from RGBA color
  HCLuv& operator= (const Colori& w){ return *this = Color(w); }

  /// Set from RGB color
  HCLuv& operator= (const RGB& w){
    return *this = Luv(w);
  }

  /// Set from HSV color
  HCLuv& operator= (const HSV& w){
    return *this = Luv(w);
  }

  /// Set from XYZ color
  HCLuv& operator= (const XYZ& w){
    return *this = Luv(w);
  }

  /// Set from Luv color
  HCLuv& operator= (const Luv& w){
    static const float TAU = 2 * M_PI;
    float L = w.l, u = w.u, v = w.v;
    //calculate hue angle from 0 to 1
    h = (atan2(v, u) + M_PI) / TAU;
    //wrap hue angle
    if(h < 0.f) h += 1.f;
    if(h >= 1.f) h -= 1.f;

    c = sqrt(u * u + v * v) / 178.387f; //range determined empirically using
                                        //16million RGB color image from http://brucelindbloom.com
    l = L / 100.0f;    
    //cout << "HCLuv: {" << h << ", " << c << ", " << l << "}" << endl;    
    return *this;
  }

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
//end hcluv
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


//move these~~~~~~~~~~~~~~~~
XYZ Lab_to_XYZ(const Lab& v){
  float l, a, b, X, Y, Z, fx, fy, fz, xr, yr, zr;
  float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
  // using reference white D65
  float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
    
  l = v.l; a = v.a; b = v.b;

  fy = (l + 16) / 116;
  fx = (a / 500) + fy;
  fz = fy - (b / 200);
    
  xr = (float)(pow(fx, 3.0) > epsilon)?pow(fx, 3.0):((116.0f * fx - 16.0f) / kappa);
  yr = (float)(l > epsilon * kappa)?pow((l + 16.0f) / 116.0f, 3.0):l / kappa;
  zr = (float)(pow(fz, 3.0) > epsilon)?pow(fz, 3.0):((116.0f * fz - 16.0f) / kappa);

  X = xr * Xn; 
  Y = yr * Yn;
  Z = zr * Zn;

  //cout << "XYZ from Lab: {" << X << ", " << Y << ", " << Z << "}" << endl;
  return XYZ(X, Y, Z);
}


RGB XYZ_to_RGB(const XYZ& v){
  //using sRGB and reference white D65 
  Mat3f transformMatrix(3.2405f, -1.5371f, -0.4985f,
			-0.9693f,  1.8760f,  0.0416f,
			0.0556f, -0.2040f,  1.0572f);
  float X = v.x, Y = v.y, Z = v.z;

  //convert XYZ to rgb (linear with respect to energy)
  Vec3f xyz(X, Y, Z);
  Vec3f rgb = transformMatrix * xyz;
  float r = rgb[0], g = rgb[1], b = rgb[2], R, G, B;

  //convert linear RGB values to vanilla RGB
  R = (float)(r <= 0.0031308)?r * 12.92:pow(r, 1.0 / 2.4) * 1.055  - 0.055;
  G = (float)(g <= 0.0031308)?g * 12.92:pow(g, 1.0 / 2.4) * 1.055  - 0.055;
  B = (float)(b <= 0.0031308)?b * 12.92:pow(b, 1.0 / 2.4) * 1.055  - 0.055;
  //clamp RGB values to [0, 1]
  if(R > 1.f) R = 1.f;
  else if(R < 0.f) R = 0.f;
  if(G > 1.f) G = 1.f;
  else if(G < 0.f) G = 0.f;
  if(B > 1.f) B = 1.f;
  else if(B < 0.f) B = 0.f;

  //cout << "RGB from XYZ: {" << R << ", " << G << ", " << B << "}" << endl;
  return RGB(R, G, B);
}

RGB Lab_to_RGB(const Lab& v){
  /*RGB rgb(XYZ_to_RGB(Lab_to_XYZ(v)));
  cout << "RGB from Lab: {" << rgb.r << ", " << rgb.g << ", " << rgb.b << "}" << endl;
  return rgb;*/
  return XYZ_to_RGB(Lab_to_XYZ(v));
}

HSV Lab_to_HSV(const Lab& v){
  /*HSV hsv(XYZ_to_RGB(Lab_to_XYZ(v)));
  cout << "HSV from Lab: {" << hsv.h << ", " << hsv.s << ", " << hsv.v << "}" << endl;
  return hsv;*/
  return HSV(XYZ_to_RGB(Lab_to_XYZ(v)));
}

Lab HCLab_to_Lab(const HCLab& v){
  float L, a, b;
  L = v.l * 100.0f;
  static const float TAU = 2 * M_PI;
  a = (v.c * 133.419f) * cos((v.h * TAU) - M_PI);
  b = (v.c * 133.419f) * sin((v.h * TAU) - M_PI);
  //cout << "Lab from HCLab: {" << L << ", " << a << ", " << b << "}" << endl;
  return Lab(L, a, b);
}

RGB HCLab_to_RGB(const HCLab& v){
  /*RGB rgb(Lab_to_RGB(HCLab_to_Lab(v)));
  cout << "RGB from HCLab: {" << rgb.r << ", " << rgb.g << ", " << rgb.b << "}" << endl;
  return rgb;*/
  return Lab_to_RGB(HCLab_to_Lab(v));
}

HSV HCLab_to_HSV(const HCLab& v){
  /*HSV hsv(Lab_to_RGB(HCLab_to_Lab(v)));
  cout << "HSV from HCLab: {" << hsv.h << ", " << hsv.s << ", " << hsv.v << "}" << endl;
  return hsv;*/
  return HSV(Lab_to_RGB(HCLab_to_Lab(v)));
}




XYZ Luv_to_XYZ(const Luv& w){
  float l, u, v, X, Y, Z, a, b, c, d, ur, vr;
  float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
  // using reference white D65
  float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
  l = w.l; u = w.u; v = w.v;

  ur = (4 * Xn) / (Xn + 15 * Yn + 3 * Zn);
  vr = (9 * Yn) / (Xn + 15 * Yn + 3 * Zn);

  c = (-1.0f / 3.0f);
  a = -c * (((52.0f * l) / (u + 13.0f * l * ur)) - 1.0f);

  Y =  (float)(l > epsilon * kappa)?pow((l + 16.0) / 116.0, 3.0):l / kappa;
  
  b = -5.0f * Y;
  d = Y * (((39.0f * l) / (v + 13.0f * l * vr)) - 5.0f);

  X = (d - b) / (a - c);
  Z = X * a + b;

  //cout << "XYZ from Luv: {" << X << ", " << Y << ", " << Z << "}" << endl;
  return XYZ(X, Y, Z);
}

RGB Luv_to_RGB(const Luv& w){
  /*RGB rgb(XYZ_to_RGB(Luv_to_XYZ(w)));
  cout << "RGB from Luv: {" << rgb.r << ", " << rgb.g << ", " << rgb.b << "}" << endl;
  return rgb;*/
  return XYZ_to_RGB(Luv_to_XYZ(w));
}

HSV Luv_to_HSV(const Luv& w){
  /*HSV hsv(XYZ_to_RGB(Luv_to_XYZ(w)));
  cout << "HSV from Luv: {" << hsv.h << ", " << hsv.s << ", " << hsv.v << "}" << endl;
  return hsv;*/
  return HSV(XYZ_to_RGB(Luv_to_XYZ(w)));
}

Luv HCLuv_to_Luv(const HCLuv& w){
  float L, u, v;
  L = w.l * 100.0f;
  static const float TAU = 2 * M_PI;
  u = (w.c * 178.387f) * cos((w.h * TAU) - M_PI);
  v = (w.c * 178.387f) * sin((w.h * TAU) - M_PI);
  //cout << "Luv from HCLuv: {" << L << ", " << u << ", " << v << "}" << endl;
  return Luv(L, u, v);
}

RGB HCLuv_to_RGB(const HCLuv& w){
  /*RGB rgb(Luv_to_RGB(HCLuv_to_Luv(w)));
  cout << "RGB from HCLuv: {" << rgb.r << ", " << rgb.g << ", " << rgb.b << "}" << endl;
  return rgb;*/
  return Luv_to_RGB(HCLuv_to_Luv(w));
}

HSV HCLuv_to_HSV(const HCLuv& w){
  /*HSV hsv(Luv_to_RGB(HCLuv_to_Luv(w)));
  cout << "HSV from HCLuv: {" << hsv.h << ", " << hsv.s << ", " << hsv.v << "}" << endl;
  return hsv;*/
  return Luv_to_RGB(HCLuv_to_Luv(w));
}
  //~~~~~~~~~~~~~~~~~~~


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
