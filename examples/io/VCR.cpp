/*
Allocore Example: VCR

Description:
The example demonstrates how to record images and sounds to disk.
Images are stored as sequentially numbered PNG files.
Sounds are stored as WAV files.

Author:
Graham Wakefield, Apr 2012
*/

#include <stdio.h>
#include "allocore/al_Allocore.hpp"
#include "Gamma/SoundFile.h"

using namespace al;

rnd::Random<> rng;

class VCR : public AudioCallback {
public:

	// unsigned (and unsigned long on a 32-bit compiler), at 44.1kHz, can count about 27 hours worth of samples.
	// unsigned long long however can count 13264000 years.
	typedef unsigned long long sampletime;

	VCR(int channels=2, double sampleRate=44100, int bufferSize=8192, double sleep = 0.01)
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
		mImageCount(0)
		
	{
		mPath = "/";
		mAudioRing.resize(mAudioFrames*mAudioChans);
		mImageRing.resize(32);
		
		mActive = 1;
		audioThread.start(audioThreadFunc, this);
		imageThread.start(imageThreadFunc, this);
	}
	
	~VCR() {
		mActive = 0;
		audioThread.join();
		imageThread.join();
	}
	
	// path should have a trailing slash:
	void setPath(std::string path) { mPath = path; }

	void start(AudioIO * io) {
		if (mAudio == 0) {
			mAudio = io;
			mAudio->append(*this);
		}
	}
	
	void stop() {
		if (mAudio) {
			mAudio->remove(*this);
			mAudio = 0;
		}
	}
	
	virtual void onAudioCB(AudioIOData& io) {
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
			printf("underflow\n");
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
	
	void * audioThreadMethod() {
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
	
	void writeToOpenSoundFile(gam::SoundFile& sf) {
		// cached, because it may be being used in a different thread
		sampletime r = mAudioReadIndex;
		sampletime w = mAudioWriteIndex;
		
		unsigned length = mAudioRing.size();
		
		// how much is there to write?
		sampletime ahead = w - r;	// how much writer is ahead of reader
		if (ahead > length) {
			// buffer is over-full!
			printf("overflow\n");
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
	
	void image(Array& arr) {
		if (mAudio) {
			unsigned w = mImageWriteIndex;
			unsigned next = w + 1;
			if (next >= mImageRing.size()) next = 0;
			
			if (next != mImageReadIndex) {
				Array& dst = mImageRing[w];
				
				// copy format & pixels:
				dst = arr;
				
				mImageWriteIndex = next;
				printf("imaged w %d r %d\n", w, mImageReadIndex);
			} else {
				printf("image overflow w %d r %d\n", mImageWriteIndex, mImageReadIndex);
			}
		}
	}
	
	void writeImages(Image& image) {
		unsigned r = mImageReadIndex;
		unsigned w = mImageWriteIndex;
		if (r == w) {
			al_sleep(mSleep * 0.1);
			return;
		}
		char path[1024];
		while (w != r) {
			sprintf(path, "%simage_%04d.png", mPath.c_str(), mImageCount++);
			Array& src = mImageRing[r];
			
			printf("save (w %d r %d) %s %p %d %d\n", w, r, path, src.data.ptr, (int)src.width(), (int)src.height());
			image.write<uint8_t>(std::string(path), (uint8_t *)src.data.ptr, (int)src.width(), (int)src.height(), Image::RGB);
			
			r = r + 1;
			if (r >= mImageRing.size()) r = 0;
			mImageReadIndex = r;
		}
	}

	void * imageThreadMethod() {
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

protected:
	
		
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
	unsigned mImageCount;
	
	Thread audioThread, imageThread;
	bool mActive;
};


VCR vcr;

struct App : Window {
	
	App() 
	:	audio(256, 44100, audioCB, this, 2, 2) 
	{
		image.format(3, AlloUInt8Ty, 2048, 2048);
		
				append(*new StandardWindowKeyControls);
		create(Window::Dim(100, 0, 400,300), "App");
		audio.start();
	}


	static void filler(uint8_t * c, double normx, double normy) {
		c[0] = normx * 255.;
		c[1] = normy * 255.;
		c[2] = rng.uniform() * 255.;
	}

	bool onFrame(){
		// for now just fill image with random data.
		image.fill(filler);
		
		// write:
		vcr.image(image);

		return true;
	}
	
	bool onKeyDown(const Keyboard& k){	
		printf("onKeyDown    "); 
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
		
		switch (k.key()) {
			case 's':
				vcr.start(&audio);
				break;
			case 'q':
				vcr.stop();
				break;
			default:
				break;
		}
		
		return 1; 
	}

	static void audioCB(AudioIOData& io){
		App * self = (App *)io.user();
		while(io()){
			self->phase += 0.001;
			float s0 = rng.uniformS() * sin(self->phase);
			float s1 = sin(self->phase * 100.);
			io.out(0) = s0;
			io.out(1) = s1;
		}
	}
	
	AudioIO audio;
	Array image;
	float phase;
};

int main(){

	App app;
	

	MainLoop::start();
	return 0;
}
