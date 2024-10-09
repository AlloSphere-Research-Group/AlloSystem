#ifndef INCLUDE_AL_VBAP_HPP
#define INCLUDE_AL_VBAP_HPP

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

#include <map>
#include "allocore/sound/al_AudioScene.hpp"

#define MAX_NUM_VBAP_TRIPLETS 512
//#define MIN_VOLUME_TO_LENGTH_RATIO 0.01

#define MIN_VOLUME_TO_LENGTH_RATIO 0.000001
#define MIN_LENGTH 0.00001

namespace al{

class AudioIOData;

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
	int speakerIdx[3];

	// Speaker speakers[3];
	int speakerChan[3];
	int s1Chan;
	int s2Chan;
	int s3Chan;

	bool loadVectors(const std::vector<Speaker>& spkrs);
};


/// Vector-based amplitude panner
///
/// @ingroup allocore
class Vbap : public Spatializer{
public:

	/// @param[in] sl	A speaker layout
	Vbap(const SpeakerLayout &sl, bool is3D = false);

	///
	/// \brief Make an existing channel a phantom channel
	/// \param channelIndex the channel index of the phantom channel
	/// \param assignedOutputs the list of channel indeces for signal reassignment
	///
	/// Signals that should go out to phantom channels will be distributed among
	/// the channels listed in the assignedOutputs vector. This can be useful to
	/// force triangulation in unusual situations (e.g. three rings on a
	/// sphere...) but it can also be used creatively to make an area in space
	/// be reassigned somewhere else, or to a wider number of speakers.
	///
	void makePhantomChannel(int channelIndex, std::vector<int> assignedOutputs);

	void compile(Listener& listener) override;

	void renderSample(AudioIOData& io, const Pose& reldir, float sample, int frameIndex) override;
	void renderBuffer(AudioIOData& io, const Pose& reldir, const float *samples, int numFrames) override;

	void print() override;

	/// Manually add a triple from indeces to speakers
	void makeTriple(int s1, int s2, int s3 = -1);


	//Returns vector of triplets
	std::vector<SpeakerTriple> triplets() const;

private:
	std::vector<SpeakerTriple> mTriplets;
	std::map<int, std::vector<int> > mPhantomChannels;
	Listener* mListener;
	bool mIs3D;

	//	void setIs3D(bool is3D){mIs3D = is3D;}

	Vec3d computeGains(const Vec3d& vecA, const SpeakerTriple& speak);

	/// 2D VBAP, Build internal list of speaker pairs
	void findSpeakerPairs(const Speakers& spkrs);

	/// 3D VBAP, build list of internal speaker triplets
	void findSpeakerTriplets(const Speakers& spkrs);

	bool isCrossing(Vec3d c, Vec3d li, Vec3d lj, Vec3d ln, Vec3d lm);

	/// Manually add triplet of speakers, in case not set automatically
	void addTriple(const SpeakerTriple& st);

};

} // al::
#endif
