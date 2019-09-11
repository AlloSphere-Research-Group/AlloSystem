#ifndef INC_AL_UTIL_VCR_HPP
#define INC_AL_UTIL_VCR_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Quick interface for capturing images & sounds to disk.
	Sounds stored as .WAV, images as .JPG sequence.

	File author(s):
	Graham Wakefield, 2012, grrrwaaa@gmail.com
*/

#include <stdio.h>
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/types/al_Array.hpp"
#include "Gamma/SoundFile.h"

namespace al {

class VCR : public AudioCallback {
public:

	// unsigned (and unsigned long on a 32-bit compiler), at 44.1kHz, can count about 27 hours worth of samples.
	// unsigned long long however can count 13264000 years.
	typedef unsigned long long sampletime;

	VCR(int channels=2, double sampleRate=44100, int bufferSize=8192, double sleep = 0.01);
	~VCR();

	// path should have a trailing slash:
	void setPath(std::string path);

	///! Start & stop recording:
	void start(AudioIO * io);
	void stop();

	///! store a new image frame:
	void image(Array& arr);

	unsigned frame() { return mImageFrame; }
	unsigned savedFrame() { return mImageCount; }

	virtual void onAudioCB(AudioIOData& io);

protected:

	void * audioThreadMethod();
	void writeToOpenSoundFile(gam::SoundFile& sf);

	void writeImages(Image& image);
	void * imageThreadMethod();

	static void * audioThreadFunc(void * userData) {
		VCR * self = (VCR *)userData;
		return self->audioThreadMethod();
	}

	static void * imageThreadFunc(void * userData) {
		VCR * self = (VCR *)userData;
		return self->imageThreadMethod();
	}

	std::string mPath;
	al_sec mSleep;

	// audio settings:
	AudioIO * mAudio;
	std::vector<float> mAudioRing;
	sampletime mAudioReadIndex, mAudioWriteIndex;
	unsigned mAudioOverflows, mAudioUnderflows;
	unsigned mAudioChans, mAudioFrames;
	double mSR;

	// image settings:
	std::vector<Array> mImageRing;
	unsigned mImageReadIndex, mImageWriteIndex;
	unsigned mImageFrame, mImageCount;

	Thread audioThread, imageThread;
	bool mActive;
};


inline VCR::VCR(int channels, double sampleRate, int bufferSize, double sleep)
:	mSleep(sleep),

	mAudio(0),
	mAudioOverflows(0),
	mAudioUnderflows(0),
	mAudioChans(channels),
	mAudioFrames(bufferSize),
	mSR(sampleRate),
	mAudioReadIndex(0),
	mAudioWriteIndex(0),

	mImageReadIndex(0),
	mImageWriteIndex(0),
	mImageFrame(0),
	mImageCount(0)

{
	mPath = "/";
	mAudioRing.resize(mAudioFrames*mAudioChans);
	mImageRing.resize(32);

	mActive = 1;
	audioThread.start(audioThreadFunc, this);
	imageThread.start(imageThreadFunc, this);
}

inline VCR::~VCR() {
	mActive = 0;
	audioThread.join();
	imageThread.join();
}

// path should have a trailing slash:
inline void VCR::setPath(std::string path) { mPath = path; }

inline void VCR::start(AudioIO * io) {
	if (mAudio == 0) {
		mImageCount = 0;
		mImageFrame = 0;
		mAudio = io;
		mAudio->append(*this);
	}
}

inline void VCR::stop() {
	if (mAudio) {
		mAudio->remove(*this);
		mAudio = 0;
	}
}

inline void VCR::onAudioCB(AudioIOData& io) {
	// cached, because it may be being used in a different thread
	sampletime r = mAudioReadIndex;
	sampletime w = mAudioWriteIndex;

	unsigned channels = io.channelsOut();
	if (channels > mAudioChans) channels = mAudioChans;

	unsigned numAudioFrames = io.framesPerBuffer();
	unsigned numSamples = numAudioFrames * channels;
	unsigned length = mAudioRing.size();

	sampletime ahead = w - r;	// how much writer is ahead of reader
	if (ahead > (length - numSamples)) {
		// not enough space left in ringbuffer!
		fprintf(stderr, "audio underflow\n");
		mAudioUnderflows++;
		return;
	}

	sampletime wnext = w + numSamples;
	unsigned wu = (unsigned)(w % length);

	//printf("writing %u samples to %llu\n", numSamples, wnext);

	for (unsigned c=0; c < channels; c++) {
		float * src = io.outBuffer(c);
		float * dst = &mAudioRing[0];

		unsigned wc = wu + c;
		unsigned srcIdx = 0;

		while (srcIdx < numAudioFrames) {
			dst[wc] = src[srcIdx];
			srcIdx++;
			wc += mAudioChans;
			if (wc >= length) wc -= length;
		}
	}

	// update write head position
	mAudioWriteIndex = wnext;
}

inline void * VCR::audioThreadMethod() {
	while (mActive) {
		if (mAudio) {
			// create & open soundfile:
			gam::SoundFile sf(mPath + "test.wav");
			sf.format(gam::SoundFile::WAV);
			sf.encoding(gam::SoundFile::FLOAT);
			sf.channels(mAudioChans);
			sf.frameRate(mSR);
			sf.openWrite();
			printf("started recording audio\n");

			while (mAudio) {
				writeToOpenSoundFile(sf);
				al_sleep(mSleep);
			}

			// write any remaining samples!
			writeToOpenSoundFile(sf);
			sf.close();
			printf("finished recording; with %u overflows\n", mAudioOverflows);
		} else {
			al_sleep(mSleep);
		}
	}
	return 0;
}

inline void VCR::writeToOpenSoundFile(gam::SoundFile& sf) {
	// cached, because it may be being used in a different thread
	sampletime r = mAudioReadIndex;
	sampletime w = mAudioWriteIndex;

	unsigned length = mAudioRing.size();

	// how much is there to write?
	sampletime ahead = w - r;	// how much writer is ahead of reader
	if (ahead > length) {
		// buffer is over-full!
		fprintf(stderr, "audio overflow\n");
		mAudioOverflows++;

		ahead = length;
		r = w - length;
	}

	// copy this many samples into the soundfile:
	if (ahead > 0) {
		unsigned written = 0;
		// copy in two passes, to handle ringbuffer boundary.
		unsigned ru = (unsigned)(r % length);
		unsigned wu = (unsigned)(w % length);

		if (ru > wu) {
			// read to end of ring buffer, then let the next block capture the remainder
			int frames = (length - ru)/mAudioChans;
			int copied = mAudioChans * sf.write(&mAudioRing[ru], frames);
			ru = (ru + copied) % length;
			written += copied;
		}
		if (ru < wu) {
			// let read head catch up to write head:
			int frames = (wu - ru)/mAudioChans;
			int copied = mAudioChans * sf.write(&mAudioRing[ru], frames);
			ru = (ru + copied) % length;
			written += copied;
		}

		// update read index:
		mAudioReadIndex = r + written;
	}
}

inline void VCR::image(Array& arr) {
	if (mAudio) {
		unsigned w = mImageWriteIndex;
		unsigned next = w + 1;
		if (next >= mImageRing.size()) next = 0;

		if (next != mImageReadIndex) {
			Array& dst = mImageRing[w];

			// copy format & pixels:
			dst = arr;

			mImageWriteIndex = next;
			//printf("imaged w %d r %d\n", w, mImageReadIndex);
			mImageFrame++;
		} else {
			printf("image overflow w %d r %d\n", mImageWriteIndex, mImageReadIndex);
		}
	}
}

inline void VCR::writeImages(Image& image) {
	unsigned r = mImageReadIndex;
	unsigned w = mImageWriteIndex;
	if (r == w) {
		al_sleep(mSleep * 0.1);
		return;
	}
	char path[1024];
	while (w != r) {
		sprintf(path, "%simage_%04d.jpg", mPath.c_str(), mImageCount++);
		Array& src = mImageRing[r];

		//printf("save (w %d r %d) %s %p %d %d\n", w, r, path, src.data.ptr, (int)src.width(), (int)src.height());
		image.save(std::string(path), (unsigned char *)src.data.ptr, (int)src.width(), (int)src.height(), Image::RGB);

		r = r + 1;
		if (r >= mImageRing.size()) r = 0;
		mImageReadIndex = r;
	}
}

inline void * VCR::imageThreadMethod() {
	Image image;
	while (mActive) {
		if (mAudio) {
			writeImages(image);
		} else {
			al_sleep(mSleep);
		}
	}
	// write any remaining images:
	writeImages(image);
	return 0;
}

} // al::

#endif
