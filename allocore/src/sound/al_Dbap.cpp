#include "allocore/sound/al_Dbap.hpp"

namespace al{

Dbap::Dbap(const SpeakerLayout &sl, float spread)
:	Spatializer(sl), mNumSpeakers(0), mSpread(spread)
{}

void Dbap::compile(Listener& listener){
	mListener = &listener;
	mNumSpeakers = mSpeakers.size();
	printf("DBAP Compiled with %d speakers\n", mNumSpeakers);

	for(int i = 0; i < mNumSpeakers; i++)
	{
		mSpeakerVecs[i] = mSpeakers[i].vec();
		mSpeakerVecs[i].normalize();
		mDeviceChannels[i] = mSpeakers[i].deviceChannel;
	}
}

void Dbap::perform(
	AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples
){
	for (unsigned k = 0; k < mNumSpeakers; ++k)
	{
		Vec3d vec = relpos.normalized();
		vec -= mSpeakerVecs[k];
		float dist = vec.mag() / 2.f; // [0, 1]
		dist = powf(dist, mSpread);
		float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);

		float * out = io.outBuffer(mDeviceChannels[k]);
		for(int i = 0; i < numFrames; ++i){
			out[i] += gain * samples[i];
		}
	}
}

void Dbap::dump() {
	printf("Using DBAP Panning- need to add panner info for dump function\n");
}

} // al::
