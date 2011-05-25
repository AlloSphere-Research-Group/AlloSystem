/*
 *  seq.cpp
 *  allo
 *
 *  Created by Graham Wakefield on 5/18/11.

A short example of real-time / non-real time rendering.

All events are time-stamped and executed in order.

RT mode:
	audio driver provides callbacks to synthesize audio.
	
NRT mode:
	audio driver is triggered manually, results are sent to a separate recorder to write to file.





TODO: 
	add flags to gam::Recorder to flag overflows (when > size samples are written but not read.)
	Or move Recorder implementation to a class here.
 */


#include "allocore/al_Allocore.hpp"
#include "Gamma/Recorder.h"
#include "Gamma/SoundFile.h"

using namespace al;

// unsigned (and unsigned long on a 32-bit compiler), at 44.1kHz, can count about 27 hours worth of samples.
// unsigned long long however can count 13264000 years.
typedef unsigned long long sampletime;

/*
	Attaches to an AudioIO callback.
	When start() has been called, 
		begins capturing the out buffers into an internal ringbuffer
		and a background thread process writes this ringbuffer to disk
	Calling stop() or destroying the AudioCapture object will finish writing the file.
*/
// currently fixed to float32 output format
class AudioCapture : public AudioCallback {
public:

	AudioCapture(AudioIO * host, std::string fileName, int channels=2, double sampleRate=44100, int bufferSize=8192, double sleep = 0.01) 
	:	mAudioIO(host),
		mFileName(fileName),
		mChans(channels),
		mSR(sampleRate),
		mFrames(bufferSize),
		mReadIndex(0), 
		mWriteIndex(0),
		mSleep(sleep),
		mOverflows(0),
		mUnderflows(0),
		mRecording(false)
	{
		mRing.resize(mFrames*mChans);
		host->append(this);
	}
	
	virtual ~AudioCapture() {
		mAudioIO->remove(this);
		stop();
	}
	
	void writeToOpenSoundFile(gam::SoundFile& sf) {
		// cached, because it may be being used in a different thread
		sampletime r = mReadIndex;
		sampletime w = mWriteIndex;
		
		unsigned length = mRing.size();
		
		// how much is there to write?
		sampletime ahead = w - r;	// how much writer is ahead of reader
		if (ahead > length) {
			// buffer is over-full!
			printf("overflow\n");
			mOverflows++;
			
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
                int frames = (length - ru)/mChans;
                int copied = mChans * sf.write(&mRing[ru], frames);
                ru = (ru + copied) % length;
				written += copied;
            }				
            if (ru < wu) {
                // let read head catch up to write head:
                int frames = (wu - ru)/mChans;
                int copied = mChans * sf.write(&mRing[ru], frames);
                ru = (ru + copied) % length;
				written += copied;
            }
			
			// update read index:
			mReadIndex = r + written;
		}
	}
	
	virtual void onAudioCB(AudioIOData& io) {
		if (recording) {
			// cached, because it may be being used in a different thread
			sampletime r = mReadIndex;
			sampletime w = mWriteIndex;
			
			unsigned channels = io.channelsOut();
			if (channels > mChans) channels = mChans;
			
			unsigned numFrames = io.framesPerBuffer();
			unsigned numSamples = numFrames * channels;
			unsigned length = mRing.size();
			
			sampletime ahead = w - r;	// how much writer is ahead of reader
			if (ahead > (length - numSamples)) {
				// not enough space left in ringbuffer!
				printf("underflow\n");
				mUnderflows++;
				return;
			}
			
			sampletime wnext = w + numSamples;
			unsigned wu = (unsigned)(w % length);
			
			//printf("writing %u samples to %llu\n", numSamples, wnext);
			
			for (unsigned c=0; c < channels; c++) {
				float * src = io.outBuffer(c);
				float * dst = &mRing[0];
				
				unsigned wc = wu + c;
				unsigned srcIdx = 0;
				
				while (srcIdx < numFrames) {
					dst[wc] = src[srcIdx];
					srcIdx++;
					wc += mChans;
					if (wc >= length) wc -= length;
				}
			}
		
			// update write head position
			mWriteIndex = wnext;
		}
	}
		
	void start() {
		if (!mRecording) {
			mRecording = true;
			mThread.start(audioRecordingThreadFunc, this);
		}
	}
	
	// waits for thread to stop.
	void stop() {
		if (mRecording) {
			mRecording = false;
			mThread.join();
		}
	}
	
	static void * audioRecordingThreadFunc(void * userData) {
		AudioCapture * self = (AudioCapture *)userData;
		if (self) {
			return (void *)self->recordingThreadFunc();
		}
		return NULL;
	}
	
	int recordingThreadFunc() {
		printf("start recording to %s\n", mFileName.c_str());
		// create & open soundfile:
		gam::SoundFile sf(mFileName);
		sf.format(gam::SoundFile::WAV);
		sf.encoding(gam::SoundFile::FLOAT);
		sf.channels(mChans);
		sf.frameRate(mSR);
		sf.openWrite();
		
		printf("started recording\n");
		while (mRecording) {		
			writeToOpenSoundFile(sf);
			al_sleep(mSleep);
		}
        
        // write any remaining samples!
        writeToOpenSoundFile(sf);
		sf.close();
		printf("finished recording; with %u overflows\n", mOverflows);
		return 0;
	}
	
	AudioIO * mAudioIO;
	std::string mFileName;
	unsigned mChans;
	double mSR;
	unsigned mFrames;
	sampletime mReadIndex, mWriteIndex;
	double mSleep;
	unsigned mOverflows, mUnderflows;
	bool mRecording;
	Thread mThread;
	
	std::vector<float> mRing;
};

void audioCB(AudioIOData& io){
	while(io()){
		float s0 = io.in(0);
		float s1 = io.in(1);
		
		io.out(0) = s0;
		io.out(1) = s1;
	}
}

int main (int argc, char * argv[]){
	
	SearchPaths paths(argc, argv);
	
	printf("outpath %s\n", paths.appPath().c_str());
	
	std::string audioFileName = paths.appPath() + "recording.wav";
	
	AudioIO audioIO(256, 44100, audioCB, NULL, 2, 2);
	AudioCapture capture(&audioIO, audioFileName, 2, 44100);
	
	
	capture.start();
	audioIO.start();
	
	printf("\nPress 'enter' to stop recording...\n"); getchar();
	
	return 0;
}
