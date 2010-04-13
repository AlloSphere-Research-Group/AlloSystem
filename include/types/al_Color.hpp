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

namespace al{

struct Color;
struct HSV;

/// An RGBA color.
struct Color{

	union{
		struct{
			float r;	///< Red component in [0, 1]
			float g;	///< Green component in [0, 1]
			float b;	///< Blue component in [0, 1]
			float a;	///< Alpha component in [0, 1]
		};
		float components[4];
	};


	/// @param[in] r			red component
	/// @param[in] g			green component
	/// @param[in] b			blue component
	/// @param[in] a			alpha component
	Color(float r, float g, float b, float a=1.f)
	:	r(r), g(g), b(b), a(a)
	{}

	/// @param[in] gray			red/green/blue components
	/// @param[in] a			alpha component
	Color(float gray=1.f, float a=1.f)
	:	r(gray), g(gray), b(gray), a(a)
	{}

	/// @param[in] hsv			HSV value
	/// @param[in] a			alpha component
	Color(const HSV& hsv, float a=1.f)
	:	a(a)
	{	*this = hsv; }


	template <class V4>
	Color& operator=(const V4& v){ r=v[0]; g=v[1]; b=v[2]; a=v[3]; return *this; }

	/// Set RGBA components
	Color& operator()(float re, float gr, float bl, float al){
		r=re; g=gr; b=bl; a=al; return *this;
	}

	/// Set RGB components
	Color& operator()(float re, float gr, float bl){ r=re; g=gr; b=bl; return *this; }

	/// Copy another Color's RGB components scaled
	Color& operator()(const Color& c, float mulRGB=1){
		return (*this)(c.r*mulRGB, c.g*mulRGB, c.b*mulRGB, c.a);
	}

	/// Set color component at index with no bounds checking
	float& operator[](int i){ return components[i]; }
	
	/// Get color component at index with no bounds checking
	const float& operator[](int i) const { return components[i]; }

	/// Set as gray
	Color& operator()(float v){ return (*this)(v,v,v); }

	///< Set as gray with alpha
	Color& operator()(float v, float al){ return (*this)(v,v,v,al); }

	/// Set gray value
	Color& operator= (const float& v){ r=v; g=v; b=v; return *this; }

	/// Set RGB components from HSV
	Color& operator= (const HSV& v);

	/// Adds argument RGBA components
	Color operator+ (const Color& c) const { return Color(r+c.r, g+c.g, b+c.b, a+c.a); }
	
	/// Subtracts argument RGBA components
	Color operator- (const Color& c) const { return Color(r-c.r, g-c.g, b-c.b, a-c.a); }

	/// Multiply RGBA values by argument
	Color& operator*=(float v){ (*this)(r*v, g*v, b*v, a*v); return *this; }
	
	/// Multiply RGBA components
	Color operator* (float v) const { return Color(*this)*=v; }

	/// Returns nearest black or white color
	Color blackAndWhite() const { return Color(luminance()>0.5f?1.f:0.f); }

	/// Clamp all components into [0,1] range
	void clamp(){
		r<0.f ? r=0.f : (r>1.f ? r=1.f : 0);
		g<0.f ? g=0.f : (g>1.f ? g=1.f : 0);
		b<0.f ? b=0.f : (b>1.f ? b=1.f : 0);
		a<0.f ? a=0.f : (a>1.f ? a=1.f : 0);
	}

	/// Returns inverted color
	Color inverse() const { return Color(1.f-r, 1.f-g, 1.f-b, a); }
	
	/// Invert RGB components
	Color& invert(){ return (*this)(1.f-r, 1.f-g, 1.f-b); }
	
	/// Returns luminance value
	float luminance() const { return r*0.3f+g*0.59f+b*0.11f; }


//	void setHSV(float h, float s, float v){ setHSV6(h * 6.f, s, v);	}
//
//	void getHSV(float &h, float &s, float &v) const{ getHSV6(h,s,v); h *= 0.166666667f; }

//
//	
//	/// Set color from HSV values in [0, 1]
//	void setHSV(float h, float s, float v);
//
//	/// Get HSV values in [0, 1] from color
//	void getHSV(float &h, float &s, float &v) const;
//
//	/// Set color from H value in [0, 6] and SV values in [0, 1].
//	void setHSV6(float h, float s, float v);
//
//	/// Get H value in [0, 6] and SV values in [0, 1] from color.
//	void getHSV6(float &h, float &s, float &v) const;
};


struct HSV{

	float h, s, v;

	HSV(float h=0, float s=1, float v=1): h(h), s(s), v(v){}	
	HSV(const Color& c){ *this = c; }

	HSV& operator()(float hue, float sat, float val){ h=hue; s=sat; v=val; return *this; }

	HSV& operator=(const Color& c);
};


} // al::

#endif
