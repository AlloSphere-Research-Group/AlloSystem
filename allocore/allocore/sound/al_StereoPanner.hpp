#ifndef INCLUDE_AL_PANNING_STEREO
#define INCLUDE_AL_PANNING_STEREO

#include "allocore/sound/al_AudioScene.hpp"
//#include "allocore/math/al_Quat.hpp"
////#include "allocore/spatial/al_CoordinateFrame.hpp"

namespace al{

class StereoPanner : public Spatializer{
public:
	
    StereoPanner(SpeakerLayout &sl) : Spatializer(sl), numSpeakers(0){}
    
	void dump() {
		printf("Using Stereo Panning- need to add panner info for dump function\n");
	}
		
	void compile(Listener& listener){
		mListener = &listener;
		numSpeakers = mSpeakers.size();
        if(numSpeakers == 2)
        {
            printf("Stereo Panner Compiled with %d speakers\n", numSpeakers);
            
            for(int i = 0; i < numSpeakers; i++)
            {
                speakerVecs[i] = mSpeakers[i].vec();
                speakerVecs[i].normalize();
                deviceChannels[i] = mSpeakers[i].deviceChannel;
            }
        }
        else
            printf("Stereo Panner Requires exactly 2 speakers, no panning will occur!\n", numSpeakers);
	}
	
#if !ALLOCORE_NO_PORTAUDIO

    
#else
    
    ///Per Sample Processing
    void perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample)
    {
        if(numSpeakers == 2 && enabled)
        {
            float *bufL = outputBuffers[deviceChannels[0]];
            float *bufR = outputBuffers[deviceChannels[1]];
            
            //Is the normalize function working correctly??? normalizing 0, 0, 0 is returning 1, 0, 0
            Vec3d zeroVec(0, 0, 0);
            Vec3d normalVec;
            if(relpos == zeroVec)
                normalVec = zeroVec;
            else
                normalVec = relpos.normalized();
           
            float panVal = (normalVec.x + 1) /2.f; //[0, 1], L to R
            
            float gainL = cos((M_PI/2)*panVal);
            float gainR = sin((M_PI/2)*panVal);
            
            bufL[frameIndex] = gainL*sample;
            bufR[frameIndex] = gainR*sample;
            
            
        }
        else // dont pan
        {
            for(int i = 0; i < numSpeakers; i++)
            {
                float *buf = outputBuffers[deviceChannels[i]];
                buf[frameIndex] = sample;
            }
        }
        
	}
    
    /// Per Buffer Processing
	void perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples)
    {
        if(numSpeakers == 2 && enabled)
        {
            float *bufL = outputBuffers[deviceChannels[0]];
            float *bufR = outputBuffers[deviceChannels[1]];
            
            //Is the normalize function working correctly??? normalizing 0, 0, 0 is returning 1, 0, 0
            Vec3d zeroVec(0, 0, 0);
            Vec3d normalVec;
            if(relpos == zeroVec)
                normalVec = zeroVec;
            else
                normalVec = relpos.normalized();
            
            float panVal = (normalVec.x + 1) /2.f; //[0, 1], L to R
            
            float gainL = cos((M_PI/2)*panVal);
            float gainR = sin((M_PI/2)*panVal);
            
            for(int i = 0; i < numFrames; i++)
            {
                bufL[i] = gainL*samples[i];
                bufR[i] = gainR*samples[i];
            }
            
        }
        else // dont pan
        {
            for(int i = 0; i < numSpeakers; i++)
            {
                float *buf = outputBuffers[deviceChannels[i]];
                for(int j = 0; j < numFrames; j++)
                {
                    buf[j] = samples[i];
                }
            }
        }

	}
    
#endif
    
	
private:
	Listener* mListener;
	Vec3f speakerVecs[DBAP_MAX_NUM_SPEAKERS];
	int deviceChannels[DBAP_MAX_NUM_SPEAKERS];
	int numSpeakers;
};
	
	
} // al::

#endif
