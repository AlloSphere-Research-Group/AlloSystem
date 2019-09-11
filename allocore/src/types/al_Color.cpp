#include "allocore/types/al_Color.hpp"
#include <algorithm> // min,max
#include <cmath>

namespace al{

constexpr auto twoPi = 6.28318530717958647692;

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


RGB& RGB::complement(){
	auto mn = std::min({r, g, b});
	auto mx = std::max({r, g, b});
	auto df = mx-mn;
	*this = (df+2.*mn)-*this; // (df-(*this-mn))+mn
	return *this;
}

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

	//cout << "RGB from CIEXYZ: {" << r << ", " << g << ", " << b << "}" << endl;
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

	//cout << "CIEXYZ from RGB: {" << x << ", " << y << ", " << z << "}" << endl;
	return *this;
};


CIEXYZ& CIEXYZ::operator=(const Lab& v){
	float l, a, b, fx, fy, fz, xr, yr, zr;
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

	x = xr * Xn;
	y = yr * Yn;
	z = zr * Zn;

	//cout << "CIEXYZ from Lab: {" << x << ", " << y << ", " << z << "}" << endl;
	return *this;
}

CIEXYZ& CIEXYZ::operator=(const Luv& w){
	float l, u, v, a, b, c, d, ur, vr;
	float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
	// using reference white D65
	float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
	l = w.l; u = w.u; v = w.v;

	ur = (4 * Xn) / (Xn + 15 * Yn + 3 * Zn);
	vr = (9 * Yn) / (Xn + 15 * Yn + 3 * Zn);

	c = (-1.0f / 3.0f);
	a = -c * (((52.0f * l) / (u + 13.0f * l * ur)) - 1.0f);

	y =  (float)(l > epsilon * kappa)?pow((l + 16.0) / 116.0, 3.0):l / kappa;

	b = -5.0f * y;
	d = y * (((39.0f * l) / (v + 13.0f * l * vr)) - 5.0f);

	x = (d - b) / (a - c);
	z = x * a + b;

	//cout << "CIEXYZ from Luv: {" << x << ", " << y << ", " << z << "}" << endl;
	return *this;
}


Lab& Lab::operator= (const CIEXYZ& v){
	float fx, fy, fz, xr, yr, zr;
	float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
	// using reference white D65
	float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;

	// convert CIEXYZ to Lab
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

Lab& Lab::operator=(const HCLab& v){
	l = v.l * 100.0f;
	a = (v.c * 133.419f) * cos(v.h * twoPi);
	b = (v.c * 133.419f) * sin(v.h * twoPi);
	//cout << "Lab from HCLab: {" << l << ", " << a << ", " << b << "}" << endl;
	return *this;
}


HCLab& HCLab::operator= (const Lab& v){
	float L = v.l, a = v.a, b = v.b;
	//calculate hue angle from 0 to 1
	h = atan2(b, a) * (1.f / twoPi);	// hue in [-0.5, 0.5]
	if(h < 0.f) h += 1.f;					// wrap hue angle into [0, 1]

	c = sqrt(a * a + b * b) / 133.419f; //range determined empirically using
										//16million RGB color image from http://brucelindbloom.com
	l = L / 100.0f;
	//cout << "HCLab: {" << h << ", " << c << ", " << l << "}" << endl;
	return *this;
}


Luv& Luv::operator= (const CIEXYZ& w){
	float up, vp, ur, yr, vr, x, y, z;
	float epsilon = (216.0f / 24389.0f), kappa  = (24389.0f / 27.0f);
	// using reference white D65
	float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
	x = w.x; y = w.y; z = w.z;

	// convert CIEXYZ to Luv
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

Luv& Luv::operator=(const HCLuv& w){
	l = w.l * 100.0f;
	u = (w.c * 178.387f) * cos(w.h * twoPi);
	v = (w.c * 178.387f) * sin(w.h * twoPi);
	//cout << "Luv from HCLuv: {" << l << ", " << u << ", " << v << "}" << endl;
	return *this;
}


HCLuv& HCLuv::operator= (const Luv& w){
	float L = w.l, u = w.u, v = w.v;
	//calculate hue angle from 0 to 1
	h = atan2(v, u) * (1.f / twoPi);	// hue in [-0.5, 0.5]
	if(h < 0.f) h += 1.f;					// wrap hue angle into [0, 1]

	c = sqrt(u * u + v * v) / 178.387f; //range determined empirically using
										//16million RGB color image from http://brucelindbloom.com
	l = L / 100.0f;
	//cout << "HCLuv: {" << h << ", " << c << ", " << l << "}" << endl;
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
