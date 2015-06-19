#ifndef INCLUDE_AL_PANNING_VBAP
#define INCLUDE_AL_PANNING_VBAP

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
	Vector-based amplitude panner (VBAP)

	File author(s):
	Ryan McGee, 2012, ryanmichaelmcgee@gmail.com
*/

#include "allocore/sound/al_AudioScene.hpp"

#define MAX_NUM_VBAP_TRIPLETS 512
#define MIN_VOLUME_TO_LENGTH_RATIO 0.01
#define MIN_LENGTH 0.00001

namespace al{

/// A triplet of speakers
struct SpeakerTriple{
	int s1;
	Vec3d s1Vec;
	int s2;
	Vec3d s2Vec;
	int s3;
	Vec3d s3Vec;
	Vec3d vec[3];
	Mat3d mat;

	void loadVectors(const std::vector<Speaker>& spkrs);
};


/// Vector-based amplitude panner
class Vbap : public Spatializer{
public:

	/// @param[in] sl	A speaker layout
	Vbap(const SpeakerLayout &sl);

	/// Add triplet of speakers
	void addTriple(const SpeakerTriple& st);

	Vec3d computeGains(const Vec3d& vecA, const SpeakerTriple& speak);


	// 2D VBAP, find pairs of speakers.
	void findSpeakerPairs(const std::vector<Speaker>& spkrs);

	bool isCrossing(Vec3d c, Vec3d v, const SpeakerTriple& trip);

	// 3D VBAP, find triplets.
	void findSpeakerTriplets(const std::vector<Speaker>& spkrs);

	void compile(Listener& listener);

	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample);

	void print();

private:
	std::vector<SpeakerTriple> mTriplets;
	unsigned mNumTriplets;
	Listener* mListener;
	unsigned int mCachedTripletIndex;
	bool mIs3D;
};

} // al::
#endif
