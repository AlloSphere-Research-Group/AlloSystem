
#include <iostream>
#include <cassert>

#include "alloaudio/al_AmbiFilePlayer.hpp"

using namespace al;

AmbiFilePlayer::AmbiFilePlayer(string fullPath, bool loop, int bufferFrames, SpeakerLayout layout)
    : SoundFileBuffered(fullPath, loop, bufferFrames),
      mSpatializer(nullptr),
      mReadBuffer(nullptr),
      mDone(false),
      mBufferSize(bufferFrames) /*,
		      mProcessAdding(false)*/ ,
      mGain("Gain", "", 0.25)
{
	AmbisonicsConfig config = fileAmbisonicsConfig();
	// Create spatializer
	mSpatializer = new AmbisonicsSpatializer(layout, config.dimensions(), config.order(), config.flavor());
	mSpatializer->numFrames(mBufferSize);
	mReadBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
}

AmbiFilePlayer::~AmbiFilePlayer()
{
	free(mReadBuffer);
}

//bool AmbiFilePlayer::processAdding() const
//{
//	return mProcessAdding;
//}

//void AmbiFilePlayer::setProcessAdding(bool processAdding)
//{
//	mProcessAdding = processAdding;
//}

AmbisonicsConfig AmbiFilePlayer::fileAmbisonicsConfig()
{
	int numChannels = channels();
	AmbisonicsConfig config;
	// Can the weighing flavor be determined from the audiofile?
	switch(numChannels) {
	case 3:
		config.setOrder(1);
		config.setDimensions(2);
		break;
	case 4:
		config.setOrder(1);
		config.setDimensions(3);
		break;
	case 9:
		config.setOrder(2);
		config.setDimensions(3);
		break;
	case 16:
		config.setOrder(3);
		config.setDimensions(3);
		break;
	default:
		config.setOrder(-1);
		config.setDimensions(-1);
	}
	return config;
}

bool AmbiFilePlayer::done() const
{
	return mDone;
}

void AmbiFilePlayer::setDone(bool done)
{
	mDone = done;
}

void AmbiFilePlayer::onAudioCB(AudioIOData &io)
{
	int numFrames = io.framesPerBuffer();

	assert(AUDIO_BLOCK_SIZE == numFrames);

	mSpatializer->prepare();

	float * ambiChans = mSpatializer->ambiChans();

	int framesRead = read(mReadBuffer, numFrames);

	if (repeats() > 0) {
		mDone = true;
	}

	for (int chan = 0; chan < channels(); chan++) {
		for (int j = 0; j < framesRead; j++) {
			// We need to de-interleave the buffers
			float sample = (mReadBuffer[j*4 + chan] * mGain.get());
			ambiChans[j + chan*AUDIO_BLOCK_SIZE] = sample;
		}
	}
	mSpatializer->finalize(io);
}


