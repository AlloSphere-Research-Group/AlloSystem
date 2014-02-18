#ifndef INCLUDE_AL_PANNING_DBAP
#define INCLUDE_AL_PANNING_DBAP

#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Quat.hpp"
//#include "allocore/spatial/al_CoordinateFrame.hpp"

namespace al{
	


class Dbap : public Spatializer{
public:
	
	void dump() {
		printf("Using DBAP Panning- need to add panner info for dump function\n");
	}
		
	void compile(Listener& listener){
		mListener = &listener;
		numSpeakers = mSpeakers.size();
	}
	
	void perform(AudioIOData& io, SoundSource& src, Vec3d& relpos, const int& numFrames, int& frameIndex, float& sample){
        
		for (unsigned i = 0; i < numSpeakers; ++i){
			Speaker spkr = mSpeakers[i];
			Vec3d vec = Vec3d(relpos);
			//printf("(%f,%f,%f)\n",vec[0],vec[1],vec[2]);
			
			Vec3d vecSpkr = spkr.vec();
			Quatd srcRot = mListener->pose().quat();
			vecSpkr = srcRot.rotate(vecSpkr);

			vec+=vecSpkr;

			double dist = vec.mag();

			double gain = sample/dist;
			io.out(spkr.deviceChannel,frameIndex) += gain * 0.1;
		}
	}
	
	
private:
	Listener* mListener;
	unsigned numSpeakers;
};
	
	

} // al::

#endif
