#ifndef INCLUDE_AL_STEREO_PANNER_HPP
#define INCLUDE_AL_STEREO_PANNER_HPP

#include <cmath>
#include <memory>
#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Constants.hpp"

namespace al{

///
/// \brief The StereoPanner class
///
/// @ingroup allocore
class StereoPanner : public Spatializer{
public:
	StereoPanner(SpeakerLayout &sl) : Spatializer(sl), numSpeakers(0) {
		numSpeakers = mSpeakers.size();
		if(numSpeakers != 2) {
			printf("Stereo Panner Requires exactly 2 speakers (%i used), no panning will occur!\n", numSpeakers);
		}
	}

//	virtual void print() override {
//		printf("Using Stereo Panning- need to add panner info for print() function\n");
//	}

	/// Compile is
	virtual void compile(Listener& listener) override {
		mListener = &listener;
	}

	///Per Sample Processing
	virtual void renderSample(AudioIOData& io, const Pose& listeningPose, const float& sample, const int& frameIndex) override
	{
		Vec3d vec = listeningPose.vec();
		Quatd srcRot = listeningPose.quat();
		vec = srcRot.rotate(vec);
		if(numSpeakers == 2 && mEnabled)
		{
			float gainL, gainR;
			equalPowerPan(vec, gainL, gainR);

			io.out(0, frameIndex) += gainL*sample;
			io.out(1, frameIndex) += gainR*sample;
		}
		else // don't pan
		{
			for(int i = 0; i < numSpeakers; i++)
				io.out(i, frameIndex) = sample;
		}

	}

	/// Per Buffer Processing
    virtual void renderBuffer(AudioIOData& io, const Pose& listeningPose, const float *samples, const int& numFrames) override
	{
		Vec3d vec = listeningPose.vec();
		Quatd srcRot = listeningPose.quat();
		vec = srcRot.rotate(vec);
		if(numSpeakers == 2 && mEnabled)
		{
			float *bufL = io.outBuffer(0);
			float *bufR = io.outBuffer(1);

			float gainL, gainR;
			equalPowerPan(vec, gainL, gainR);

			for(int i = 0; i < numFrames; i++)
			{
				bufL[i] += gainL*samples[i];
				bufR[i] += gainR*samples[i];
			}
		}
		else // dont pan
		{
			for(int i = 0; i < numSpeakers; i++)
				memcpy(io.outBuffer(i), samples, sizeof(float)*numFrames);
		}
	}


private:
	Listener* mListener;
	int numSpeakers;
	std::vector<float> mBuffer;

	void equalPowerPan(const Vec3d& relPos, float &gainL, float &gainR)
	{
		double panVal = 1.0 - std::fabs(std::atan2(relPos.z, relPos.x)/M_PI);
		gainL = std::cos((M_PI/2.0)*panVal);
		gainR = std::sin((M_PI/2.0)*panVal);
	}

};


} // al::

#endif
