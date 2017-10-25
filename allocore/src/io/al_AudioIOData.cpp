#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring> /* memset() */

#include "allocore/io/al_AudioIOData.hpp"

namespace al {

template <class T>
void zero(T * buf, int n){ memset(buf, 0, n*sizeof(T)); }

//==============================================================================

AudioDeviceInfo::AudioDeviceInfo(int deviceNum)
    : mID(deviceNum), mChannelsInMax(0), mChannelsOutMax(0),
      mDefaultSampleRate(0.0) {}

bool AudioDeviceInfo::valid() const { return true; }
int AudioDeviceInfo::id() const { return mID; }
const char *AudioDeviceInfo::name() const { return mName; }
int AudioDeviceInfo::channelsInMax() const { return mChannelsInMax; }
int AudioDeviceInfo::channelsOutMax() const { return mChannelsOutMax; }
double AudioDeviceInfo::defaultSampleRate() const { return mDefaultSampleRate; }

void AudioDeviceInfo::setName(char *name) {
	strncpy(mName, name, 127);
	mName[127] = '\0';
}
void AudioDeviceInfo::setID(int iD) { mID = iD; }
void AudioDeviceInfo::setChannelsInMax(int num) { mChannelsInMax = num; }
void AudioDeviceInfo::setChannelsOutMax(int num) { mChannelsOutMax = num; }
void AudioDeviceInfo::setDefaultSampleRate(double rate) {
	mDefaultSampleRate = rate;
}

//==============================================================================

AudioIOData::AudioIOData(void *userData)
    : mUser(userData), mFrame(0), mFramesPerBuffer(0), mFramesPerSecond(0),
      mBufI(nullptr), mBufO(nullptr), mBufB(nullptr), mBufT(nullptr), mNumI(0),
      mNumO(0), mNumB(0), mGain(1), mGainPrev(1) {}

AudioIOData::~AudioIOData() {
	deleteBuf(mBufI);
	deleteBuf(mBufO);
	deleteBuf(mBufB);
	deleteBuf(mBufT);
}

void AudioIOData::zeroBus() { zero(mBufB, framesPerBuffer() * mNumB); }
void AudioIOData::zeroOut() { zero(mBufO, channelsOut() * framesPerBuffer()); }

int AudioIOData::channelsIn() const { return mNumI; }
int AudioIOData::channelsOut() const { return mNumO; }
int AudioIOData::channelsBus() const { return mNumB; }

double AudioIOData::framesPerSecond() const { return mFramesPerSecond; }
int AudioIOData::framesPerBuffer() const { return mFramesPerBuffer; }
double AudioIOData::secondsPerBuffer() const {
	return (double)framesPerBuffer() / framesPerSecond();
}

} // al::
