#include <cmath> // atan2, sin, cos
#include <cstring> // memcpy
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/sound/al_StereoPanner.hpp"
using namespace al;

StereoPanner::StereoPanner(const SpeakerLayout& sl)
:	Spatializer(sl), mNumSpeakers(0)
{
	mNumSpeakers = mSpeakers.size();
	if(mNumSpeakers != 2) {
		printf("Stereo Panner Requires exactly 2 speakers (%i used), no panning will occur!\n", mNumSpeakers);
	}
}

void StereoPanner::compile(Listener& listener){
	mListener = &listener;
}

void equalPowerPan(const Vec3d& relPos, float &gainL, float &gainR){
	double panVal = 1.0 - std::fabs(std::atan2(relPos.z, relPos.x)/M_PI);
	static const double pi_2 = 1.5707963267948966192313216916398;
	gainL = std::cos(pi_2*panVal);
	gainR = std::sin(pi_2*panVal);
}

void StereoPanner::renderSample(AudioIOData& io, const Pose& listeningPose, float sample, int frameIndex){
	Vec3d vec = listeningPose.vec();
	Quatd srcRot = listeningPose.quat();
	vec = srcRot.rotate(vec);
	if(mNumSpeakers == 2 && mEnabled)
	{
		float gainL, gainR;
		equalPowerPan(vec, gainL, gainR);

		io.bufferOut().at(frameIndex,0) += gainL*sample;
		io.bufferOut().at(frameIndex,1) += gainR*sample;
	}
	else // don't pan
	{
		for(int i = 0; i < mNumSpeakers; i++)
			io.bufferOut().at(frameIndex,i) = sample;
	}
}

void StereoPanner::renderBuffer(AudioIOData& io, const Pose& listeningPose, const float *samples, int numFrames){
	Vec3d vec = listeningPose.vec();
	Quatd srcRot = listeningPose.quat();
	vec = srcRot.rotate(vec);
	if(mNumSpeakers == 2 && mEnabled)
	{
		float gainL, gainR;
		equalPowerPan(vec, gainL, gainR);

		for(int i = 0; i < numFrames; i++)
		{
			io.bufferOut().at(i,0) += gainL*samples[i];
			io.bufferOut().at(i,1) += gainR*samples[i];
		}
	}
	else // dont pan
	{
		for(int j = 0; j < mNumSpeakers; j++){
			for(int i = 0; i < numFrames; i++){
				io.bufferOut().at(i,j) = samples[i];
			}
		}
	}
}
