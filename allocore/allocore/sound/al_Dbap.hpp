#ifndef INCLUDE_AL_PANNING_DBAP
#define INCLUDE_AL_PANNING_DBAP

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
	Distance-based amplitude panner (DBAP)

	File author(s):
	Ryan McGee, 2012, ryanmichaelmcgee@gmail.com
*/

#include "allocore/sound/al_AudioScene.hpp"

namespace al{

#define DBAP_MAX_NUM_SPEAKERS 192
#define DBAP_MAX_DIST 100

/// Distance-based amplitude panner
class Dbap : public Spatializer{
public:

	/// @param[in] sl		A speaker layout
	/// @param[in] spread	Amplitude spread to nearby speakers
	Dbap(const SpeakerLayout &sl, float spread = 5.f);

	void compile(Listener& listener);

	///Per Sample Processing
	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample);

	/// Per Buffer Processing
	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples);

	/// Spread is an exponent determining the amplitude spread to nearby speakers.

	/// Values below 1.0 will widen the sound field to more speakers.
	/// Values greater than 1 will focus the sound field to fewer speakers.
	void setSpread(float spread) { mSpread = spread; }

	void print();

private:
	Listener * mListener;
	Vec3f mSpeakerVecs[DBAP_MAX_NUM_SPEAKERS];
	int mDeviceChannels[DBAP_MAX_NUM_SPEAKERS];
	int mNumSpeakers;
    float mSpread;
};



inline void Dbap::perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample)
{
	for (unsigned i = 0; i < mNumSpeakers; ++i)
	{
		Vec3d vec = relpos.normalized();
		vec -= mSpeakerVecs[i];
		float dist = vec.mag() / 2.f; // [0, 1]
		dist = powf(dist, mSpread);
		float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);

		io.out(mDeviceChannels[i], frameIndex) += gain*sample;
	}
}



} // al::

#endif
