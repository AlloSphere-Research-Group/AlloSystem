#ifndef INCLUDE_AL_AUDIO_IO_HPP
#define INCLUDE_AL_AUDIO_IO_HPP

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
	An interface to low-level audio device streams

	File author(s):
	Lance Putnam, 2022
*/

#include <functional>
#include <string>
#include <vector>

namespace al {

/// Buffer of multi-channel audio samples
class AudioBlock {
public:
	typedef float value_type;

	~AudioBlock();

	int frames() const { return mFrames; }
	int channels() const { return mChannels; }
	int samples() const { return mFrames * mChannels; }
	int interleaved() const { return mInterleaved; }

	value_type& at(int frame, int chan=0){
		return mData[mChannels*frame + chan]; // interleaved
		//return mData[frame + mFrames*chan]; // non-interleaved
	}

	value_type& operator[] (int i){ return mData[i]; }

	const value_type * data() const { return mData; }
	value_type * data(){ return mData; }

	AudioBlock& resize(int frames, int chans);

	AudioBlock& ref(float * src, int frames, int chans);

	AudioBlock& zero();

private:
	value_type * mData = nullptr;
	int mFrames = 0;
	int mChannels = 0;
	bool mInterleaved = true;
	bool mOwner = false;
	void clear();
};


/// Data passed in to audio callback
class AudioIOData {
public:

	typedef AudioBlock::value_type value_type;

	/// Iterate frame counter, returning true while more frames
	bool operator()() const { return ++mFrame < mFramesPerBuffer; }
		
	/// Get current frame number
	int frame() const { return mFrame; }

	AudioIOData& frame(int i){ mFrame = i-1; return *this; }

	const AudioBlock& bufferIn() const { return mBufI; }
	AudioBlock& bufferIn(){ return mBufI; }
	const AudioBlock& bufferOut() const { return mBufO; }
	AudioBlock& bufferOut(){ return mBufO; }

	const value_type& in(int chan) const { return const_cast<AudioBlock&>(mBufI).at(mFrame, chan); }
	const value_type& in(int chan){ return mBufI.at(mFrame, chan); }
	value_type& out(int chan) const { return const_cast<AudioBlock&>(mBufO).at(mFrame, chan); }
	value_type& out(int chan){ return mBufO.at(mFrame, chan); }

	bool interleaved() const { return mBufO.interleaved(); }
	bool empty() const { return 0==mFramesPerBuffer; }
	int framesPerBuffer() const { return mFramesPerBuffer; }
	double framesPerSecond() const { return mFramesPerSecond; }
	double fps() const { return mFramesPerSecond; }
	double secondsPerBuffer() const { return mFramesPerBuffer/mFramesPerSecond; }

	float gain() const { return mGain; }
	AudioIOData& gain(float v){ mGain=v; return *this; }
	AudioIOData& gainMul(float v){ mGain*=v; return *this; }

protected:
	mutable int mFrame = 0;
	int mFramesPerBuffer = 0;
	double mFramesPerSecond = 0.;
	AudioBlock mBufI, mBufO;
	float mGain = 1.f, mGainPrev = 1.f;
};


/// Real-time audio input/output stream
class AudioIO : public AudioIOData {
public:

	/// Audio device
	struct Device {
		int id = -1;				///< Unique ID
		std::string name;			///< Device name
		double defSampleRate = 1.;	///< Default sample rate
		short chanIMax = 0;			///< Maximum input channels
		short chanOMax = 0;			///< Maximum output channels
		bool hasInput() const { return chanIMax; }
		bool hasOutput() const { return chanOMax; }
		bool valid() const { return id>=0; }
		void print() const;
	};

	/// Audio callback
	typedef std::function<void(const AudioIOData&)> Callback;


	AudioIO();
	~AudioIO();


	AudioIO& configure(int framesPerBuf, double framesPerSec, int chansOut, int chansIn);

	AudioIO& configure(int framesPerBuf, double framesPerSec, int chansOut, int chansIn, const Callback& cb){
		return configure(framesPerBuf, framesPerSec, chansOut, chansIn).callback(cb);
	}

	/// Set audio callback
	AudioIO& callback(const Callback& cb){ mCallback = cb; return *this; }

	AudioIO& append(const Callback& cb);
	AudioIO& prepend(const Callback& cb);
	AudioIO& remove(const Callback& cb);

	bool open();			///< Prepare I/O for streaming
	bool start();			///< Start streaming (and ensure I/O open)
	bool stop();			///< Stop streaming
	bool close();			///< Close I/O (and stop streaming )

	AudioIO& processAudio();

	/// Set whether to zero output before calling callback(s)
	AudioIO& zeroOut(bool v){ mZeroOut=v; return *this; }

	/// Set whether to clip output in [-1,1]
	AudioIO& clipOut(bool v){ mClipOut=v; return *this; }

	/// Set whether to zero any NaNs in the output
	AudioIO& zeroNANs(bool v){ mZeroNANs=v; return *this; }

	
	int numDevices() const;					///< Get number of devices
	Device device(int i) const;				///< Get device with ID
	Device defaultDeviceIn() const;			///< Get default input device
	Device defaultDeviceOut() const;		///< Get default output device

	Device deviceIn() const { return mDevI; } ///< Get current input device
	Device deviceOut() const { return mDevO; } ///< Get current output device

	AudioIO& deviceIn(const Device& d);		///< Set input device
	AudioIO& deviceOut(const Device& d);	///< Set output device

	double cpu() const;						///< Get CPU usage

private:
	struct Impl; Impl * mImpl;
	Device mDevI, mDevO;
	Callback mCallback = nullptr;
	std::vector<Callback> mCallbacks;
	bool mIsOpen = false;		// An audio device is open
	bool mIsRunning = false;	// An audio stream is running
	bool mZeroOut = true;		// whether to zero output buffer before calling callbacks
	bool mZeroNANs = true;		// whether to zero NANs
	bool mClipOut = true;		// whether to clip output between -1 and 1
};

}  // al::

#endif
