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

void Dbap::perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples){
	for (unsigned k = 0; k < mNumSpeakers; ++k)
	{
        float gain = 1.f;
        float prevGain = 1.f;
        if(mEnabled)
        {
            float foc = mFocus;
            
            foc *= src.mSoundFocus; //0 to 1
            
            Vec3d vec = relpos - mSpeakerVecs[k];
            float dist = vec.mag();
            gain = 1.f / powf((1.f + dist), foc);
            if(mFocus > 1) gain *= foc;
            
            Vec3d prevVec = src.posHistory()[1] - mSpeakerVecs[k];
            float prevDist = dist = prevVec.mag();
            prevGain = 1.f / powf((1.f + prevDist), foc);
            if(mFocus > 1) prevGain *= foc;
        }

		float * out = io.outBuffer(mDeviceChannels[k]);
		for(int i = 0; i < numFrames; ++i)
        {
            float f = (float)i/(float)(numFrames-1);
            float g = f*gain + (1.f - f)*prevGain;
			out[i] += g * samples[i];
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
            float foc = mFocus;
            
            foc *= src.mSoundFocus; //0 to 1
            
            Vec3d vec = relpos - mSpeakerVecs[i];
            float dist = vec.mag();
            gain = 1.f / powf((1.f + dist), foc);
            //gain = 1.f / (1.f + dist);
            //gain = powf(gain, mFocus);
            if(mFocus > 1) gain *= foc;
        }
        
        io.out(mDeviceChannels[i], frameIndex) += gain*sample;
    }
}
    
//#else
//void Dbap::perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample){
//    
//    for (unsigned i = 0; i < mNumSpeakers; ++i){
//        
//        float gain = 1.f;
//        if(mEnabled)
//        {
//            Vec3d vec = relpos - mSpeakerVecs[i];
//            float dist = vec.mag();
//            gain = 1.f / (1.f + dist);
//            gain = powf(gain, mFocus);
//        }
//        float *buf = outputBuffers[mDeviceChannels[i]];
//        buf[frameIndex] += gain*sample;
//
//    }
//}
//    
//    
//    
///// Per Buffer Processing
//void Dbap::perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples)
//    {
//        for (unsigned i = 0; i < mNumSpeakers; ++i)
//        {
//            float gain = 1.f;
//            if(mEnabled)
//            {
//                Vec3d vec = relpos - mSpeakerVecs[i];
//                float dist = vec.mag();
//                gain = 1.f / (1.f + dist);
//                gain = powf(gain, mFocus);
//            }
//            
//            float *buf = outputBuffers[mDeviceChannels[i]];
//            
//            float *samps = samples;
//            for(int j = 0; j < numFrames; j++)
//                *buf++ += gain* *samps++;
//        }
//    }
//#endif

void Dbap::print() {
	printf("Using DBAP Panning- need to add panner info for print function\n");
}

} // al::
