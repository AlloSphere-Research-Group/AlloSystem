#include "allocore/sound/al_Dbap.hpp"

namespace al{

Dbap::Dbap(const SpeakerLayout &sl, float focus)
:	Spatializer(sl), mNumSpeakers(0), mFocus(focus)
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

#if !ALLOCORE_GENERIC_AUDIOSCENE
    
void Dbap::perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples){
	for (unsigned k = 0; k < mNumSpeakers; ++k)
	{
        float gain = 1.f;
        if(mEnabled)
        {
            Vec3d vec = relpos - mSpeakerVecs[k];
            float dist = vec.mag();
            gain = 1.f / (1.f + dist);
            gain = powf(gain, mFocus);
        }

		float * out = io.outBuffer(mDeviceChannels[k]);
		for(int i = 0; i < numFrames; ++i){
			out[i] += gain * samples[i];
		}
	}
}
    
void Dbap::perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample)
{
    for (unsigned i = 0; i < mNumSpeakers; ++i)
    {
        float gain = 1.f;
        if(mEnabled)
        {
            Vec3d vec = relpos - mSpeakerVecs[i];
            float dist = vec.mag();
            gain = 1.f / (1.f + dist);
            gain = powf(gain, mFocus);
        }
        
        io.out(mDeviceChannels[i], frameIndex) += gain*sample;
    }
}
    
#else
void Dbap::perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample){
    
    for (unsigned i = 0; i < mNumSpeakers; ++i){
        
        float gain = 1.f;
        if(mEnabled)
        {
            Vec3d vec = relpos - mSpeakerVecs[i];
            float dist = vec.mag();
            gain = 1.f / (1.f + dist);
            gain = powf(gain, mFocus);
        }
        float *buf = outputBuffers[mDeviceChannels[i]];
        buf[frameIndex] += gain*sample;

    }
}
    
    
    
/// Per Buffer Processing
void Dbap::perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples)
    {
        for (unsigned i = 0; i < mNumSpeakers; ++i)
        {
            float gain = 1.f;
            if(mEnabled)
            {
                Vec3d vec = relpos - mSpeakerVecs[i];
                float dist = vec.mag();
                gain = 1.f / (1.f + dist);
                gain = powf(gain, mFocus);
            }
            
            float *buf = outputBuffers[mDeviceChannels[i]];
            
            float *samps = samples;
            for(int j = 0; j < numFrames; j++)
                *buf++ += gain* *samps++;
        }
    }
#endif

void Dbap::print() {
	printf("Using DBAP Panning- need to add panner info for print function\n");
}

} // al::
