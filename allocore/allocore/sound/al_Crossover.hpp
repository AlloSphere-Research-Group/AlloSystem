#ifndef INCLUDE_AL_CROSSOVER_HPP
#define INCLUDE_AL_CROSSOVER_HPP

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
	CrossOver: a cross-over shelf filter that sums to an allpass
	(Useful for mixing different Ambisonic encoding flavors)

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include <cfloat> // FLT_EPSILON, DBL_EPSILON
#include <cmath>

namespace al {

///
///
///
/// @ingroup allocore
template<typename T=double>
class Crossover {
public:

	/// set the cross-over middle frequency
	void freq(T f, T fs);

	Crossover(T f=(T)600, T fs=(T)44100.) { freq(f, fs); clear(); }

	/// process one sample and return hi/lo shelf
	void next(const T in, T * lo, T * hi);

	void clear() { mZ0=(T)0; mZ1=(T)0; mZ2=(T)0; }

protected:
	// coefficients and history
	T mC0, mC1, mZ0, mZ1, mZ2;

	static constexpr double twoPi = 6.283185307179586476925286766559;
};


template<>
void Crossover<double> :: freq(double f, double fs) {
	double rad = twoPi * f / fs;
	double cosine = std::cos(rad);
	double sine = std::sin(rad);
	if (std::abs(cosine) > 0.0001) {
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
void Crossover<float> :: freq(float f, float fs) {
	float rad = twoPi * f / fs;
	float cosine = std::cos(rad);
	float sine = std::sin(rad);
	if (std::abs(cosine) > 0.0001f) {
		mC0 = (sine - 1.f)/cosine;
	} else {
		mC0 = cosine * 0.5f;
	}
	mC1 = (1.f + mC0) * 0.5f;
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
