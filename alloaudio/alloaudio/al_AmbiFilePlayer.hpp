#ifndef AL_AMBIFILEPLAYER_H
#define AL_AMBIFILEPLAYER_H

#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "allocore/io/al_AudioIOData.hpp"
#include "allocore/system/al_Parameter.hpp"
#include "alloaudio/al_SoundfileBuffered.hpp"
#include "alloaudio/al_AmbiTunedDecoder.hpp"

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

class AmbiFilePlayer : public AudioCallback, public SoundFileBuffered
{

public:
	AmbiFilePlayer(std::string fullPath, bool loop, int bufferFrames,
	               SpeakerLayout &layout);

	~AmbiFilePlayer();

	virtual void onAudioCB(AudioIOData& io) override;

	bool done() const;
	void setDone(bool done);

private:

	AmbisonicsConfig fileAmbisonicsConfig();

	// Internal

	AmbiDecode *mDecoder;
	float *mReadBuffer;
	float *mDeinterleavedBuffer;
	bool mDone;
	int mBufferSize;

	//Parameters
	Parameter mGain;

};

/** @} */
}

#endif // AL_AMBIFILEPLAYER_H
