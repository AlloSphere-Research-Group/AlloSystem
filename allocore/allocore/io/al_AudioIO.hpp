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

#include "allocore/io/al_AudioIOData.hpp"

namespace al{

/// Audio callback type
typedef void (* audioCallback)(AudioIOData& io);

/// Audio device
///
/// @ingroup allocore
class AudioDevice: public AudioDeviceInfo {
public:

	/// Stream mode
	enum StreamMode{
		INPUT	= 1,	/**< Input stream */
		OUTPUT	= 2		/**< Output stream */
	};


	/// @param[in] deviceNum	Device enumeration number
	AudioDevice(int deviceNum = -1);

	/// @param[in] nameKeyword	Keyword to search for in device name
	/// @param[in] stream		Whether to search for input and/or output devices
	AudioDevice(const std::string& nameKeyword, StreamMode stream = StreamMode(INPUT | OUTPUT));


	/// Find device number of given device name keyword

	/// \returns device number on success or -1 if there is no match
	///
	int findDeviceNumber(const std::string& nameKeyword, StreamMode stream = StreamMode(INPUT | OUTPUT));

	virtual bool valid() const;
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
	const void * mImpl;
};

inline AudioDevice::StreamMode operator| (const AudioDevice::StreamMode& a, const AudioDevice::StreamMode& b){
	return static_cast<AudioDevice::StreamMode>(+a|+b);
}

/// Audio input/output streaming
///
/// @ingroup allocore
class AudioIO : public AudioIOData {
public:

	/// Creates AudioIO using default I/O devices.

	/// @param[in] framesPerBuf		Number of sample frames to process per callback
	/// @param[in] framesPerSec		Frame rate.  Unsupported values will use default rate of device.
	/// @param[in] callback			Audio processing callback (optional)
	/// @param[in] userData			Pointer to user data accessible within callback (optional)
	/// @param[in] outChans			Number of output channels to open
	/// @param[in] inChans			Number of input channels to open
	/// @param[in] devNum			ID of the device to open. -1 Uses default device.
	/// @param[in] backend			Audio backend to use
	/// If the number of input or output channels is greater than the device
	/// supports, virtual buffers will be created.
	AudioIO(int framesPerBuf=64, double framesPerSec=44100.0,
			void (* callback)(AudioIOData &) = 0, void * userData = 0,
			int outChans = 2, int inChans = 0,
			AudioIO::Backend backend = PORTAUDIO
			);

	virtual ~AudioIO();

	audioCallback callback;						///< User specified callback function.

	/// Add an AudioCallback handler (internal callback is always called first)
	AudioIO& append(AudioCallback& v);
	AudioIO& prepend(AudioCallback& v);
	AudioIO& insertBefore(AudioCallback& v);
	AudioIO& insertAfter(AudioCallback& v);

	/// Remove all input event handlers matching argument
	AudioIO& remove(AudioCallback& v);

	bool autoZeroOut() const { return mAutoZeroOut; }
	int channels(bool forOutput) const;
	int channelsInDevice() const;				///< Get number of channels opened on input device
	int channelsOutDevice() const;				///< Get number of channels opened on output device
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

	void channelsIn(int n);						///< Set number of input channels
	void channelsOut(int n);					///< Set number of output channels
	void channelsBus(int num);					///< Set number of bus channels
	void clipOut(bool v){ mClipOut=v; }			///< Set whether to clip output between -1 and 1
	void device(const AudioDevice& v);			///< Set input/output device (must be duplex)
	void deviceIn(const AudioDevice& v);		///< Set input device
	void deviceOut(const AudioDevice& v);		///< Set output device
	void framesPerSecond(double v);				///< Set number of frames per second
	void framesPerBuffer(int n);				///< Set number of frames per processing buffer
	void zeroNANs(bool v){ mZeroNANs=v; }		///< Set whether to zero NANs in output buffer going to DAC

	void print() const;							///< Prints info about current i/o devices to stdout.

	static const char * errorText(int errNum);		// Returns error string.

	using AudioIOData::channelsIn;
	using AudioIOData::channelsOut;
	using AudioIOData::channelsBus;
	using AudioIOData::framesPerSecond;
	using AudioIOData::framesPerBuffer;

private:
	AudioDevice mInDevice, mOutDevice;
	bool mZeroNANs;			// whether to zero NANs
	bool mClipOut;			// whether to clip output between -1 and 1
	bool mAutoZeroOut;		// whether to automatically zero output buffers each block
	std::vector<AudioCallback *> mAudioCallbacks;

	void init(int outChannels, int inChannels);			//
	void reopen();			// reopen stream (restarts stream if needed)
	void resizeBuffer(bool forOutput);
};

} // al::

#endif
