
#include <iostream>
#include <sstream>

#include <cstring>  // For memset
#include <cassert>

#include "alloaudio/al_BassManager.hpp"

#include "alloaudio/butter.h"

using namespace al;

BassManager::BassManager()
{
}

BassManager::~BassManager()
{
	for (int i = 0; i < mNumChnls; i++) {
		butter_free(mLopass1[i]);
		butter_free(mLopass2[i]);
		butter_free(mHipass1[i]);
		butter_free(mHipass2[i]);
	}
}

void BassManager::configure(int numChnls, double sampleRate,
                            int framesPerBuffer, float frequency,
                            BassManager::bass_mgmt_mode_t mode)
{
	mNumChnls = numChnls;
	mFramesPerSec = sampleRate;
	mFramesPerBuffer = framesPerBuffer;
	allocateChannels(mNumChnls);


	mBass_buf = new double[mFramesPerBuffer];
	mFilt_out = new double[mFramesPerBuffer];
	mFilt_low = new double[mFramesPerBuffer];
	mIn_buf = new double[mFramesPerBuffer];

	setBassManagementFreq(frequency);
	setBassManagementMode(mode);
}

void BassManager::setBassManagementFreq(double frequency)
{
	if (frequency > 0) {
		for (int i = 0; i < mNumChnls; i++) {
			butter_set_fc(mLopass1[i], frequency);
			butter_set_fc(mLopass2[i], frequency);
			butter_set_fc(mHipass1[i], frequency);
			butter_set_fc(mHipass2[i], frequency);
		}
	}
}

void BassManager::setBassManagementMode(bass_mgmt_mode_t mode)
{
	if (mode >= 0 && mode < BASSMODE_COUNT) {
		mBassManagementMode = mode;
	}
}

void BassManager::setSwIndeces(int i1, int i2, int i3, int i4)
{
	swIndex[0] = i1;
	swIndex[1] = i1;
	swIndex[2] = i1;
	swIndex[3] = i1;
}

void BassManager::onAudioCB(AudioIOData &io)
{
	int i, chan = 0;
	int nframes = io.framesPerBuffer();
	assert(nframes == mFramesPerBuffer);

	memset(mBass_buf, 0, nframes * sizeof(double));
	for (chan = 0; chan < mNumChnls; chan++) {
		const float *in = io.outBuffer(chan);  // Yes, the input here is the output from previous runs for the io object
//		float *out = io.outBuffer(chan);
		double filt_temp[nframes];
		double *buf = mBass_buf;

		for (i = 0; i < nframes; i++) {
			mIn_buf[i] = *in++;
		}
		switch (mBassManagementMode) {
		case BASSMODE_NONE:
			break;
		case BASSMODE_MIX:
			for (i = 0; i < nframes; i++) {
				mFilt_low[i] = mIn_buf[i];
			}
			break;
		case BASSMODE_LOWPASS:
			butter_next(mLopass1[chan], mIn_buf, filt_temp, nframes);
			butter_next(mLopass2[chan], filt_temp, mFilt_low, nframes);
			break;
		case BASSMODE_HIGHPASS:
			for (i = 0; i < nframes; i++) {
				mFilt_low[i] = mIn_buf[i];
			}
			butter_next(mHipass1[chan], mIn_buf, filt_temp, nframes);
			butter_next(mHipass2[chan], filt_temp, mFilt_out, nframes);
			for (i = 0; i < nframes; i++) {
				mIn_buf[i] = mFilt_out[i];
			}
			break;
		case BASSMODE_FULL:
			butter_next(mLopass1[chan], mIn_buf, filt_temp, nframes);
			butter_next(mLopass2[chan], filt_temp, mFilt_low, nframes);
			butter_next(mHipass1[chan], mIn_buf, filt_temp, nframes);
			butter_next(mHipass2[chan], filt_temp, mFilt_out, nframes);
			for (i = 0; i < nframes; i++) {
				mIn_buf[i] = mFilt_out[i]; /* a bit inefficient to copy here, but makes code simpler below */
			}
			break;
		default:
			break;
		}
		for (i = 0; i < nframes; i++) { /* accumulate SW signal */
			*buf++ += mFilt_low[i];
		}
	}
	if (mBassManagementMode != BASSMODE_NONE) {
		int sw;
		for(sw = 0; sw < 4; sw++) {
			if (swIndex[sw] < 0) continue;
			float *out = io.outBuffer(swIndex[sw]);
			memset(out, 0, nframes * sizeof(float));
			for (i = 0; i < nframes; i++) {
				*out++ = mBass_buf[i];
			}
		}
	}
}


int BassManager::chanIsSubwoofer(int index)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (swIndex[i] == index && mBassManagementMode != BASSMODE_NONE) return 1;
	}
	return 0;
}

void BassManager::initializeData()
{

}

void BassManager::allocateChannels(int numChnls)
{
	swIndex[0] = numChnls - 1;
	swIndex[1] =  swIndex[2] = swIndex[3] = -1;

	mLopass1.resize(numChnls);
	mLopass2.resize(numChnls);
	mHipass1.resize(numChnls);
	mHipass2.resize(numChnls);

	for (int i = 0; i < numChnls; i++) {
		mLopass1[i] = butter_create(mFramesPerSec, BUTTER_LP);
		mLopass2[i] = butter_create(mFramesPerSec, BUTTER_LP);
		mHipass1[i] = butter_create(mFramesPerSec, BUTTER_HP);
		mHipass2[i] = butter_create(mFramesPerSec, BUTTER_HP);
	}
}
