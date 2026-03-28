#ifndef INC_AL_AUDIO_TEST_KIT_HPP
#define INC_AL_AUDIO_TEST_KIT_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Unit generators to be used for testing audio. The generators are not built
	for efficiency, but rather to provide the simplest thing that works so you
	don't have to write/debug your own from scratch.

	Author(s):
	Lance Putnam, 2022
*/

#include <cmath>

namespace al{

/// Sine oscillator
struct SineOsc{
	float p = 0;
	float f = 0; // set as: Hz / SR
	void freq(float hz, float SR=1.){ f=hz/SR; }
	void phase(float v){ p=v; }
	float operator()(){
		auto s = std::sin(p * 2.*355./113.);
		p = std::fmod(p + f, 1.f);
		return s;
	}
};

/// White noise
struct Noise{
	unsigned r = 17;
	float operator()(){
		r *= 2891336453UL;
		return r/2147483648.-1.;
	}
};

/// Linearly decreasing envelope
struct LinDec{
	float dec = 0.; // set as: 1 / (sec * SR)
	float val = 1.;
	void length(float sec, float SR){ dec=1./(sec*SR); }
	void reset(){ val=1.; }
	float operator()(){
		float r = val;
		val -= dec;
		if(val < 0.f) val=0.f;
		return r;
	}
	bool done() const { return 0.f==val; }
};

/// One-pole low-pass filter
struct LowPass{
	float operator()(float v, float f){ return p = p*(1-f) + v*f; }
	float p = 0.f;
};

/// Fixed-length delay line
template <int N>
struct DelayLine{
	DelayLine(){ for(int i=0; i<N; ++i) buf[i]=0; }
	float operator()(float v){ buf[tap]=v; return buf[tap=(++tap)==N?0:tap]; }
	float buf[N];
	int tap = 0;
};

} // al::
#endif
