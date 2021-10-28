#ifndef INCLUDE_AL_AUDIODATA_IO_HPP
#define INCLUDE_AL_AUDIODATA_IO_HPP

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
	Lance Putnam, 2010, putnam.lance@gmail.com
	Andres Cabrera 2017 mantaraya36@gmail.com
*/

#include <cassert>
#include <cstdio>
#include <string>

namespace al {

/// Audio device information
///
/// @ingroup allocore
class AudioDeviceInfo {
public:
	/// Stream mode
	enum StreamMode {
		INPUT = 1, /**< Input stream */
		OUTPUT = 2 /**< Output stream */
	};

	/// @param[in] deviceNum	Device enumeration number
	AudioDeviceInfo(int deviceNum);

	//	/// @param[in] nameKeyword	Keyword to search for in device name
	//	/// @param[in] stream		Whether to search for input and/or
	//output
	// devices
	//	AudioDeviceInfo(const std::string& nameKeyword, StreamMode stream =
	// StreamMode(INPUT | OUTPUT)) : mID(-1) {}

	virtual ~AudioDeviceInfo() {}

	virtual bool valid() const;					///< Returns whether device is valid
	virtual int id() const;						///< Get device unique ID
	virtual const std::string& name() const;	///< Get device name
	virtual int channelsInMax()	const;			///< Get maximum number of input channels supported
	virtual int channelsOutMax() const;			///< Get maximum number of output channels supported
	virtual double defaultSampleRate() const;	///< Get default sample rate

	virtual void setID(int iD);					///< Sets unique ID
	virtual void setName(const std::string& name); ///< Sets device name
	virtual void setChannelsInMax(int num);		///< Sets maximum number of Input channels supported
	virtual void setChannelsOutMax(int num);	///< Sets maximum number of Output channels supported
	virtual void setDefaultSampleRate(double rate); ///< Sets default sample rate

	virtual bool hasInput() const = 0;			///< Returns whether device has input
	virtual bool hasOutput() const = 0;			///< Returns whether device has output

	virtual void print() const = 0;				///< Prints info about specific i/o device to stdout

protected:
	int mID{-1};
	std::string mName;
	int mChannelsInMax{0};
	int mChannelsOutMax{0};
	double mDefaultSampleRate{0};
	bool mValid{false};
};

inline AudioDeviceInfo::StreamMode operator|(
        const AudioDeviceInfo::StreamMode& a,
        const AudioDeviceInfo::StreamMode& b) {
	return static_cast<AudioDeviceInfo::StreamMode>(+a | +b);
}

/// Audio data to be sent to callback
/// Audio buffers are guaranteed to be stored in a contiguous non-interleaved
/// format, i.e., frames are tightly packed per channel.
///
/// @ingroup allocore
class AudioIOData {
public:
	/// Constructor
	AudioIOData(void* user);

	virtual ~AudioIOData();

	typedef enum { PORTAUDIO, RTAUDIO, DUMMY } Backend;

	/// Iterate frame counter, returning true while more frames
	bool operator()() const { return (++mFrame) < framesPerBuffer(); }

	/// Get current frame number
	int frame() const { return mFrame; }

	/// Get bus sample at current frame iteration on specified channel
	float& bus(int chan) const { return bus(chan, frame()); }

	/// Get bus sample at specified channel and frame
	float& bus(int chan, int frame) const;

	/// Get non-interleaved bus buffer on specified channel
	float* busBuffer(int chan = 0) const { return &bus(chan, 0); }

	/// Get input sample at current frame iteration on specified channel
	const float& in(int chan) const { return in(chan, frame()); }

	/// Get input sample at specified channel and frame
	const float& in(int chan, int frame) const;

	/// Get non-interleaved input buffer on specified channel
	const float* inBuffer(int chan = 0) const { return &in(chan, 0); }

	/// Get output sample at current frame iteration on specified channel
	float& out(int chan) const { return out(chan, frame()); }

	/// Get output sample at specified channel and frame
	float& out(int chan, int frame) const;

	/// Get non-interleaved output buffer on specified channel
	float* outBuffer(int chan = 0) const { return &out(chan, 0); }

	/// Add value to current output sample on specified channel
	void sum(float v, int chan) const { out(chan) += v; }

	/// Add value to current output sample on specified channels
	void sum(float v, int ch1, int ch2) const {
		sum(v, ch1);
		sum(v, ch2);
	}

	/// Get sample from temporary buffer at specified frame
	float& temp(int frame) const;

	/// Get non-interleaved temporary buffer on specified channel
	float* tempBuffer() const { return &temp(0); }

	void* user() const { return mUser; }  ///< Get pointer to user data

	template <class UserDataType>
	UserDataType& user() const {
		return *(static_cast<UserDataType*>(mUser));
	}

	int channelsIn() const;          ///< Get effective number of input channels
	int channelsOut() const;         ///< Get effective number of output channels
	int channelsBus() const;         ///< Get number of allocated bus channels
	int framesPerBuffer() const;     ///< Get frames/buffer of audio I/O stream
	double framesPerSecond() const;  ///< Get frames/second of audio I/O streams
	double fps() const { return framesPerSecond(); }
	double secondsPerBuffer() const;  ///< Get seconds/buffer of audio I/O stream

	void user(void* v) { mUser = v; }      ///< Set user data
	void frame(int v) { mFrame = v - 1; }  ///< Set frame count for next iteration
	void zeroBus();                        ///< Zeros all the bus buffers
	void zeroOut();  ///< Zeros all the internal output buffers

	AudioIOData& gain(float v) {
		mGain = v;
		return *this;
	}
	bool usingGain() const { return mGain != 1.f || mGainPrev != 1.f; }

protected:
	void* mUser;  // User specified data
	mutable int mFrame;
	int mFramesPerBuffer;
	double mFramesPerSecond;
	float *mBufI, *mBufO, *mBufB;  // input, output, and aux buffers
	float* mBufT;                  // temporary one channel buffer
	int mNumI, mNumO, mNumB;       // input, output, and aux channels
private:
	void operator=(const AudioIOData&);  // Disallow copy
public:
	float mGain, mGainPrev;
};



//==============================================================================
inline float& AudioIOData::bus(int c, int f) const {
	assert(c < mNumB);
	assert(f < framesPerBuffer());
	return mBufB[c * framesPerBuffer() + f];
}

inline const float& AudioIOData::in(int c, int f) const {
	assert(c < mNumI);
	assert(f < framesPerBuffer());
	return mBufI[c * framesPerBuffer() + f];
}

inline float& AudioIOData::out(int c, int f) const {
	assert(c < mNumO);
	assert(f < framesPerBuffer());
	return mBufO[c * framesPerBuffer() + f];
}
inline float& AudioIOData::temp(int f) const { return mBufT[f]; }


/// Utility function to deinterleave samples
template <class T>
void deinterleave(T * dst, const T * src, int numFrames, int numChannels){
	int numSamples = numFrames * numChannels;
	for(int c=0; c < numChannels; c++){
		for(int i=c; i < numSamples; i+=numChannels){
			*dst++ = src[i];
		}
	}
}

/// Utility function to interleave samples
template <class T>
void interleave(T * dst, const T * src, int numFrames, int numChannels){
	int numSamples = numFrames * numChannels;
	for(int c=0; c < numChannels; c++){
		for(int i=c; i < numSamples; i+=numChannels){
			dst[i] = *src++;
		}
	}
}

template <class T>
void deleteBuf(T *& buf){ delete[] buf; buf=0; }

template <class T>
int resizeBuf(T *& buf, int n){
	deleteBuf(buf);
	buf = new T[n];
	return n;
}

}  // al::

#endif
