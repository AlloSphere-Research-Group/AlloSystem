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


/// Crossover filter

/// This filter simultaneously computes low- and high-pass outputs. The sum of
/// the outputs is an all-pass response. The filter is a 2nd-order IIR with
/// 12 dB/octave cutoff slope.
///
/// @ingroup allocore
template<typename T=double>
class Crossover {
public:
	
	Crossover(T f=(T)600, T fs=(T)44100.) { freq(f, fs); clear(); }

	/// set the cross-over middle frequency
	void freq(T f, T fs);

	/// process one sample and return hi/lo shelf
	void next(const T in, T * lo, T * hi);

	void clear() { mZ0=(T)0; mZ1=(T)0; mZ2=(T)0; }

protected:
	// coefficients and history
	T mC0, mC1, mZ0, mZ1, mZ2;

	static constexpr double twoPi = 6.283185307179586476925286766559;
};


template <typename T>
void Crossover<T> :: freq(T f, T fs) {
	auto rad = twoPi * f / fs;
	auto cosine = std::cos(rad);
	auto sine = std::sin(rad);
	if (std::abs(cosine) > (T)0.0001) {
		mC0 = (sine - (T)1.)/cosine;
	} else {
		mC0 = cosine * (T)0.5;
	}
	mC1 = ((T)1. + mC0) * (T)0.5;
}

namespace{
	template <typename T> constexpr T denorm_offset();
	template<> constexpr float denorm_offset<float>(){ return FLT_EPSILON*2.; }
	template<> constexpr double denorm_offset<double>(){ return DBL_EPSILON*2.; }
}

template <typename T>
inline void Crossover<T> :: next(const T in, T * lo, T * hi) {

	const auto v0 = in - mC0 * mZ0;
	const auto x0 = mZ0 + mC0 * v0;

	const auto v1 = mC1 * (in - mZ1);
	const auto x1 = v1 + mZ1;

	const auto v2 = mC1 * (x1 - mZ2);
	const auto x2 = v2 + mZ2;

	mZ0 = v0 + denorm_offset<T>();
	mZ1 = v1 + x1 + denorm_offset<T>();
	mZ2 = v2 + x2 + denorm_offset<T>();

	*lo = x2;
	*hi = x0 - x2;
}

} // al::
#endif
