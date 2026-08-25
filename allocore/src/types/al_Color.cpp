#include "allocore/types/al_Color.hpp"
#include <algorithm> // min,max
#include <cmath> // sin, cos, cbrt, sqrt, pow

namespace al{

constexpr auto twoPi = 6.28318530717958647692;

template <class T>
T clampValue(T v, T max = T(1)){
	return v<T(0) ? T(0) : (v>max ? max : v);
}

void wrapHue(float& h){
	if(h>1.f){ h -= int(h); }
	else if(h<0.f){ h -= int(h)-1; }
}

// Returns vector v transformed by matrix m (via m * v)
template <class T, class Vec>
Vec transform(
	T m11, T m12, T m13,
	T m21, T m22, T m23,
	T m31, T m32, T m33,
	const Vec& v
){
	static_assert(sizeof(v)/sizeof(v[0]) >= 3, "Vector needs at least 3 components");
	return Vec(
		m11*v[0] + m12*v[1] + m13*v[2],
		m21*v[0] + m22*v[1] + m23*v[2],
		m31*v[0] + m32*v[1] + m33*v[2]
	);
}


Color& Color::clamp(float max){
	for(auto& c : components) c = clampValue(c,max);
	return *this;
}


/*static*/ uint8_t Colori::toi(float v){
	//return uint8_t(v*255.f);
	constexpr float maxVal = 255./256.;
	if(v > maxVal) return 255;
	union{ float f; uint32_t i; } u{v+1.f};
	return uint8_t((u.i & 0x007fffff) >> 15);
}

Colori& Colori::fromHex(const char * s){
	auto h2d = [](char h) -> unsigned char {
		if('0'<=h && h<='9') return h-'0';
		if('a'<=h && h<='f') return h-'a'+10;
		if('A'<=h && h<='F') return h-'A'+10;
		return 0;
	};
	auto hh2d = [&](const char * hh){
		return (h2d(hh[0])<<4) + h2d(hh[1]);
	};

	int len=0;
	for(; len<8; ++len){
		if(s[len] == '\0') break;
	}

	// valid strings: rgb, rgba, rrggbb, rrggbbaa
	switch(len){
	case 3: return set(h2d(s[0])*17, h2d(s[1])*17, h2d(s[2])*17, 255);
	case 4: return set(h2d(s[0])*17, h2d(s[1])*17, h2d(s[2])*17, h2d(s[3])*17);
	case 6:	return set(hh2d(s), hh2d(s+2), hh2d(s+4), 255);
	case 8: return set(hh2d(s), hh2d(s+2), hh2d(s+4), hh2d(s+6));
	default:return *this;
	}
}

Colori::HexString Colori::toHex(unsigned len, char pre) const {
	const char * d2h = "0123456789abcdef";
	auto d2hh = [&](unsigned char d, char * hh){
		hh[0] = d2h[(d>>4)&0xf];
		hh[1] = d2h[(d   )&0xf];
	};
	HexString s;
	auto * ptr = s.data;
	if(pre) *ptr++ = pre;
	switch(len){
	default:
		len=8;
		d2hh(a, ptr+6);
	case 6:
		d2hh(r, ptr);
		d2hh(g, ptr+2);
		d2hh(b, ptr+4);
		break;
	case 4:
		ptr[3] = d2h[a>>4];
	case 3:
		ptr[0] = d2h[r>>4];
		ptr[1] = d2h[g>>4];
		ptr[2] = d2h[b>>4];
		break;
	}
	ptr[len] = '\0';
	return s;
}

Colori Colori::mix(const Colori& v, float amt) const {
	Colori res;
	uint8_t f = toi(amt);
	for(int i=0; i<size(); ++i){
		auto a = int((*this)[i]);
		auto b = int(v[i]);
		res[i] = ((b-a)*f + (a<<8))>>8;
	}
	return res;
}


HSV& HSV::wrapHue(){
	al::wrapHue(h);
	return *this;
}

HSV& HSV::clamp(){
	s = clampValue(s);
	v = clampValue(v);
	return wrapHue();
}


RGB& RGB::complement(){
	// max(c) - min(c) - (c - min(c)) + min(c)
	return *this = (max() + min()) - *this;
}

RGB& RGB::clamp(float max){
	for(auto& c : components) c = clampValue(c,max);
	return *this;
}

RGB& RGB::value(float v){
	auto mx = max();
	return mx > 0.f ? *this *= v/mx : *this = v;
}

float RGB::saturation() const {
	auto mx = max();
	return mx > 0.f ? (mx-min())/mx : 0.f;
}

RGB& RGB::saturation(float s){
	min() = 0.f;
	return *this = RGB(max()).mix(*this, s);
}

RGB& RGB::toSRGB(){
	auto g = [](float x){
		return x>0.0031308f ? 1.055f*std::pow(x,1.f/2.4f)-0.055f : x*12.92f;
	};
	for(auto& c : *this) c = g(c);
	return *this;
}

RGB& RGB::fromSRGB(){
	auto g = [](float x){
		return x>0.04045f ? std::pow((x+0.055f)/1.055f,2.4f) : x/12.92f;
	};
	for(auto& c : *this) c = g(c);
	return *this;
}


HCLab& HCLab::wrapHue(){
	al::wrapHue(h);
	return *this;
}


HCLuv& HCLuv::wrapHue(){
	al::wrapHue(h);
	return *this;
}


// From here, we define color space conversions. These are paired up for
// convenience.

RGB& RGB::operator= (const HSV& hsv){

	auto s=hsv.s, v=hsv.v;

	/* Removed extra branch as we typically will not use HSV for grayscale
	if(s == 0.f){	// achromatic (gray)
		return set(v);
	}//*/

	auto h=hsv.h*6.f;

	auto i = (unsigned int)(h);	// hue sector 0, 1, 2, 3, 4, or 5
	float f = h - float(i);		// fraction between sectors
	auto vs = v*s;
	auto p = v - vs;

	switch(i){
		default:{auto w=p+vs*f; return set(v,w,p);} // also catches hue=1
		case 1:	{auto w=v-vs*f; return set(w,v,p);}
		case 2:	{auto w=p+vs*f; return set(p,v,w);}
		case 3:	{auto w=v-vs*f; return set(p,w,v);}
		case 4: {auto w=p+vs*f; return set(w,p,v);}
		case 5: {auto w=v-vs*f; return set(v,p,w);}
	}
}

HSV& HSV::operator= (const RGB& c){

	auto r=c.r, g=c.g, b=c.b;

	auto min = std::min({r,g,b});
	auto max = std::max({r,g,b});

	v = max;							// set value
	auto rng = max - min;				// range of RGB components

	if(rng != 0.f && max != 0.f){		// chromatic data...
		s = rng / max;					// set saturation

		float hl;
		if     (r == max)	hl =	   (g - b)/rng; // between yellow & magenta
		else if(g == max)	hl = 2.f + (b - r)/rng;	// between cyan & yellow
		else				hl = 4.f + (r - g)/rng;	// between magenta & cyan

		if(hl < 0.f) hl += 6.f;

		h = hl * (1.f/6.f);
	}
	else{ h=s=0.f; }					// this is a gray, no chroma...

	return *this;
}


RGB& RGB::operator= (const CIEXYZ& v){
	//convert CIEXYZ to rgb (linear with respect to energy)
	//using sRGB and reference white D65
	*this = transform(
		 3.2405f, -1.5371f, -0.4985f,
		-0.9693f,  1.8760f,  0.0416f,
		 0.0556f, -0.2040f,  1.0572f,
		RGB(v.components)
	);

	//convert linear RGB values to vanilla RGB
	for(auto& c : components)
		c = c <= 0.0031308f ? c*12.92f : float(pow(c, 1./2.4) * 1.055  - 0.055);

	clamp(); //clamp RGB values to [0, 1]

	return *this;
}

CIEXYZ& CIEXYZ::operator= (const RGB& v){
	auto rgbLin = v;

	//convert vanilla RGB values to be linear with respect to energy
	for(auto& c : rgbLin.components)
		c = c <= 0.04045f ? c/12.92f : float(pow(((c + 0.055)/1.055), 2.4));

	//convert rgb to CIEXYZ
	//using sRGB and reference white D65
	*this = transform(
		0.4124f,  0.3576f,  0.1805f,
		0.2126f,  0.7152f,  0.0722f,
		0.0193f,  0.1192f,  0.9505f,
		CIEXYZ(rgbLin.components)
	);

	return *this;
};


const float cieEps = 216.f / 24389.f;
const float cieKap = 24389.f / 27.f;
float cube(float x){ return x*x*x; }

CIEXYZ& CIEXYZ::operator=(const Lab& v){
	float Xn = 0.95047f, Yn = 1.f, Zn = 1.08883f; // reference white D65

	float l = v.l, a = v.a, b = v.b;

	float fy = (l + 16.f) / 116.f;
	float fx = (a / 500.f) + fy;
	float fz = fy - (b / 200.f);

	float xr = cube(fx) > cieEps ? cube(fx) : (116.f * fx - 16.f) / cieKap;
	float yr = l > cieEps*cieKap ? cube((l + 16.f) / 116.f) : l / cieKap;
	float zr = cube(fz) > cieEps ? cube(fz) : (116.f * fz - 16.f) / cieKap;

	x = xr * Xn;
	y = yr * Yn;
	z = zr * Zn;

	return *this;
}

Lab& Lab::operator= (const CIEXYZ& v){
	float Xn = 0.95047f, Yn = 1.f, Zn = 1.08883f; // reference white D65

	float xr = v.x / Xn, yr = v.y / Yn, zr = v.z / Zn;
	float fx = xr > cieEps ? std::cbrt(xr) : (cieKap * xr + 16.f) / 116.f;
	float fy = yr > cieEps ? std::cbrt(yr) : (cieKap * yr + 16.f) / 116.f;
	float fz = zr > cieEps ? std::cbrt(zr) : (cieKap * zr + 16.f) / 116.f;

	l = 116.f * fy - 16.f;
	a = 500.f * (fx - fy);
	b = 200.f * (fy - fz);

	return *this;
}


CIEXYZ& CIEXYZ::operator=(const Luv& w){
	float Xn = 0.95047f, Yn = 1.f, Zn = 1.08883f; // reference white D65

	float l = w.l, u = w.u, v = w.v;

	float ur = (4.f * Xn) / (Xn + 15.f * Yn + 3.f * Zn);
	float vr = (9.f * Yn) / (Xn + 15.f * Yn + 3.f * Zn);

	const float _1_3 = 1.f/3.f;
	float a = _1_3 * (((52.f * l) / (u + 13.f * l * ur)) - 1.f);

	y = l > cieEps * cieKap ? cube((l + 16.f) / 116.f) : l / cieKap;

	float d = y * (((39.f * l) / (v + 13.f * l * vr)) - 5.f);
	float b = -5.f * y;

	x = (d - b) / (a + _1_3);
	z = x * a + b;

	return *this;
}

Luv& Luv::operator= (const CIEXYZ& w){
	float Xn = 0.95047f, Yn = 1.f, Zn = 1.08883f; // reference white D65

	float x = w.x, y = w.y, z = w.z;

	float ur = (4.f * Xn) / (Xn + 15.f * Yn + 3.f * Zn);
	float yr = y / Yn;
	float vr = (9.f * Yn) / (Xn + 15.f * Yn + 3.f * Zn);

	float up = (4.f * x) / (x + 15.f * y + 3.f * z);
	float vp = (9.f * y) / (x + 15.f * y + 3.f * z);

	l = yr > cieEps ? 116.f * std::cbrt(yr) - 16.f : cieKap * yr;
	u = 13.f * l * (up - ur);
	v = 13.f * l * (vp - vr);

	return *this;
}


void lch2LXY(float& L, float& X, float& Y, float l, float c, float h){
	L = l * 100.f;
	X = c * std::cos(h * twoPi);
	Y = c * std::sin(h * twoPi);
}

void LXY2lch(float& l, float& c, float& h, float L, float X, float Y){
	l = L * 0.01f;
	c = std::sqrt(X*X + Y*Y);			// this will be scaled according to color space
	h = std::atan2(Y,X) * (1.f/twoPi);	// hue in [-0.5, 0.5]
	if(h < 0.f) h += 1.f;				// wrap hue angle into [0, 1)
}

Lab& Lab::operator=(const HCLab& v){
	lch2LXY(l,a,b, v.l,v.c*133.419f,v.h);
	return *this;
}

HCLab& HCLab::operator= (const Lab& v){
	LXY2lch(l,c,h, v.l,v.a,v.b);
	c /= 133.419f; // scaling factor from http://brucelindbloom.com
	return *this;
}


Luv& Luv::operator=(const HCLuv& w){
	lch2LXY(l,u,v, w.l,w.c*178.387f,w.h);
	return *this;
}

HCLuv& HCLuv::operator= (const Luv& w){
	LXY2lch(l,c,h, w.l,w.u,w.v);
	c /= 178.387f; // scaling factor from http://brucelindbloom.com
	return *this;
}


RGB& RGB::operator=(const Lab& v){ return *this = CIEXYZ(v); }
RGB& RGB::operator=(const HCLab& v){ return *this = Lab(v); }
RGB& RGB::operator=(const Luv& v){ return *this = CIEXYZ(v); }
RGB& RGB::operator=(const HCLuv& v){ return *this = Luv(v); }

HSV& HSV::operator=(const CIEXYZ& v){ return *this = RGB(v); }
HSV& HSV::operator=(const Lab& v){ return *this = CIEXYZ(v); }
HSV& HSV::operator=(const HCLab& v){ return *this = Lab(v); }
HSV& HSV::operator=(const Luv& v){ return *this = CIEXYZ(v); }
HSV& HSV::operator=(const HCLuv& v){ return *this = Luv(v); }

Color& Color::operator=(const CIEXYZ& v){ return *this = RGB(v); }
Color& Color::operator=(const Lab& v){ return *this = RGB(v); }
Color& Color::operator=(const HCLab& v){ return *this = RGB(v); }
Color& Color::operator=(const Luv& v){ return *this = RGB(v); }
Color& Color::operator=(const HCLuv& v){ return *this = RGB(v); }

Colori& Colori::operator=(const CIEXYZ& v){ return *this = RGB(v); }
Colori& Colori::operator=(const Lab& v){ return *this = RGB(v); }
Colori& Colori::operator=(const HCLab& v){ return *this = RGB(v); }
Colori& Colori::operator=(const Luv& v){ return *this = RGB(v); }
Colori& Colori::operator=(const HCLuv& v){ return *this = RGB(v); }

} // al::
