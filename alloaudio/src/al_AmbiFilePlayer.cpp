
#include <iostream>
#include <cassert>

#include "alloaudio/al_AmbiFilePlayer.hpp"

using namespace al;

AmbiFilePlayer::AmbiFilePlayer(string fullPath, bool loop, int bufferFrames, SpeakerLayout &layout)
    : SoundFileBuffered(fullPath, loop, bufferFrames),
      mReadBuffer(nullptr),
      mDone(false),
      mBufferSize(bufferFrames),
      mGain("Gain", "", 0.25),
      mPlaying(false)
{
	// Create spatializer
	mDecoder = new AmbiDecode(getFileDimensions(), getFileOrder(), layout.numSpeakers());
	mDecoder->setSpeakers(&(layout.speakers()));
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
	mDecoder = new AmbiTunedDecoder();
	static_cast<AmbiTunedDecoder *>(mDecoder)->setConfiguration(configPath);
	mReadBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
	mDeinterleavedBuffer = (float *) calloc(mBufferSize * channels(), sizeof(float));
}


AmbiFilePlayer::~AmbiFilePlayer()
{
	pause();
	free(mReadBuffer);
	free(mDeinterleavedBuffer);
	delete mDecoder;
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

void AmbiFilePlayer::play()
{
	mPlaying = true;
}

void AmbiFilePlayer::pause()
{
	mPlaying = false;
}

int AmbiFilePlayer::numDeviceChannels()
{
	int maxIndex = 0;
	for (int i = 0; i < mDecoder->numSpeakers(); i++) {
		int index = mDecoder->speaker(i).deviceChannel;
		if (index > maxIndex) { maxIndex = index; }
	}
	return maxIndex + 1;
}

void AmbiFilePlayer::onAudioCB(AudioIOData &io)
{
	if (!mPlaying) {return;}

	int numFrames = io.framesPerBuffer();

	assert(mBufferSize >= numFrames);

	int framesRead = read(mReadBuffer, numFrames);

	// We need to de-interleave the file buffers
	for (int chan = 0; chan < channels(); chan++) {
		for (int j = 0; j < framesRead; j++) {
			float sample = (mReadBuffer[j*channels() + chan] * mGain.get());
			mDeinterleavedBuffer[chan*mBufferSize + j] = sample;
		}
	}

	mDecoder->decode(&io.out(0,0), mDeinterleavedBuffer, framesRead);

	if (repeats() > 0) {
		mDone = true;
	}
}


