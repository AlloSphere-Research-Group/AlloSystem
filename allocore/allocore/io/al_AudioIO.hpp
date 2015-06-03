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
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


#include <string>
#include <vector>

namespace al{

class AudioIOData;


/// Audio callback type
typedef void (* audioCallback)(AudioIOData& io);

/// Audio device abstraction
class AudioDeviceInfo{
public:

	/// Stream mode
	enum StreamMode{
		INPUT	= 1,	/**< Input stream */
		OUTPUT	= 2		/**< Output stream */
	};

	/// @param[in] deviceNum	Device enumeration number
	AudioDeviceInfo(int deviceNum) {}
	
	/// @param[in] nameKeyword	Keyword to search for in device name
	/// @param[in] stream		Whether to search for input and/or output devices
	AudioDeviceInfo(const std::string& nameKeyword, StreamMode stream = StreamMode(INPUT | OUTPUT)) {}

//	~AudioDeviceInfo() = 0;

	virtual bool valid() const = 0;	///< Returns whether device is valid
	virtual int id() const = 0;			///< Get device unique ID
	virtual const char * name() const = 0;				///< Get device name
	virtual int channelsInMax() const = 0;				///< Get maximum number of input channels supported
	virtual int channelsOutMax() const = 0;				///< Get maximum number of output channels supported
	virtual double defaultSampleRate() const = 0;		///< Get default sample rate
	
	virtual bool hasInput() const = 0;					///< Returns whether device has input
	virtual bool hasOutput() const = 0;					///< Returns whether device has output
	
	virtual void print() const = 0;						///< Prints info about specific i/o device to stdout

protected:

};

inline AudioDeviceInfo::StreamMode operator| (const AudioDeviceInfo::StreamMode& a, const AudioDeviceInfo::StreamMode& b){
	return static_cast<AudioDeviceInfo::StreamMode>(+a|+b);
}

class AudioDevice: public AudioDeviceInfo {
public:

	/// @param[in] deviceNum	Device enumeration number
	AudioDevice(int deviceNum);

	/// @param[in] nameKeyword	Keyword to search for in device name
	/// @param[in] stream		Whether to search for input and/or output devices
	AudioDevice(const std::string& nameKeyword, StreamMode stream = StreamMode(INPUT | OUTPUT));

//	~AudioDevice();

	virtual bool valid() const { return 0!=mImpl; }	///< Returns whether device is valid
	virtual int id() const { return mID; }			///< Get device unique ID
	virtual const char * name() const;				///< Get device name
	virtual int channelsInMax() const;				///< Get maximum number of input channels supported
	virtual int channelsOutMax() const;				///< Get maximum number of output channels supported
	virtual double defaultSampleRate() const;		///< Get default sample rate

	virtual bool hasInput() const;					///< Returns whether device has input
	virtual bool hasOutput() const;					///< Returns whether device has output

	virtual void print() const;						///< Prints info about specific i/o device to stdout

	// TODO: these should be removed from here and moved to AudioBackend
	static AudioDevice defaultInput();		///< Get system's default input device
	static AudioDevice defaultOutput();		///< Get system's default output device
	static int numDevices();				///< Returns number of audio i/o devices available
	static void printAll();					///< Prints info about all available i/o devices to stdout

protected:
	void setImpl(int deviceNum);
	static void initDevices();
	int mID;
	const void * mImpl;
};

class AudioBackend{
public:
	AudioBackend(): mIsOpen(false), mIsRunning(false){}

	virtual bool isOpen() const = 0;
	virtual bool isRunning() const = 0;
	virtual bool error() const = 0;

	virtual void printError(const char * text = "") const = 0;
	virtual void printInfo() const = 0;

	virtual bool supportsFPS(double fps) const = 0;

	virtual void inDevice(int index) = 0;
	virtual void outDevice(int index) = 0;

	virtual int channels(int num, bool forOutput) = 0;

	virtual int inDeviceChans() = 0;
	virtual int outDeviceChans() = 0;
	virtual void setInDeviceChans(int num) = 0;
	virtual void setOutDeviceChans(int num) = 0;

	virtual double time() = 0;

	virtual bool open(int framesPerSecond, int framesPerBuffer, void *userdata) = 0;
	virtual bool close() = 0;

	virtual bool start(int framesPerSecond, int framesPerBuffer, void *userdata) = 0;
	virtual bool stop() = 0;
	virtual double cpu() = 0;

protected:
	bool mIsOpen;						// An audio device is open
	bool mIsRunning;					// An audio stream is running
};

/// Audio data to be sent to callback

/// Audio buffers are guaranteed to be stored in a contiguous non-interleaved 
/// format, i.e., frames are tightly packed per channel.
class AudioIOData {
public:
	/// Constructor
	AudioIOData(void * user, int backend = PortAudio);

	virtual ~AudioIOData();

	enum Backend{
		PortAudio,
		Dummy
	};

	/// Iterate frame counter, returning true while more frames
	bool operator()() const { return (++mFrame)<framesPerBuffer(); }
		
	/// Get current frame number
	int frame() const { return mFrame; }

	/// Get bus sample at current frame iteration on specified channel
	float& bus(int chan) const { return bus(chan, frame()); }

	/// Get bus sample at specified channel and frame
	float& bus(int chan, int frame) const;

	/// Get non-interleaved bus buffer on specified channel
	float * busBuffer(int chan=0) const { return &bus(chan,0); }

	/// Get input sample at current frame iteration on specified channel
	const float& in(int chan) const { return in (chan, frame()); }

	/// Get input sample at specified channel and frame
	const float& in (int chan, int frame) const;

	/// Get non-interleaved input buffer on specified channel
	const float * inBuffer(int chan=0) const { return &in(chan,0); }

	/// Get output sample at current frame iteration on specified channel
	float& out(int chan) const { return out(chan, frame()); }

	/// Get output sample at specified channel and frame
	float& out(int chan, int frame) const;

	/// Get non-interleaved output buffer on specified channel
	float * outBuffer(int chan=0) const { return &out(chan,0); }
	
	/// Add value to current output sample on specified channel
	void sum(float v, int chan) const { out(chan)+=v; }
	
	/// Add value to current output sample on specified channels
	void sum(float v, int ch1, int ch2) const { sum(v, ch1); sum(v,ch2); }
	
	/// Get sample from temporary buffer at specified frame
	float& temp(int frame) const;

	/// Get non-interleaved temporary buffer on specified channel
	float * tempBuffer() const { return &temp(0); }

	void * user() const{ return mUser; } ///< Get pointer to user data

	template<class UserDataType>
	UserDataType& user() const { return *(static_cast<UserDataType *>(mUser)); }

	int channelsIn () const;			///< Get effective number of input channels
	int channelsOut() const;			///< Get effective number of output channels
	int channelsBus() const;			///< Get number of allocated bus channels
	int channelsInDevice() const;		///< Get number of channels opened on input device
	int channelsOutDevice() const;		///< Get number of channels opened on output device
	int framesPerBuffer() const;		///< Get frames/buffer of audio I/O stream
	double framesPerSecond() const;		///< Get frames/second of audio I/O streams
	double fps() const { return framesPerSecond(); }
	double secondsPerBuffer() const;	///< Get seconds/buffer of audio I/O stream
	double time() const;				///< Get current stream time in seconds
	double time(int frame) const;		///< Get current stream time in seconds of frame

	void user(void * v){ mUser=v; }		///< Set user data
	void frame(int v){ mFrame=v-1; }	///< Set frame count for next iteration
	void zeroBus();						///< Zeros all the bus buffers
	void zeroOut();						///< Zeros all the internal output buffers

	AudioIOData& gain(float v){ mGain=v; return *this; }
	bool usingGain() const { return mGain != 1.f || mGainPrev != 1.f; }

protected:
	AudioBackend * mImpl;
	void * mUser;					// User specified data
	mutable int mFrame;
	int mFramesPerBuffer;
	double mFramesPerSecond;
	float *mBufI, *mBufO, *mBufB;	// input, output, and aux buffers
	float * mBufT;					// temporary one channel buffer
	int mNumI, mNumO, mNumB;		// input, output, and aux channels
public:
	float mGain, mGainPrev;
};



/// Interface for objects which can be registered with an audio IO stream
class AudioCallback {
public:
	virtual ~AudioCallback() {}
	virtual void onAudioCB(AudioIOData& io) = 0;	///< Callback
};


/// Audio input/output streaming
class AudioIO : public AudioIOData {
public:

	/// Creates AudioIO using default I/O devices.

	/// @param[in] framesPerBuf		Number of sample frames to process per callback
	/// @param[in] framesPerSec		Frame rate.  Unsupported values will use default rate of device.
	/// @param[in] callback			Audio processing callback (optional)
	/// @param[in] userData			Pointer to user data accessible within callback (optional)
	/// @param[in] outChans			Number of output channels to open
	/// @param[in] inChans			Number of input channels to open
	/// If the number of input or output channels is greater than the device
	/// supports, virtual buffers will be created.
	AudioIO(int framesPerBuf=64, double framesPerSec=44100.0,
			void (* callback)(AudioIOData &) = 0, void * userData = 0,
			int outChans = 2, int inChans = 0,
			int backend = PortAudio
			);

	virtual ~AudioIO();

	using AudioIOData::channelsIn;
	using AudioIOData::channelsOut;
	using AudioIOData::framesPerBuffer;
	using AudioIOData::framesPerSecond;

	audioCallback callback;						///< User specified callback function.
	
	/// Add an AudioCallback handler (internal callback is always called first)
	AudioIO& append(AudioCallback& v);		
	AudioIO& prepend(AudioCallback& v);

	/// Remove all input event handlers matching argument
	AudioIO& remove(AudioCallback& v);

	bool autoZeroOut() const { return mAutoZeroOut; }
	int channels(bool forOutput) const;
	bool clipOut() const { return mClipOut; }	///< Returns clipOut setting
	double cpu() const;							///< Returns current CPU usage of audio thread
	bool supportsFPS(double fps) const;			///< Return true if fps supported, otherwise false
	bool zeroNANs() const;						///< Returns whether to zero NANs in output buffer going to DAC
	
	void processAudio();						///< Call callback manually
	bool open();								///< Opens audio device.
	bool close();								///< Closes audio device. Will stop active IO.
	bool start();								///< Starts the audio IO.  Will open audio device if necessary.
	bool stop();								///< Stops the audio IO.

	void autoZeroOut(bool v){ mAutoZeroOut=v; }

	/// Sets number of effective channels on input or output device depending on 'forOutput' flag.
	
	/// An effective channel is either a real device channel or virtual channel 
	/// depending on how many channels the device supports. Passing in -1 for
	/// the number of channels opens all available channels.
	void channels(int num, bool forOutput);

	void channelsIn(int n){channels(n,false);}	///< Set number of input channels
	void channelsOut(int n){channels(n,true);}	///< Set number of output channels
	void channelsBus(int num);					///< Set number of bus channels
	void clipOut(bool v){ mClipOut=v; }			///< Set whether to clip output between -1 and 1
	void device(const AudioDevice& v);			///< Set input/output device (must be duplex)	
	void deviceIn(const AudioDevice& v);		///< Set input device
	void deviceOut(const AudioDevice& v);		///< Set output device
	void framesPerSecond(double v);				///< Set number of frames per second
	void framesPerBuffer(int n);				///< Set number of frames per processing buffer
	void zeroNANs(bool v){ mZeroNANs=v; }		///< Set whether to zero NANs in output buffer going to DAC

	void print();								///< Prints info about current i/o devices to stdout.

	static const char * errorText(int errNum);		// Returns error string.

private:
	AudioDevice mInDevice, mOutDevice;
	bool mZeroNANs;			// whether to zero NANs
	bool mClipOut;			// whether to clip output between -1 and 1
	bool mAutoZeroOut;		// whether to automatically zero output buffers each block
	std::vector<AudioCallback *> mAudioCallbacks;

	void init();			//
	void reopen();			// reopen stream (restarts stream if needed)
	void resizeBuffer(bool forOutput);
};





//==============================================================================
inline float&       AudioIOData::bus(int c, int f) const { return mBufB[c*framesPerBuffer() + f]; }
inline const float& AudioIOData::in (int c, int f) const { return mBufI[c*framesPerBuffer() + f]; }
inline float&       AudioIOData::out(int c, int f) const { return mBufO[c*framesPerBuffer() + f]; }
inline float&       AudioIOData::temp(int f) const { return mBufT[f]; }

} // al::

#endif
