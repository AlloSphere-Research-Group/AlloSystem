#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>		/* memset() */
#include <cmath>
#include <cassert>

#include "allocore/io/al_AudioIOData.hpp"

namespace al{

template <class T>
void zero(T * buf, int n){ memset(buf, 0, n*sizeof(T)); }


AudioBackend::AudioBackend()
:	mIsOpen(false), mIsRunning(false)
{}

//==============================================================================

AudioDeviceInfo::AudioDeviceInfo(int deviceNum)
:	mID(deviceNum), mChannelsInMax(0), mChannelsOutMax(0), mDefaultSampleRate(0.0)
{}

bool AudioDeviceInfo::valid() const { return true; }
int AudioDeviceInfo::id() const { return mID; }
const char * AudioDeviceInfo::name() const { return mName; }
int AudioDeviceInfo::channelsInMax() const { return mChannelsInMax; }
int AudioDeviceInfo::channelsOutMax() const { return mChannelsOutMax; }
double AudioDeviceInfo::defaultSampleRate() const { return mDefaultSampleRate; }

void AudioDeviceInfo::setName(char *name) { strncpy(mName, name, 127); mName[127] = '\0'; }
void AudioDeviceInfo::setID(int iD) { mID = iD;}
void AudioDeviceInfo::setChannelsInMax(int num) { mChannelsInMax = num;}
void AudioDeviceInfo::setChannelsOutMax(int num) { mChannelsOutMax = num;}
void AudioDeviceInfo::setDefaultSampleRate(double rate) { mDefaultSampleRate = rate;}

//==============================================================================

AudioIOData::AudioIOData(void * userData)
:	mImpl(NULL), mUser(userData), mFrame(0),
	mFramesPerBuffer(0), mFramesPerSecond(0),
	mBufI(0), mBufO(0), mBufB(0), mBufT(0), mNumI(0), mNumO(0), mNumB(0),
	mGain(1), mGainPrev(1)
{
}

AudioIOData::~AudioIOData(){
	deleteBuf(mBufI);
	deleteBuf(mBufO);
	deleteBuf(mBufB);
	deleteBuf(mBufT);
}

void AudioIOData::zeroBus(){ zero(mBufB, framesPerBuffer() * mNumB); }
void AudioIOData::zeroOut(){ zero(mBufO, channelsOut() * framesPerBuffer()); }

int AudioIOData::channelsIn () const { return mNumI; }
int AudioIOData::channelsOut() const { return mNumO; }
int AudioIOData::channelsBus() const { return mNumB; }

double AudioIOData::framesPerSecond() const { return mFramesPerSecond; }
double AudioIOData::time() const {
	assert(mImpl);
	return mImpl->time();
}
double AudioIOData::time(int frame) const { return (double)frame / framesPerSecond() + time(); }
int AudioIOData::framesPerBuffer() const { return mFramesPerBuffer; }
double AudioIOData::secondsPerBuffer() const { return (double)framesPerBuffer() / framesPerSecond(); }

} // al::
