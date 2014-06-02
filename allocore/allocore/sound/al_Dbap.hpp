#ifndef INCLUDE_AL_PANNING_DBAP
#define INCLUDE_AL_PANNING_DBAP

#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Quat.hpp"
//#include "allocore/spatial/al_CoordinateFrame.hpp"

namespace al{
	
#define DBAP_MAX_DIST 100
#define DBAP_SPREAD 3 // > 1 adds width, < 1 narrows

class Dbap : public Spatializer{
public:
	
    Dbap(SpeakerLayout &sl) : Spatializer(sl){}
    
	void dump() {
		printf("Using DBAP Panning- need to add panner info for dump function\n");
	}
		
	void compile(Listener& listener){
		mListener = &listener;
		
	}
	
	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample){
        
		for (unsigned i = 0; i < mSpeakers.size(); ++i)
        {
			Speaker spkr = mSpeakers[i];
            Vec3d vec = Vec3d(relpos);
            vec.normalize();
			Vec3d vecSpkr = spkr.vec();
            vecSpkr.normalize();
            
            vec -= vecSpkr;
			float dist = vec.mag() / 2.f;
            dist = powf(dist, DBAP_SPREAD);
            float gain = 1.f / (1.f + DBAP_MAX_DIST*dist);
            
			io.out(spkr.deviceChannel,frameIndex) += gain*sample;
		}
	}
	
	
private:
	Listener* mListener;
};
	
	

} // al::

#endif
