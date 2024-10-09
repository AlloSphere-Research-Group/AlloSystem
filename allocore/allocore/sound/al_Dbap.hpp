#ifndef INCLUDE_AL_DBAP_HPP
#define INCLUDE_AL_DBAP_HPP

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

/// Distance-based amplitude panner
///
/// @ingroup allocore
class Dbap : public Spatializer{
public:

	/// @param[in] sl		A speaker layout
	/// @param[in] focus	Amplitude focus to nearby speakers
	Dbap(const SpeakerLayout &sl, float focus = 1.f);

	void compile(Listener& listener) override;
	void renderBuffer(AudioIOData& io, const Pose& reldir, const float *samples, const int& numFrames) override;
	void renderSample(AudioIOData& io, const Pose& reldir, const float& sample, const int& frameIndex) override;

	/// Set exponent determining the amplitude focus to nearby speakers.

	///focus is (0, inf) with usable range typically [0.2, 5]. Default is 1.
	///A denser speaker layout my benefit from a high focus > 1, and a sparse layout may benefit from focus < 1
	void setFocus(float focus) { mFocus = focus; }

	void print() override;

private:
	Listener * mListener;
	Vec3f mSpeakerVecs[DBAP_MAX_NUM_SPEAKERS];
	int mDeviceChannels[DBAP_MAX_NUM_SPEAKERS];
	int mNumSpeakers;
	float mFocus;
};


} // al::

#endif
