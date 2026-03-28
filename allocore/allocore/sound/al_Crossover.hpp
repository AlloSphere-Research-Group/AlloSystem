#ifndef INC_AL_CROSSOVER_HPP
#define INC_AL_CROSSOVER_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	A cross-over shelf filter that sums to an allpass

	Author(s):
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

	/// Set cross-over middle frequency
	void freq(T f, T fs){
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

	/// Process one sample and return hi/lo shelf
	void next(const T in, T * lo, T * hi);

	void clear() { mZ0=(T)0; mZ1=(T)0; mZ2=(T)0; }

protected:
	// coefficients and history
	T mC0, mC1, mZ0, mZ1, mZ2;

	static constexpr double twoPi = 6.283185307179586476925286766559;
};


namespace detail{
	template <typename T> constexpr T denorm_offset();
	template<> constexpr float denorm_offset<float>(){ return FLT_EPSILON*2.; }
	template<> constexpr double denorm_offset<double>(){ return DBL_EPSILON*2.; }
}

template <typename T>
inline void Crossover<T>::next(const T in, T * lo, T * hi){

	const auto v0 = in - mC0 * mZ0;
	const auto x0 = mZ0 + mC0 * v0;

	const auto v1 = mC1 * (in - mZ1);
	const auto x1 = v1 + mZ1;

	const auto v2 = mC1 * (x1 - mZ2);
	const auto x2 = v2 + mZ2;

	auto eps = detail::denorm_offset<T>();

	mZ0 = v0 + eps;
	mZ1 = v1 + x1 + eps;
	mZ2 = v2 + x2 + eps;

	*lo = x2;
	*hi = x0 - x2;
}

} // al::
#endif
