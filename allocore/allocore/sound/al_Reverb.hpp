#ifndef INCLUDE_AL_REVERB_HPP
#define INCLUDE_AL_REVERB_HPP

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
	Mono-to-stereo reverberator

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <cmath>
#include <stdlib.h>
#include <string.h>

namespace al{


/// Delay-line whose maximum size is fixed

/// The advantage of using a static versus dynamic array is that its elements
/// can be laid out in a predictable location in memory. This can improve
/// access speeds if many delay-lines are used within another object, like a
/// reverb.
///
/// @ingroup allocore
template <int N, class T>
class StaticDelayLine {
public:

	StaticDelayLine(): mPos(0){ zero(); }


	/// Get size of delay-line
	static int size(){ return N; }

	/// Get element at back
	const T& back() const { return mBuf[indexBack()]; }

	/// Get index of back element
	int indexBack() const {
		int i = pos()+1;
		return (i < size()) ? i : 0;
	}

	/// Get absolute index of write tap
	int pos() const { return mPos; }


	/// Read value at delay i
	const T& read(int i) const {
		int ind = pos()-i;
		if(ind < 0) ind += size();
		//else if(ind >= size()) ind -= size();
		return mBuf[ind];
	}

	/// Write value to delay
	void write(const T& v){
		mBuf[pos()] = v;
		++mPos; if(mPos >= size()) mPos=0;
	}

	/// Write new value and return oldest value
	T operator()(const T& v){
		T r = mBuf[pos()];
		write(v);
		return r;
	}

	/// Comb filter input using a delay time equal to the maximum size of the delay-line
	T comb(const T& v, const T& ffd, const T& fbk){
		T d = mBuf[pos()];
		T r = v + d*fbk;
		write(r);
		return d + r*ffd;
	}

	/// Allpass filter input using a delay time equal to the maximum size of the delay-line
	T allpass(const T& v, const T& ffd){
		return comb(v, ffd,-ffd);
	}

	/// Zeroes all elements (byte-wise)
	void zero(){ ::memset(&mBuf, 0, sizeof(mBuf)); }

protected:
	int mPos;
	T mBuf[N];
};



/// Plate reverberator

/// Design from:
/// Dattorro, J. (1997). Effect design: Part 1: Reverberator and other filters.
/// Journal of the Audio Engineering Society, 45(9):660-684.
/// https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf
///
/// @ingroup allocore
template <class T = float>
class Reverb{
public:

	Reverb(){
		bandwidth(0.9995);
		decay(0.85);
		damping(0.4);
		diffusion(0.76, 0.666, 0.707, 0.571);
	}


	/// Set input signal bandwidth, in [0,1]

	/// This sets the cutoff frequency of a one-pole low-pass filter on the
	/// input signal.
	Reverb& bandwidth(T v){ mOPIn.damping(T(1)-v); return *this; }

	/// Set high-frequency damping amount, in [0, 1]

	/// Higher amounts will dampen the diffusive sound more quickly.
	/// Note: values in [-1, 0] create an inverse effect that attentuates low
	/// rather than high frequencies.
	Reverb& damping(T v){ mOP1.damping(v); mOP2.damping(v); return *this; }

	/// Set decay factor, in [0, 1)
	Reverb& decay(T v){ mDecay=v; return *this; }

	/// Set diffusion amounts, in [0, 1)

	/// Values near 0.7 are recommended. Moving further away from 0.7 will lead
	/// to more distinct echoes.
	Reverb& diffusion(T in1, T in2, T decay1, T decay2){
		mDfIn1=in1;	mDfIn2=in2; mDfDcy1=decay1; mDfDcy2=decay2;
		return *this;
	}

	/// Set input diffusion 1 amount, [0,1)
	Reverb& diffusionIn1(T v){ mDfIn1=v; return *this; }

	/// Set input diffusion 2 amount, [0,1)
	Reverb& diffusionIn2(T v){ mDfIn2=v; return *this; }

	/// Set tank decay diffusion 1 amount, [0,1)
	Reverb& diffusionDecay1(T v){ mDfDcy1=v; return *this; }

	/// Set tank decay diffusion 2 amount, [0,1)
	Reverb& diffusionDecay2(T v){ mDfDcy2=v; return *this; }


	/// Compute wet stereo output from dry mono input

	/// @param[ in] in		dry input sample
	/// @param[out] out1	wet output sample 1
	/// @param[out] out2	wet output sample 2
	/// @param[ in] gain	gain of output
	void operator()(T in, T& out1, T& out2, T gain = T(0.6)){
		T v = mPreDelay(in * T(0.5));
		v = mOPIn(v);
		v = mAPIn1.allpass(v, mDfIn1);
		v = mAPIn2.allpass(v, mDfIn1);
		v = mAPIn3.allpass(v, mDfIn2);
		v = mAPIn4.allpass(v, mDfIn2);

		T a = v + mDly22.back() * mDecay;
		T b = v + mDly12.back() * mDecay;

		a = mAPDecay11.allpass(a,-mDfDcy1);
		a = mDly11(a);
		a = mOP1(a) * mDecay;
		a = mAPDecay12.allpass(a, mDfDcy2);
		mDly12.write(a);

		b = mAPDecay21.allpass(b,-mDfDcy1);
		b = mDly21(b);
		b = mOP2(b) * mDecay;
		b = mAPDecay22.allpass(b, mDfDcy2);
		mDly22.write(b);

		out1 = (  mDly21.read(266)
				+ mDly21.read(2974)
				- mAPDecay22.read(1913)
				+ mDly22.read(1996)
				- mDly11.read(1990)
				- mAPDecay12.read(187)
				- mDly12.read(1066)) * gain;

		out2 = (  mDly11.read(353)
				+ mDly11.read(3627)
				- mAPDecay12.read(1228)
				+ mDly12.read(2673)
				- mDly21.read(2111)
				- mAPDecay22.read(335)
				- mDly22.read(121)) * gain;
	}

	/// Compute wet/dry mix stereo output from dry mono input

	/// @param[in,out] inout1		the input sample and wet/dry output 1
	/// @param[   out]   out2		wet/dry output 2
	/// @param[in    ] wetAmt		wet mix amount
	/// \returns dry input sample
	T mix(T& inout1, T& out2, T wetAmt){
		T s = inout1;
		(*this)(s, inout1, out2, wetAmt*T(0.6));
		inout1 += s;
		  out2 += s;
		return s;
	}

protected:

	class OnePole{
	public:
		OnePole(): mO1(0), mA0(1), mB1(0){}
		void damping(T v){ coef(v); }
		void coef(T v){ mA0=T(1)-std::abs(v); mB1=v; }
		T operator()(T i0){ return mO1 = i0*mA0 + mO1*mB1; }
	protected:
		T mO1, mA0, mB1;
	};

	T mDfIn1, mDfIn2, mDfDcy1, mDfDcy2, mDecay;

	StaticDelayLine<  10,T> mPreDelay;
	OnePole mOPIn;
	StaticDelayLine< 142,T> mAPIn1;
	StaticDelayLine< 107,T> mAPIn2;
	StaticDelayLine< 379,T> mAPIn3;
	StaticDelayLine< 277,T> mAPIn4;
	StaticDelayLine< 672,T> mAPDecay11;
	StaticDelayLine<1800,T> mAPDecay12;
	StaticDelayLine<4453,T> mDly11;
	StaticDelayLine<3720,T> mDly12;
	OnePole mOP1;
	StaticDelayLine< 908,T> mAPDecay21;
	StaticDelayLine<2656,T> mAPDecay22;
	StaticDelayLine<4217,T> mDly21;
	StaticDelayLine<3163,T> mDly22;
	OnePole mOP2;
};

} // al::
#endif
