
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
	// Create spatializer
	mDecoder = new AmbiDecode(getFileDimensions(), getFileOrder(), layout.numSpeakers());
	mDecoder->setSpeakers(layout.speakers());
	mReadBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
	mDeinterleavedBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
}

AmbiFilePlayer::AmbiFilePlayer(std::string fullPath, bool loop, int bufferFrames, string configPath)
    : SoundFileBuffered(fullPath, loop, bufferFrames),
      mReadBuffer(nullptr),
      mDone(false),
      mBufferSize(bufferFrames),
      mGain("Gain", "", 0.25)
{
	// Create spatializer
	mDecoder = new AmbiTunedDecoder(configPath);
	mReadBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
	mDeinterleavedBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
}


AmbiFilePlayer::~AmbiFilePlayer()
{
	free(mReadBuffer);
}

int AmbiFilePlayer::getFileDimensions()
{
	return AmbiBase::channelsToDimensions(channels());
}

int AmbiFilePlayer::getFileOrder()
{
	return  AmbiBase::channelsToOrder(channels());
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

	assert(mBufferSize >= numFrames);

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

	mDecoder->decode(outs, mDeinterleavedBuffer, framesRead);

	if (repeats() > 0) {
		mDone = true;
	}
}


