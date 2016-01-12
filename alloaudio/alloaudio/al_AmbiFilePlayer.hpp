#ifndef AL_AMBIFILEPLAYER_H
#define AL_AMBIFILEPLAYER_H

#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "allocore/io/al_AudioIOData.hpp"
#include "allocore/system/al_Parameter.hpp"
#include "alloaudio/al_SoundfileBuffered.hpp"

namespace al {

using namespace std;

class AmbisonicsConfig
{
public:
	AmbisonicsConfig(int order = 3, int dimensions = 3, int flavor = 1)
	    : mOrder(order), mDimensions(dimensions) , mFlavor(flavor)
	{}

	int order() const { return mOrder; }
	int dimensions() const { return mDimensions; }
	int flavor() const { return mFlavor; }

	void setOrder(int order) { mOrder = order; }
	void setDimensions(int dimensions) { mDimensions = dimensions; }
	void setFlavor(int flavor) { mFlavor = flavor; }

private:
	int mOrder;
	int mDimensions;
	int mFlavor;
};

// TODO: Do this better (remove this macro)
#define AUDIO_BLOCK_SIZE 1024

class AmbiFilePlayer : public AudioCallback, public SoundFileBuffered
{

public:
	AmbiFilePlayer(std::string fullPath, bool loop = false, int bufferFrames = 1024,
	               SpeakerLayout layout = OctalSpeakerLayout());

	~AmbiFilePlayer();

	virtual void onAudioCB(AudioIOData& io) override;

	// TODO implement process adding instead of process replacing
//	bool processAdding() const;
//	void setProcessAdding(bool processAdding);

	bool done() const;
	void setDone(bool done);

private:

	AmbisonicsConfig fileAmbisonicsConfig();

	// Internal
//	SoundFileBuffered mSoundFile;
	AmbisonicsSpatializer *mSpatializer;
	float *mReadBuffer;
	bool mDone;
	int mBufferSize;

	//Parameters
	Parameter mGain;

//	bool mProcessAdding;
};

/** @} */
}

#endif // AL_AMBIFILEPLAYER_H
