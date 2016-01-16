
#include <iostream>
#include <cassert>

#include "alloaudio/al_AmbiFilePlayer.hpp"

using namespace al;

AmbiFilePlayer::AmbiFilePlayer(string fullPath, bool loop, int bufferFrames, SpeakerLayout &layout)
    : SoundFileBuffered(fullPath, loop, bufferFrames),
      mReadBuffer(nullptr),
      mDone(false),
      mBufferSize(bufferFrames),
      mGain("Gain", "", 0.25)
{
	AmbisonicsConfig config = fileAmbisonicsConfig();
	// Create spatializer
	mDecoder = new AmbiDecode(config.dimensions(), config.order(), layout.numSpeakers(), config.flavor());
	mDecoder->setSpeakers(&(layout.speakers()));
	mReadBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
	mDeinterleavedBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
}

AmbiFilePlayer::~AmbiFilePlayer()
{
	free(mReadBuffer);
}

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

	assert(mBufferSize == numFrames);

	int framesRead = read(mReadBuffer, numFrames);

	float *outs = &io.out(0,0);

	// We need to de-interleave the file buffers
	for (int chan = 0; chan < channels(); chan++) {
		for (int j = 0; j < framesRead; j++) {
			float sample = (mReadBuffer[j*channels() + chan] * mGain.get());
			mDeinterleavedBuffer[chan*mBufferSize + j] = sample;
//			io.out(chan, j) = sample;
		}
	}

	mDecoder->decode(outs, mDeinterleavedBuffer, numFrames);

	if (repeats() > 0) {
		mDone = true;
	}
}


