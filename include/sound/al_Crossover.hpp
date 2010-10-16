#ifndef INCLUDE_AL_CROSSOVER_HPP
#define INCLUDE_AL_CROSSOVER_HPP

/*
 *  A collection of functions and classes related to application mainloops
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

/*
	CrossOver: a cross-over shelf filter that sums to an allpass
*/
#include <stdio.h>
#include <float.h>

namespace al {

template<typename T=double>
class Crossover {
public:

	/// set the cross-over middle frequency
	void freq(T f);
	
	Crossover(T f=(T)600) { freq(f); clear(); }
	
	/// process one sample and return hi/lo shelf
	void next(const T in, T * lo, T * hi);
	
	void clear() { mZ0=(T)0; mZ1=(T)0; mZ2=(T)0; }
	
protected:
	// coefficients and history
	T mC0, mC1, mZ0, mZ1, mZ2;

};


template<>
void Crossover<double> :: freq(double f) {
	double rad = M_PI * 2. * f;
	double cosine = cos(rad);
	double sine = sin(rad);
	if (abs(c) > 0.0001) {
		mC0 = (sine - 1.)/cosine;
	} else {
		mC0 = cosine * 0.5;
	}
	mC1 = (1. + mC0) * 0.5;
}

template<>
inline void Crossover<double> :: next(const double in, double * lo, double * hi) {
	static const double denorm_offset = DBL_EPSILON*2.;
	
	const double v0 = in - mC0 * mZ0;
	const double x0 = mZ0 + mC0 * v0;
	
	const double v1 = mC1 * (in - mZ1);
	const double x1 = v1 + mZ1;
	
	const double v2 = mC1 * (x1 - mZ2);
	const double x2 = v2 + mZ2;
	
	mZ0 = v0 + denorm_offset;
	mZ1 = v1 + x1 + denorm_offset;
	mZ2 = v2 + x2 + denorm_offset;
	
	*lo = x2;
	*hi = x0 - x2;
}

template<>
void Crossover<float> :: freq(float f) {
	float rad = M_PI * 2.f * f;
	float cosine = cosf(rad);
	float sine = sinf(rad);
	if (fabs(c) > 0.0001f) {
		mC0 = (sine - 1.f)/cosine;
	} else {
		mC0 = cosine * 0.5f;
	}
	mC1 = (1f + mC0) * 0.5f;
}

template<>
inline void Crossover<float> :: next(const float in, float * lo, float * hi) {
	static const float denorm_offset = FLT_EPSILON*2.;
	
	const float v0 = in - mC0 * mZ0;
	const float x0 = mZ0 + mC0 * v0;
	
	const float v1 = mC1 * (in - mZ1);
	const float x1 = v1 + mZ1;
	
	const float v2 = mC1 * (x1 - mZ2);
	const float x2 = v2 + mZ2;
	
	mZ0 = v0 + denorm_offset;
	mZ1 = v1 + x1 + denorm_offset;
	mZ2 = v2 + x2 + denorm_offset;
	
	*lo = x2;
	*hi = x0 - x2;
}

} // al::
#endif
