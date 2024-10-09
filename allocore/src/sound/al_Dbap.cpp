#include "allocore/io/al_AudioIO.hpp"
#include "allocore/sound/al_Dbap.hpp"

namespace al{

Dbap::Dbap(const SpeakerLayout &sl, float focus)
	:	Spatializer(sl), mListener(NULL), mNumSpeakers(0), mFocus(focus)
{}

void Dbap::compile(Listener& listener){
	mListener = &listener;
	mNumSpeakers = mSpeakers.size();
	printf("DBAP Compiled with %d speakers\n", mNumSpeakers);

	for(int i = 0; i < mNumSpeakers; i++)
	{
		mSpeakerVecs[i] = mSpeakers[i].vec();
		mDeviceChannels[i] = mSpeakers[i].deviceChannel;
	}
}

void Dbap::renderBuffer(AudioIOData& io, const Pose& src, const float *samples, int numFrames){

	for(int k = 0; k < mNumSpeakers; ++k) {
		float gain = 1.f;
		if(mEnabled){
			auto vec = src.pos() - mSpeakerVecs[k];
			float dist = vec.mag();
			gain = 1.f / (1.f + dist);
			gain = powf(gain, mFocus);
		}

		/*
		float * out = io.outBuffer(mDeviceChannels[k]);
		for(int i = 0; i < numFrames; ++i){
			out[i] += gain * samples[i];
		}*/
		for(int i = 0; i < numFrames; ++i){
			io.bufferOut().at(i, mDeviceChannels[k]) += gain * samples[i];
		}
	}

}

void Dbap::renderSample(AudioIOData& io, const Pose& src, float sample, int frameIndex){
	for(int i = 0; i < mNumSpeakers; ++i){
		float gain = 1.f;
		if(mEnabled){
			auto vec = src.pos() - mSpeakerVecs[i];
			float dist = vec.mag();
			gain = 1.f / (1.f + dist);
			gain = powf(gain, mFocus);
		}

		io.bufferOut().at(frameIndex, mDeviceChannels[i]) += gain*sample;
	}
}

void Dbap::print() {
	printf("Using DBAP Panning- need to add panner info for print function\n");
}

} // al::
