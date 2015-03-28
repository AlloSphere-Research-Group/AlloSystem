#ifndef INCLUDE_AL_PANNING_DBAP
#define INCLUDE_AL_PANNING_DBAP

#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Quat.hpp"
//#include "allocore/spatial/al_CoordinateFrame.hpp"

namespace al{

#define DBAP_MAX_NUM_SPEAKERS 192
#define DBAP_MAX_DIST 100

class Dbap : public Spatializer{
public:
	
    Dbap(SpeakerLayout &sl, float _focus = 1.f) : Spatializer(sl), numSpeakers(0), focus(_focus){}
    
	void dump() {
		printf("Using DBAP Panning- need to add panner info for dump function\n");
	}
		
	void compile(Listener& listener){
		mListener = &listener;
		numSpeakers = mSpeakers.size();
		printf("DBAP Compiled with %d speakers\n", numSpeakers);
		
		for(int i = 0; i < numSpeakers; i++)
		{
			speakerVecs[i] = mSpeakers[i].vec();
			//speakerVecs[i].normalize();
            //if(numSpeakers == 2) //psuedo-stereo panning hack
                //speakerVecs[i].x /= 6;
			deviceChannels[i] = mSpeakers[i].deviceChannel;
		}
			
	}
	
#if !ALLOCORE_NO_PORTAUDIO
    ///Per Sample Processing
    void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample)
    {
        //Is the normalize function working correctly??? normalizing 0, 0, 0 is returning 1, 0, 0
        Vec3d zeroVec(0, 0, 0);
        Vec3d normalVec;
        if(relpos == zeroVec)
            normalVec = zeroVec;
        else
            normalVec = relpos.normalized();
            
		for (unsigned i = 0; i < numSpeakers; ++i)
        {
            Vec3d vec = normalVec - speakerVecs[i];
			float dist = vec.mag() / 2.f; // [0, 1]
            dist = powf(dist, focus);
            float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);
            
            if(enabled)
                io.out(deviceChannels[i],frameIndex) += gain*sample;
            else
                io.out(deviceChannels[i],frameIndex) += sample;
		}
	}
    
    /// Per Buffer Processing
	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples)
    {
		for (unsigned i = 0; i < numSpeakers; ++i)
        {
            Vec3d vec = relpos.normalized();
            vec -= speakerVecs[i];
            float dist = vec.mag() / 2.f; // [0, 1]
            dist = powf(dist, focus);
            float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);
            
            float *buf = io.outBuffer(deviceChannels[i]);
			float *samps = samples;
            for(int j = 0; j < numFrames; j++)
				*buf++ += gain* *samps++;
		}
	}
    
#else
    
    ///Per Sample Processing
    void perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample)
    {
//        //Is the normalize function working correctly??? normalizing 0, 0, 0 is returning 1, 0, 0
//        Vec3d zeroVec(0, 0, 0);
//        Vec3d normalVec;
//        if(relpos == zeroVec)
//            normalVec = zeroVec;
//        else
//            normalVec = relpos.normalized();
        
		for (unsigned i = 0; i < numSpeakers; ++i)
        {
            Vec3d vec = relpos - speakerVecs[i];
            float dist = vec.mag();
            float gain = 1.f / (1.f + dist);
            gain = powf(gain, focus);
            
            float *buf = outputBuffers[deviceChannels[i]];
            
//            for(int j = 0; j < numFrames*1; j++)
//            {
//                outputBuffer[j] = 0.2f*(float)(rand()%10000)/10000.f;
//            }
            
            if(enabled)
                //io.out(deviceChannels[i],frameIndex) += gain*sample;
                buf[frameIndex] += gain*sample;
            else
                //io.out(deviceChannels[i],frameIndex) += sample;
                buf[frameIndex] += sample;
		}
	}
    

    
    /// Per Buffer Processing
	void perform(float** outputBuffers, SoundSource& src, Vec3d& relpos, const int& numFrames, float *samples)
    {
		for (unsigned i = 0; i < numSpeakers; ++i)
        {
            Vec3d vec = relpos.normalized();
            vec -= speakerVecs[i];
            float dist = vec.mag() / 2.f; // [0, 1]
            dist = powf(dist, focus);
            float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);
            
            //float *buf = io.outBuffer(deviceChannels[i]);
            float *buf = outputBuffers[deviceChannels[i]];
            
			float *samps = samples;
            for(int j = 0; j < numFrames; j++)
				*buf++ += gain* *samps++;
		}
	}
    
#endif
    
    ///Focus is (0, inf) with usable range typically [0.2, 5]. Default is 1.
    ///A denser speaker layout my benefit from a high focus > 1, and a sparse layout may benefit from focus < 1
    void setFocus(float _focus) { focus = _focus; }
	
private:
	Listener* mListener;
	Vec3f speakerVecs[DBAP_MAX_NUM_SPEAKERS];
	int deviceChannels[DBAP_MAX_NUM_SPEAKERS];
	int numSpeakers;
    float focus;
};
	
	
} // al::

#endif
