#ifndef INCLUDE_AL_AUDIO_IO_HPP
#define INCLUDE_AL_AUDIO_IO_HPP

/*
 *	Audio device and input/output streaming
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/*	This is a simple example demonstrating how to set up a callback
	and process input and output buffers.
	
	struct MyStuff{};
	
	void audioCB(AudioIOData& io){
		float * out1 = io.out(0);
		float * out2 = io.out(1);
		const float * in1 = io.in(0);
		const float * in2 = io.in(1);
		
		MyStuff& stuff = *(MyStuff *)io.user;

		for(unsigned i=0; i<io.framesPerBuffer(); ++i){

			float inSample1 = in1[i];
			float inSample2 = in2[i];

			out1[i] = -inSample1;
			out2[i] = -inSample2;
		}
	}
	
	int main(){
		MyStuff stuff;
		
		AudioIO audioIO(128, 44100, audioCB, &stuff, 2,2);
		audioIO.start();
	}
*/

#include <string>
#include <string.h>		/* memset() */
#include "portaudio.h"


namespace allo{


/// Audio data to be sent to callback
class AudioIOData {
public:
	AudioIOData(void * user);
	virtual ~AudioIOData();

	void * user;							///< User specified data
	
	float *       aux(int channel);	///< Returns an aux channel buffer
	const float * in (int channel);	///< Returns an in channel buffer
	float *       out(int channel);	///< Returns an out channel buffer
	float *		  temp();					///< Returns single channel temporary buffer
	
	int channelsIn () const;			///< Returns effective number of input channels
	int channelsOut() const;			///< Returns effective number of output channels
	int channelsAux() const;			///< Returns number of aux channels

	int channelsInDevice() const;		///< Returns number of channels opened on input device
	int channelsOutDevice() const;		///< Returns number of channels opened on output device
	int framesPerBuffer() const;		///< Returns frames/buffer of audio I/O stream
	double framesPerSecond() const;			///< Returns frames/second of audio I/O streams
	double secondsPerBuffer() const;		///< Returns seconds/buffer of audio I/O stream
	double time() const;					///< Returns current stream time in seconds
	double time(int frame) const;		///< Returns current stream time in seconds of frame
	void zeroAux();							///< Zeros all the aux buffers
	void zeroOut();							///< Zeros all the internal output buffers
	
protected:
	PaStreamParameters mInParams, mOutParams;	// Input and output stream parameters.
	PaStream * mStream;
	int mFramesPerBuffer;
	double mFramesPerSecond;
	
	float *mBufI, *mBufO, *mBufA;		// input, output, and aux buffers
	float * mBufT;						// temporary one channel buffer
	int mNumI, mNumO, mNumA;		// input, output, and aux channels
};



class AudioDevice{
public:
	AudioDevice(int deviceNum);
	AudioDevice(const std::string& nameKeyword, bool input=true, bool output=true);

	~AudioDevice();

	bool valid() const { return 0 != mImpl; }
	int id() const { return mID; }
	const char * name() const;
	int channelsInMax() const;
	int channelsOutMax() const;
	double defaultSampleRate() const;
	
	bool hasInput() const;
	bool hasOutput() const;
	
	void print() const;	/// Prints info about specific i/o device to stdout.

	static AudioDevice defaultInput();
	static AudioDevice defaultOutput();
	static int numDevices();			///< Returns number of audio i/o devices available.
	static void printAll();				///< Prints info about all available i/o devices to stdout.

private:
	void setImpl(int deviceNum);
	static void initDevices();
	int mID;
	const void * mImpl;
};


/// Audio callback type
typedef void (*audioCallback)(AudioIOData& io);


/// Audio input/output streaming.

/// This is a C++ wrapper around the PortAudio v1.9 library.
/// 
class AudioIO : public AudioIOData {
public:
	using AudioIOData::channelsIn;
	using AudioIOData::channelsOut;
	using AudioIOData::framesPerBuffer;
	using AudioIOData::framesPerSecond;

	/// Creates AudioIO using default I/O devices.
	///
	/// @param[in]	framesPerBuf	Number of sample frames to process per callback
	/// @param[in]	framesPerSec	Frame rate.  Unsupported values will use default rate of device.
	/// @param[in]	callback		Audio processing callback
	/// @param[in]	userData		Pointer to user data accessible within callback
	/// @param[in]	outChans		Number of output channels to open
	/// @param[in]	inChans			Number of input channels to open
	/// If the number of input or output channels is greater than the device
	/// supports, virtual buffers will be created.
	AudioIO(
		int framesPerBuf=64, double framesPerSec=44100.0,
		void (* callback)(AudioIOData &) = 0, void * userData = 0,
		int outChans = 2, int inChans = 0 );

	virtual ~AudioIO();
		
	//static audioCallback callback;	///< User specified callback function.
	audioCallback callback;	///< User specified callback function.
	
	void operator()();				///< Calls callback manually

	bool open();					///< Opens audio device.
	bool close();					///< Closes audio device. Will stop active IO.
	bool start();					///< Starts the audio IO.  Will open audio device if necessary.
	bool stop();					///< Stops the audio IO.

	/// Sets number of effective channels on input or output device depending on 'forOutput' flag.
	
	/// An effective channel is either a real device channel or virtual channel 
	/// depending on how many channels the device supports. Passing in -1 for
	/// the number of channels opens all available channels.
	void channels(int num, bool forOutput);
	void channelsIn(int n){ channels(n,false); }	///< Set number of input channels
	void channelsOut(int n){ channels(n,true); }	///< Set number of output channels
	void channelsAux(int num);				///< Set number of auxiliary channels
	void clipOut(bool v){ mClipOut=v; }			///< Set whether to clip output between -1 and 1
	
	void deviceIn(const AudioDevice& v);		///< Set input device
	void deviceOut(const AudioDevice& v);		///< Set output device
	
	void framesPerSecond(double v);				///< Set number of frames per second
	void framesPerBuffer(int n);			///< Set number of frames per processing buffer
	void zeroNANs(bool v){ mZeroNANs=v; }		///< Set whether to zero NANs in output buffer

	int channels(bool forOutput) const;
	bool clipOut() const { return mClipOut; }	///< Returns clipOut setting
	double cpu() const;							///< Returns current CPU usage of audio thread
	bool supportsFPS(double fps);				///< Return true if fps supported, otherwise false
	bool zeroNANs() const;						///< Returns zeroNANs setting

	void print();								///< Prints info about current i/o devices to stdout.
	void printError();							///< Prints info about current error status to stdout.

	static const char * errorText(int errNum);		// Returns error string.
	
private:
	PaError mErrNum;							// Most recent error number.
	//PaDeviceIndex mInDevice, mOutDevice;		// Input and output device ids.
	AudioDevice mInDevice, mOutDevice;

	bool mIsOpen;			// An audio device is open
	bool mIsRunning;		// An audio stream is running
	bool mInResizeDeferred, mOutResizeDeferred;
	bool mZeroNANs;			// whether to zero NANs
	bool mClipOut;			// whether to clip output between -1 and 1

	void init();		// Initializes PortAudio and member variables.
	
	static int paCallback(	const void *input,
							void *output,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void *userData );

	PaDeviceIndex defaultInDevice();
	PaDeviceIndex defaultOutDevice();
	
	bool error() const;
	void inDevice(PaDeviceIndex index);		// directly set input device
	void outDevice(PaDeviceIndex index);	// directly set output device
	void setInDeviceChans(int num);			// directly set # device input channels
	void setOutDeviceChans(int num);			// directly set # device output channels
	//void virtualChans(int num, bool forOutput);
	
	void deferBufferResize(bool forOutput);
	void resizeBuffer(bool forOutput);

	void reopen();		// reopen stream (restarts stream if needed)

};

} // allo::

#endif
