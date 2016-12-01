#include "allocore/types/al_Color.hpp"
#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Mat.hpp"
#include <cmath>

namespace al{

RGB& RGB::operator= (const HSV& hsv){

	float h=hsv.h*6.f, s=hsv.s, v=hsv.v;

	if(s == 0.f){	// achromatic (gray)
		return set(v);
	}
										// sector 0 to 5
	unsigned int i = (unsigned int)(h);	// integer part of h
	float f = h - float(i);				// fractional part of h
	float p = v * (1.f - s);

	// depends on hue section being even or odd
	float q = v * (1.f - s*( (i & 1U) ? f : (1.f - f) ));

	switch( i ) {
		case 0: r=v; g=q; b=p; break;
		case 1:	r=q; g=v; b=p; break;
		case 2:	r=p; g=v; b=q; break;
		case 3:	r=p; g=q; b=v; break;
		case 4: r=q; g=p; b=v; break;
		default:r=v; g=p; b=q; break;
	}

	return *this;
}


HSV& HSV::operator= (const RGB& c){

	float r=c.r, g=c.g, b=c.b;

	float min = r<g ? (r<b ? r:b) : (g<b ? g:b);
	float max = r>g ? (r>b ? r:b) : (g>b ? g:b);

	v = max;							// set value
	float rng = max - min;			// range of RGB components

	if(rng != 0.f && max != 0.f){		// chromatic data...
		s = rng / max;					// set saturation

		float hl;
		if	 (r == max)	hl =	   (g - b)/rng;		// between yellow & magenta
		else if(g == max)	hl = 2.f + (b - r)/rng;	// between cyan & yellow
		else				hl = 4.f + (r - g)/rng;	// between magenta & cyan

		if(hl < 0.f) hl += 6.f;

		static const float _1_6 = 1.f/6.f;
		h = hl * _1_6;
	}
	else{ h=s=0.f; }					// this is a gray, no chroma...

	return *this;
}


//RGB operators
RGB& RGB::operator= (const CIEXYZ& v){
	//using sRGB and reference white D65
	static const Mat3f transformMatrix( 3.2405f, -1.5371f, -0.4985f,
									   -0.9693f,  1.8760f,  0.0416f,
									   0.0556f, -0.2040f,  1.0572f);
	float X = v.x, Y = v.y, Z = v.z;

	//convert CIEXYZ to rgb (linear with respect to energy)
	Vec3f xyz(X, Y, Z);
	Vec3f rgb = transformMatrix * xyz;
	float rl = rgb[0], gl = rgb[1], bl = rgb[2];

	//convert linear RGB values to vanilla RGB
	r = (float)(rl <= 0.0031308)?rl * 12.92:pow(rl, 1.0 / 2.4) * 1.055  - 0.055;
	g = (float)(gl <= 0.0031308)?gl * 12.92:pow(gl, 1.0 / 2.4) * 1.055  - 0.055;
	b = (float)(bl <= 0.0031308)?bl * 12.92:pow(bl, 1.0 / 2.4) * 1.055  - 0.055;
	//clamp RGB values to [0, 1]
	if(r > 1.f) r = 1.f;
	else if(r < 0.f) r = 0.f;
	if(g > 1.f) g = 1.f;
	else if(g < 0.f) g = 0.f;
	if(b > 1.f) b = 1.f;
	else if(b < 0.f) b = 0.f;

	//cout << "RGB from CIEXYZ: {" << r << ", " << g << ", " << b << "}" << endl;
	return *this;
}

//CIEXYZ operators
CIEXYZ& CIEXYZ::operator= (const RGB& v){
	//using sRGB and reference white D65
	static const Mat3f transformMatrix(0.4124f,  0.3576f,  0.1805f,
									   0.2126f, 0.7152f, 0.0722f,
									   0.0193f,  0.1192f,  0.9505f);
	float R = v.r, G = v.g, B = v.b;
	//convert vanilla RGB values to be linear with respect to energy
	R = (float)(R <= 0.04045)?R / 12.92:pow(((R + 0.055) / 1.055), 2.4);
	G = (float)(G <= 0.04045)?G / 12.92:pow(((G + 0.055) / 1.055), 2.4);
	B = (float)(B <= 0.04045)?B / 12.92:pow(((B + 0.055) / 1.055), 2.4);

	//convert rgb to CIEXYZ
	Vec3f rgb(R, G, B);
	Vec3f xyz = transformMatrix * rgb;
	x = xyz[0]; y = xyz[1]; z = xyz[2];

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

//Lab operators
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
	static const float TAU = 2 * M_PI;
	a = (v.c * 133.419f) * cos(v.h * TAU);
	b = (v.c * 133.419f) * sin(v.h * TAU);
	//cout << "Lab from HCLab: {" << l << ", " << a << ", " << b << "}" << endl;
	return *this;
}

//HCLab operators
HCLab& HCLab::operator= (const Lab& v){
	float L = v.l, a = v.a, b = v.b;
	//calculate hue angle from 0 to 1
	h = atan2(b, a) * (1.f / (2.f*M_PI));	// hue in [-0.5, 0.5]
	if(h < 0.f) h += 1.f;					// wrap hue angle into [0, 1]

	c = sqrt(a * a + b * b) / 133.419f; //range determined empirically using
										//16million RGB color image from http://brucelindbloom.com
	l = L / 100.0f;
	//cout << "HCLab: {" << h << ", " << c << ", " << l << "}" << endl;
	return *this;
}

//Luv operators
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
	static const float TAU = 2 * M_PI;
	u = (w.c * 178.387f) * cos(w.h * TAU);
	v = (w.c * 178.387f) * sin(w.h * TAU);
	//cout << "Luv from HCLuv: {" << l << ", " << u << ", " << v << "}" << endl;
	return *this;
}

//HCLuv operators
HCLuv& HCLuv::operator= (const Luv& w){
	float L = w.l, u = w.u, v = w.v;
	//calculate hue angle from 0 to 1
	h = atan2(v, u) * (1.f / (2.f*M_PI));	// hue in [-0.5, 0.5]
	if(h < 0.f) h += 1.f;					// wrap hue angle into [0, 1]

	c = sqrt(u * u + v * v) / 178.387f; //range determined empirically using
										//16million RGB color image from http://brucelindbloom.com
	l = L / 100.0f;
	//cout << "HCLuv: {" << h << ", " << c << ", " << l << "}" << endl;
	return *this;
}




//RGB operators
RGB& RGB::operator=(const Lab& v){
	return *this = CIEXYZ(v);
}

RGB& RGB::operator=(const HCLab& v){
	return *this = Lab(v);
}

RGB& RGB::operator=(const Luv& v){
	return *this = CIEXYZ(v);
}

RGB& RGB::operator=(const HCLuv& v){
	return *this = Luv(v);
}

//HSV operators
HSV& HSV::operator=(const CIEXYZ& v){
	return *this = RGB(v);
}

HSV& HSV::operator=(const Lab& v){
	return *this = CIEXYZ(v);
}

HSV& HSV::operator=(const HCLab& v){
	return *this = Lab(v);
}

HSV& HSV::operator=(const Luv& v){
	return *this = CIEXYZ(v);
}

HSV& HSV::operator=(const HCLuv& v){
	return *this = Luv(v);
}


//Color operators
Color& Color::operator=(const CIEXYZ& v){
	return *this = RGB(v);
}

Color& Color::operator=(const Lab& v){
	return *this = RGB(v);
}

Color& Color::operator=(const HCLab& v){
	return *this = RGB(v);
}

Color& Color::operator=(const Luv& v){
	return *this = RGB(v);
}

Color& Color::operator=(const HCLuv& v){
	return *this = RGB(v);
}

//Colori operators
Colori& Colori::operator=(const CIEXYZ& v){
	return *this = RGB(v);
}

Colori& Colori::operator=(const Lab& v){
	return *this = RGB(v);
}

Colori& Colori::operator=(const HCLab& v){
	return *this = RGB(v);
}

Colori& Colori::operator=(const Luv& v){
	return *this = RGB(v);
}

Colori& Colori::operator=(const HCLuv& v){
	return *this = RGB(v);
}


} // al::
