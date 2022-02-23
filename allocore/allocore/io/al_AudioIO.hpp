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

	/// Get number of frames
	int frames() const { return mFrames; }
	/// Get number of channels per frame
	int channels() const { return mChannels; }
	/// Get total number of samples
	int samples() const { return mFrames * mChannels; }
	/// Whether channels are interleaved
	bool interleaved() const { return mInterleaved; }

	/// Get sample at frame and channel
	value_type& at(int frame, int chan=0){
		return mData[mChannels*frame + chan]; // interleaved
		//return mData[frame + mFrames*chan]; // non-interleaved
	}

	/// Raw access to sample
	value_type& operator[] (int i){ return mData[i]; }

	/// Get pointer to raw data
	const value_type * data() const { return mData; }
	value_type * data(){ return mData; }

	/// Resize buffer
	AudioBlock& resize(int frames, int chans);

	/// Reference external array (frees any internal memory)
	AudioBlock& ref(float * src, int frames, int chans);

	/// Zero all samples in buffer
	AudioBlock& zero();

private:
	value_type * mData = nullptr;
	int mFrames = 0;
	int mChannels = 0;
	bool mInterleaved = true;
	bool mOwner = false;
	void clear(); // free memory
};


/// Data passed in to audio callback
class AudioIOData {
public:

	typedef AudioBlock::value_type value_type;

	/// Iterate frame counter, returning true while more frames
	bool operator()() const { return ++mFrame < mFramesPerBuffer; }
		
	/// Get current frame number
	int frame() const { return mFrame; }

	/// Set current frame number
	AudioIOData& frame(int i){ mFrame = i-1; return *this; }

	/// Get input buffer
	const AudioBlock& bufferIn() const { return mBufI; }
	AudioBlock& bufferIn(){ return mBufI; }
	/// Get output buffer
	const AudioBlock& bufferOut() const { return mBufO; }
	AudioBlock& bufferOut(){ return mBufO; }

	/// Get number of input channels
	int channelsIn() const { return mBufI.channels(); }
	/// Get number of output channels
	int channelsOut() const { return mBufO.channels(); }

	/// Get current input sample on specified channel
	const value_type& in(int chan) const { return const_cast<AudioBlock&>(mBufI).at(mFrame, chan); }
	const value_type& in(int chan){ return mBufI.at(mFrame, chan); }
	/// Get current output sample on specified channel
	value_type& out(int chan) const { return const_cast<AudioBlock&>(mBufO).at(mFrame, chan); }
	value_type& out(int chan){ return mBufO.at(mFrame, chan); }

	/// Whether channels are interleaved in buffers
	bool interleaved() const { return mBufO.interleaved(); }
	/// Whether buffers are empty
	bool empty() const { return 0==mFramesPerBuffer; }
	/// Get number of frames per buffer
	int framesPerBuffer() const { return mFramesPerBuffer; }
	/// Get number of frames per second
	double framesPerSecond() const { return mFramesPerSecond; }
	double fps() const { return mFramesPerSecond; }
	/// Get number of seconds per buffer
	double secondsPerBuffer() const { return mFramesPerBuffer/mFramesPerSecond; }

	/// Get gain of output
	float gain() const { return mGain; }
	/// Set gain of output (ramped smoothly)
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

	/// For multi-channel devices, be aware that the information reported here
	/// (namely channel count) may not be what you expect. Often this is due to
	/// the fact that devices are not accessed directly, but rather through a
	/// mixer in the OS. In Windows, you may need to configure multi-channel
	/// devices via Control Panel > Hardware and Sound > Sound > Configure.
	struct Device {
		int id = -1;				///< Unique ID
		std::string name;			///< Device name
		double frameRate = 1.;		///< Default frame rate
		short channelsIn = 0;		///< Maximum input channels
		short channelsOut = 0;		///< Maximum output channels
		bool nameMatches(const char * key) const; ///< Whether key string is in name
		bool hasInput() const { return channelsIn; } ///< Whether device has input
		bool hasOutput() const { return channelsOut; } ///< Whether device has output
		bool valid() const { return id>=0; } ///< Whether device is valid
		void print() const;
	};

	/// Audio callback
	typedef std::function<void(const AudioIOData&)> Callback;


	AudioIO();
	~AudioIO();

	int numDevices() const;					///< Get number of devices
	Device device(int i) const;				///< Get device with ID
	Device defaultDeviceIn() const;			///< Get default input device
	Device defaultDeviceOut() const;		///< Get default output device
	/// Find device using predicate
	Device findDevice(std::function<bool(Device d)> predicate) const;

	Device deviceIn() const { return mDevI; } ///< Get current input device
	Device deviceOut() const { return mDevO; } ///< Get current output device

	AudioIO& deviceIn(const Device& d);		///< Set input device
	AudioIO& deviceOut(const Device& d);	///< Set output device


	AudioIO& configure(int framesPerBuf, double framesPerSec, int chansOut, int chansIn);

	/// Configure streams

	/// This should be called after setting the devices and before opening them.
	///
	/// @param[in] framesPerBuf		Number of frames per buffer
	/// @param[in] framesPerSec		Frame rate of streams
	/// @param[in] chansOut			Number of output channels (-1 for max)
	/// @param[in] chansIn			Number of input channels (-1 for max)
	/// @param[in] cb				Audio callback function
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

	void print() const; ///< Print current configuration
	void printDevices() const; ///< Print available devices

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
