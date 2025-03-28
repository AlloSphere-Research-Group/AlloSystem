#include <cmath> // sin
#include <cstdio>
#include <cstring> // memcpy, memset
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp" // AL_WARN
using namespace al;

// Useful for debugging...
struct SineOsc{
	float phase = 0;
	float freq = 0;
	float operator()(){
		auto s = std::sin(phase * 6.283185307);
		phase += freq;
		if(phase>1.) phase--;
		return s;
	}
};


#if defined(AL_AUDIO_DUMMY)

struct AudioIO::Impl{
	Impl(AudioIO& aio){}
	bool open(){ return false; }
	bool start(){ return false; }
	bool close(){ return false; }
	bool stop(){ return false; }
};

int AudioIO::numDevices() const { return 0; }
AudioIO::Device AudioIO::defaultDeviceIn() const { return {}; }
AudioIO::Device AudioIO::defaultDeviceOut() const { return {}; }
AudioIO::Device AudioIO::device(int i) const { return {}; }
//---- End Dummy backend


#elif defined(AL_AUDIO_PORTAUDIO)

#include <portaudio.h>

static int paCallback(
	const void * input,
	void * output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo * timeInfo,
	PaStreamCallbackFlags statusFlags,
	void * userData
){
	auto& io = *static_cast<AudioIO *>(userData);

	// Copy PA input buffer to AudioIO
	std::memcpy(io.bufferIn().data(), input, io.bufferIn().samples()*sizeof(float));

	io.processAudio();

	// Copy AudioIO output buffer to PA
	std::memcpy(output, io.bufferOut().data(), io.bufferOut().samples()*sizeof(float));

	return 0;
}

struct AudioIO::Impl{

	Impl(AudioIO& aio)
	:	mAudioIO(aio), mInited(paNoError == Pa_Initialize())
	{}

	~Impl(){ if(mInited) Pa_Terminate(); }

	bool open(){
		PaStreamParameters pi, po;
		{
			const auto& dev = mAudioIO.mDevI;
			auto& P = pi;
			P.device = dev.id >= 0 ? dev.id : paNoDevice;
			P.channelCount = mAudioIO.mBufI.channels();
			P.sampleFormat = paFloat32; // | paNonInterleaved
			const auto * dInfo = Pa_GetDeviceInfo(P.device);
			if(dInfo) P.suggestedLatency = dInfo->defaultLowInputLatency; // for RT
			P.hostApiSpecificStreamInfo = nullptr;
		}
		{
			const auto& dev = mAudioIO.mDevO;
			auto& P = po;
			P.device = dev.id >= 0 ? dev.id : paNoDevice;
			P.channelCount = mAudioIO.mBufO.channels();
			P.sampleFormat = paFloat32; // | paNonInterleaved
			const auto * dInfo = Pa_GetDeviceInfo(P.device);
			if(dInfo) P.suggestedLatency = dInfo->defaultLowOutputLatency; // for RT
			P.hostApiSpecificStreamInfo = nullptr;
		}

		//mAudioIO.mDevI.print();
		//mAudioIO.mDevO.print();

		// Must pass in nullptrs for input- or output-only streams.
		// Stream will not be opened if no device or channel count is zero
		auto * ppi = &pi;
		auto * ppo = &po;
		if((paNoDevice == pi.device) || (0 == pi.channelCount)) ppi = nullptr;
		if((paNoDevice == po.device) || (0 == po.channelCount)) ppo = nullptr;

		if(paFormatIsSupported != Pa_IsFormatSupported(ppi, ppo, mAudioIO.fps())){
			//warn("Stream configuration not supported");
			return false;
		}

		return paNoError == Pa_OpenStream(
			&mStream,			// PortAudioStream **
			ppi,				// PaStreamParameters * in
			ppo,				// PaStreamParameters * out
			mAudioIO.mFramesPerSecond,	// frames/sec (double)
			mAudioIO.mFramesPerBuffer,	// frames/buffer (unsigned long)
			paNoFlag,			// paNoFlag, paClipOff, paDitherOff
			paCallback,			// static callback function (PaStreamCallback *)
			&mAudioIO
		);
	}

	bool start(){ return paNoError == Pa_StartStream(mStream); }
	bool close(){ return paNoError == Pa_CloseStream(mStream); }
	bool stop(){ return paNoError == Pa_StopStream(mStream); }

	AudioIO& mAudioIO;
	PaStream * mStream = nullptr;		// i/o stream
	//mutable PaError mErrNum = 0;		// Most recent error number
	bool mInited;
};

//double AudioIO::cpu() const { return Pa_GetStreamCpuLoad(mImpl->mStream); }

int AudioIO::numDevices() const { return Pa_GetDeviceCount(); }

AudioIO::Device deviceFromImplID(int devID){
	AudioIO::Device dev;
	auto * info = Pa_GetDeviceInfo(devID);
	dev.id = devID;
	dev.name = info->name;
	dev.frameRate = info->defaultSampleRate;
	dev.channelsIn  = info->maxInputChannels;
	dev.channelsOut = info->maxOutputChannels;
	return dev;
}

AudioIO::Device AudioIO::defaultDeviceIn() const {
	return deviceFromImplID(Pa_GetDefaultInputDevice());
}

AudioIO::Device AudioIO::defaultDeviceOut() const {
	return deviceFromImplID(Pa_GetDefaultOutputDevice());
}

AudioIO::Device AudioIO::device(int i) const {
	return deviceFromImplID(i);
}
//---- End PortAudio backend


#elif defined(AL_AUDIO_RTAUDIO)

#if defined(AL_OSX)
	#define __MACOSX_CORE__
#elif defined(AL_WINDOWS)
	//#define __WINDOWS_WASAPI__
	//#define __WINDOWS_DS__
	//#define __WINDOWS_ASIO__
#elif defined(AL_LINUX)
	//#define __LINUX_PULSE__
	#define __LINUX_ALSA__
	//#define __LINUX_OSS__
#endif
#include "rtaudio/RtAudio.h"
#include "rtaudio/RtAudio.cpp"

static int rtCallback(
	void * output,
	void * input,
	unsigned int frameCount,
	double streamTime,
	RtAudioStreamStatus status,
	void * userData
){
	auto& io = *static_cast<AudioIO *>(userData);
	const auto chanBufBytes = frameCount * sizeof(float);
	auto min = [](int a, int b){ return a<b?a:b; };

	// Copy input buffer to AudioIO
	if(input){
		auto& buf = io.bufferIn();
		int procChans = min(io.deviceIn().channelsIn, buf.channels());
		std::memcpy(buf.data(), input, chanBufBytes*procChans);
	}

	io.processAudio();

	/* Sine test
	static SineOsc osc;
	osc.freq = 440./io.fps();
	for(int i=0; i<frameCount; ++i){
		((float*)output)[i*io.channelsOut()] = osc()*0.2;
	}
	return 0;
	//*/

	// Copy AudioIO to output buffer
	if(output){
		const auto& buf = io.bufferOut();
		int procChans = min(io.deviceOut().channelsOut, buf.channels());
		std::memcpy(output, buf.data(), chanBufBytes*procChans);
	}

	return 0;
}

struct AudioIO::Impl{

	Impl(AudioIO& aio)
	:	mAudioIO(aio)
	{}

	bool open(){
		auto fps = (unsigned int)(mAudioIO.mFramesPerSecond+0.5);
		RtAudio::StreamParameters pi, po;
		
		auto configStream = [&](bool in){
			const auto& dev = in ? mAudioIO.mDevI : mAudioIO.mDevO;
			auto& P = in ? pi : po;
			P.deviceId = dev.id;
			P.nChannels = (in ? mAudioIO.mBufI : mAudioIO.mBufO).channels();
			// Do not try to open more channels than are available
			auto devChans = in ? dev.channelsIn : dev.channelsOut;
			//printf("want:%d have:%d\n", P.nChannels, devChans);
			if(P.nChannels > devChans) P.nChannels = devChans;
			if(P.nChannels){
				// Use nearest supported FPS
				auto rtDevInfo = mRtAudio.getDeviceInfo(dev.id);
				unsigned int fpsNearest = 0;
				for(auto v : rtDevInfo.sampleRates){
					fpsNearest = v;
					if(fpsNearest >= fps) break;
				}
				//printf("Nearest FPS:%d\n", fpsNearest);
				if(fpsNearest) fps = fpsNearest;
			}
		};
		configStream(true);
		configStream(false);
		
		mAudioIO.mFramesPerSecond = fps;

		// Must pass in nullptrs for input- or output-only streams.
		// Stream will not be opened if no device or channel count is zero
		auto * ppi = pi.nChannels ? &pi : nullptr;
		auto * ppo = po.nChannels ? &po : nullptr;
		unsigned int framesPerBuf = mAudioIO.mFramesPerBuffer;

		RtAudio::StreamOptions opts;
		opts.flags = RTAUDIO_SCHEDULE_REALTIME; // | RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_NONINTERLEAVED
		//opts.priority = 4; // real-time thread priority: what should this be?

		auto err = mRtAudio.openStream(
			ppo,			// RtAudio::StreamParameters * out
			ppi,			// RtAudio::StreamParameters * in
			RTAUDIO_FLOAT32,// RtAudioFormat
			fps,			// frames/sec (unsigned int)
			&framesPerBuf,	// unsigned int *bufferFrames
			rtCallback,		// static callback function (RtAudioCallback)
			&mAudioIO,		// void * userData
			&opts			// RtAudio::StreamOptions * (NULL for defaults)
		);

		if(RTAUDIO_NO_ERROR == err){
			if(framesPerBuf != mAudioIO.mFramesPerBuffer){
				AL_WARN("Opened audio stream with %d frames/buffer instead of requested %d frames/buffer", framesPerBuf, mAudioIO.mFramesPerBuffer);
				mAudioIO.configure(framesPerBuf, fps, ppo?ppo->nChannels:0, ppi?ppi->nChannels:0);
			}
			return true;
		}

		//printf("Error opening stream (err=%d)\n", err);

		return false;
	}

	bool start(){ return RTAUDIO_NO_ERROR == mRtAudio.startStream(); }
	bool stop(){ return RTAUDIO_NO_ERROR == mRtAudio.stopStream(); }
	bool close(){ mRtAudio.closeStream(); return true; }

	AudioIO::Device deviceFromImplID(int devID){
		AudioIO::Device dev;
		auto info = mRtAudio.getDeviceInfo(devID);
		//printf("[%2d] chansI/O/D:%d/%d/%d SR(curr):%d SR(pref):%d\n", devID, info.inputChannels, info.outputChannels, info.duplexChannels, info.currentSampleRate, info.preferredSampleRate);
		dev.id = devID;
		dev.name = info.name;
		//dev.frameRate = info.currentSampleRate;
		dev.frameRate = info.preferredSampleRate;
		dev.channelsIn  = info.inputChannels;
		dev.channelsOut = info.outputChannels;
		//printf("[%2d] ",devID); for(auto sr : info.sampleRates) printf("%d ", sr); printf("\n");
		return dev;
	}

	AudioIO& mAudioIO;
	RtAudio mRtAudio;
};

int AudioIO::numDevices() const { return mImpl->mRtAudio.getDeviceCount(); }

AudioIO::Device AudioIO::defaultDeviceIn() const {
	return mImpl->deviceFromImplID(mImpl->mRtAudio.getDefaultInputDevice());
}

AudioIO::Device AudioIO::defaultDeviceOut() const {
	return mImpl->deviceFromImplID(mImpl->mRtAudio.getDefaultOutputDevice());
}

AudioIO::Device AudioIO::device(int i) const {
	return mImpl->deviceFromImplID(i);
}
//---- End RtAudio backend


#elif defined(AL_AUDIO_SDL)

#include "al_SDL.hpp"

void initSDLAudio(){
	static bool needsInit = true;
	if(needsInit){
		if(SDL_INIT_ERROR(SDL_InitSubSystem(SDL_INIT_AUDIO))){
			AL_WARN("Failed to init SDL audio");
		}
		else{
			needsInit = false;
		}
	}
}

#if defined USING_SDL3

#define AL_DEFAULT_DEVICE_IN  0x7ffffffe
#define AL_DEFAULT_DEVICE_OUT 0x7fffffff

SDL_AudioDeviceID deviceID_al2sdl(int id){
	if(AL_DEFAULT_DEVICE_IN == id){
		return SDL_AUDIO_DEVICE_DEFAULT_RECORDING;
	} else if(AL_DEFAULT_DEVICE_OUT == id){
		return SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
	}
	return id;
}

int deviceID_sdl2al(SDL_AudioDeviceID id){
	if(id == SDL_AUDIO_DEVICE_DEFAULT_RECORDING){
		return AL_DEFAULT_DEVICE_IN;
	} else if(id == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK){
		return AL_DEFAULT_DEVICE_OUT;
	}
	return id;
}


struct AudioIO::Impl{
	Impl(AudioIO& aio)
	:	mAudioIO(aio)
	{
		initSDLAudio();
	}

	// https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_audio.h
	// https://github.com/libsdl-org/SDL/blob/main/examples/audio/02-simple-playback-callback/simple-playback-callback.c
	bool open(){

		// TODO: audio input

		auto audioCB = [](void * userData, SDL_AudioStream * stream, int bytesNeeded, int bytesBuffering){

			auto& io = *static_cast<AudioIO *>(userData);
			auto& buf = io.bufferOut();

			const int bytesPerSample = sizeof(float);
			const auto samplesPerBlock = buf.samples();
			const auto bytesPerBlock = samplesPerBlock * bytesPerSample;
			auto samplesNeeded = bytesNeeded/bytesPerSample;

			//printf("SDL3 audioCB (needs:%d b:%d)\n", samplesNeeded, samplesPerBlock);

			while(samplesNeeded > 0){

				io.processAudio();

				// Copy AudioIO to output buffer
				SDL_PutAudioStreamData(stream, buf.data(), bytesPerBlock);

				samplesNeeded -= samplesPerBlock;
			}
		};

		if(mAudioIO.mDevO.hasOutput()){
			SDL_AudioSpec want;
			want.channels = mAudioIO.mBufO.channels();
			want.freq = mAudioIO.fps();
			want.format = SDL_AUDIO_F32;

			auto sdlID = deviceID_al2sdl(mAudioIO.mDevO.id);

			//printf("SDL_OpenAudioDeviceStream(%u c:%d r:%d)\n", sdlID, want.channels, want.freq);
			auto * stream = SDL_OpenAudioDeviceStream(sdlID, &want, audioCB, &mAudioIO);
	
			if(stream){
				deviceStreamO.dev = sdlID;
				deviceStreamO.stream = stream;

				// TODO: validate format?
				//SDL_AudioSpec got;
				//SDL_GetAudioStreamFormat(stream, nullptr, &got);

				return true;

			} else {
				AL_WARN("Failed to create audio output stream: %s", SDL_GetError());
			}

		}

		return false;
	}

	bool start(){
		bool res = false;
		for(auto& dev : deviceStreams) res |= dev.start();
		return res;
	}

	bool stop(){
		bool res = false;
		for(auto& dev : deviceStreams) res |= dev.stop();
		return res;
	}

	bool close(){
		for(auto& dev : deviceStreams) dev.close();
		return true;
	}


	struct DeviceStream{
		SDL_AudioDeviceID dev = 0; // valid devices are >0
		SDL_AudioStream * stream = nullptr;

		bool start(){
			if(stream) return SDL_ResumeAudioStreamDevice(stream);
			return false;
		}

		bool stop(){
			if(stream) return SDL_PauseAudioStreamDevice(stream);
			return false;
		}

		void close(){
			if(stream){
				SDL_DestroyAudioStream(stream); // also closes audio device
				stream = nullptr;
				//SDL_CloseAudioDevice(dev);
				dev = 0;
			}
		}
	};

	AudioIO& mAudioIO;
	DeviceStream deviceStreams[2];
	DeviceStream& deviceStreamI = deviceStreams[0];
	DeviceStream& deviceStreamO = deviceStreams[1];
};


// Use RAII for getting devices
struct Devices {
	~Devices(){
		SDL_free(devs);
	}
	SDL_AudioDeviceID * devs = nullptr;
	int count = 0;
};

struct OutputDevices : public Devices {
	OutputDevices(){
		devs = SDL_GetAudioPlaybackDevices(&count);
	}
};

struct InputDevices : public Devices {
	InputDevices(){
		devs = SDL_GetAudioRecordingDevices(&count);
	}
};

AudioIO::Device deviceFromSDL(SDL_AudioDeviceID id){
	AudioIO::Device dev;

	// Default device IDs are "special" in that they cannot be passed in directly to several of the getter functions. Since there seems no way to get a "physical" default device ID, we must get a "logical" ID by actually opening the device!
	//if(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK == id || SDL_AUDIO_DEVICE_DEFAULT_RECORDING == id){
	if(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK == id){
		auto tempID = SDL_OpenAudioDevice(id, nullptr);
		dev = deviceFromSDL(tempID);
		dev.id = deviceID_sdl2al(id);
		SDL_CloseAudioDevice(tempID);
	} else if(SDL_AUDIO_DEVICE_DEFAULT_RECORDING == id){
		// For input devices, we do not want to open a device as it can force a prompt in the browser. Instead, we will just grab the first device in the list, if any.
		InputDevices idevs;
		if(idevs.count){
			return deviceFromSDL(idevs.devs[0]);
		}
	} else if(id){ // SDL IDs of 0 are invalid

		dev.id = id;
		dev.name = SDL_GetAudioDeviceName(dev.id);
		//printf("%s\n", dev.name.c_str());

		SDL_AudioSpec spec;
		int blockSize;
		SDL_GetAudioDeviceFormat(dev.id, &spec, &blockSize);

		dev.frameRate = spec.freq;

		if(SDL_IsAudioDevicePlayback(dev.id)){
			dev.channelsIn  = 0;
			dev.channelsOut = spec.channels;
		} else {
			dev.channelsIn  = spec.channels;
			dev.channelsOut = 0;
		}
	}

	return dev;
}

int AudioIO::numDevices() const {
	return InputDevices().count + OutputDevices().count;
}

AudioIO::Device AudioIO::device(int i) const {
	// SDL3 decided not to simply use enumerated device IDs, so we have to do some gymnastics to get a seemingly arbitrary ID from SDL.
	InputDevices idevs;
	OutputDevices odevs;

	int sdlID = 0;

	if(i < idevs.count){
		sdlID = idevs.devs[i];
	} else if(i < idevs.count + odevs.count){
		sdlID = odevs.devs[i - idevs.count];
	}

	return deviceFromSDL(sdlID);
}

AudioIO::Device AudioIO::defaultDeviceIn() const {
	return deviceFromSDL(SDL_AUDIO_DEVICE_DEFAULT_RECORDING);
}

AudioIO::Device AudioIO::defaultDeviceOut() const {
	return deviceFromSDL(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
}

// end SDL3

#elif defined USING_SDL2

#define SDL_AUDIO_F32 AUDIO_F32
#define AL_SDL_FALSE SDL_FALSE
#define AL_SDL_TRUE SDL_TRUE
#define AL_SDL_OUT AL_SDL_FALSE
#define AL_SDL_IN AL_SDL_TRUE

struct AudioIO::Impl{
	Impl(AudioIO& aio)
	:	mAudioIO(aio)
	{
		initSDLAudio();
	}

	bool open(){
		auto audioCallback = [](void * userData, unsigned char * stream, int streamSizeBytes){
			auto& io = *static_cast<AudioIO *>(userData);

			//const auto& specOut = backendData.specOut;
			//int numFrames = io.framesPerBuffer();

			//assert(specOut.samples == numFrames);
			/*const float **inBuffers = (const float **)input;
			for(int i = 0; i < mBufI.channels(); i++){
				memcpy(const_cast<float *>(&io.in(i, 0)), inBuffers[i], frameCount * sizeof(float));
			}*/

			io.processAudio();

			// Copy AudioIO buffers over to backend implementation
			/*int chansOut = io.bufferOut().channels();
			auto * sdlBuf = (float *)stream; // samples are interleaved in SDL
			for(int chan=0; chan<chansOut; ++chan){
				for(int i=0; i<numFrames; ++i){
					sdlBuf[i*chansOut + chan] = io.out(chan, i);
				}
			}*/

			// Copy AudioIO to output buffer
			std::memcpy(stream, io.bufferOut().data(), io.bufferOut().samples()*sizeof(float));
		};

		SDL_AudioSpec want;
		SDL_zero(want);
		want.freq = mAudioIO.fps();
		want.format = SDL_AUDIO_F32;
		want.samples = std::max(512, mAudioIO.mFramesPerBuffer); // values < 512 tend to create problems
		want.userdata = &mAudioIO;

		if(mAudioIO.mDevO.hasOutput()){
			auto& dev = mAudioIO.mDevO;
			want.channels = mAudioIO.mBufO.channels();
			want.callback = audioCallback;
			SDL_AudioSpec got;

			devO = SDL_OpenAudioDevice(
				dev.name.c_str(), AL_SDL_OUT, &want, &got,
				SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE
			);

			if(devO){
				bool reconfigure = false;
				if(mAudioIO.mFramesPerBuffer != got.samples){
					AL_WARN("Opened audio stream with %d frames/buffer instead of requested %d frames/buffer", got.samples, mAudioIO.mFramesPerBuffer);
					reconfigure = true;
				}
				if(want.freq != got.freq){
					AL_WARN("Opened audio stream at %d Hz instead of requested %d Hz", got.freq, want.freq);
					reconfigure = true;
				}
				if(want.channels != got.channels){
					AL_WARN("Opened audio stream with %d channels instead of requested %d channels", got.channels, want.channels);
					reconfigure = true;
				}
				if(reconfigure){
					mAudioIO.configure(got.samples, got.freq, got.channels, 0);
				}
			}
		}

		// TODO: audio input

		return devO;
	}

	bool start(){
		if(devO){
			SDL_PauseAudioDevice(devO, AL_SDL_FALSE);
			return true;
		}
		return false;
	}
	bool stop(){
		if(devO){
			SDL_PauseAudioDevice(devO, AL_SDL_TRUE);
			return true;
		}
		return false;
	}

	bool close(){
		for(auto& dev : devs){
			if(dev){
				SDL_CloseAudioDevice(dev); // no return value
				dev = 0;
			}
		}
		return true;
	}

	AudioIO& mAudioIO;
	SDL_AudioDeviceID devs[2] = {0, 0}; // valid devices are >0
	SDL_AudioDeviceID& devI  = devs[0]; 
	SDL_AudioDeviceID& devO = devs[1]; 
};

int AudioIO::numDevices() const {
	return SDL_GetNumAudioDevices(AL_SDL_OUT);
}

AudioIO::Device AudioIO::device(int i) const {
	AudioIO::Device dev;

	dev.id = i;
	dev.name = SDL_GetAudioDeviceName(i, AL_SDL_OUT);
	// The only way to get detailed device information seems to be to call
	// SDL_OpenAudioDevice. But, we don't want to open devices here, so just
	// choose reasonable defaults.
	dev.frameRate = 44100;
	dev.channelsIn  = 0;
	dev.channelsOut = 2;

	return dev;
}

AudioIO::Device AudioIO::defaultDeviceIn() const { return {}; }
AudioIO::Device AudioIO::defaultDeviceOut() const { return device(0); }

#endif // SDL2

#endif //---- End SDL backend



AudioBlock::~AudioBlock(){ clear(); }

AudioBlock& AudioBlock::resize(int frames, int chans){
	if(!mLocked){
		int newSize = frames*chans;
		if(samples() != newSize || !mOwner){
			clear();
			mFrames = frames;
			mChannels = chans;
			mData = new value_type[newSize];
			mOwner = true;
		}
	}
	return *this;
}

AudioBlock& AudioBlock::ref(float * src, int frames, int chans){
	if(!mLocked){
		clear();
		mData = src;
		mFrames = frames;
		mChannels = chans;
	}
	return *this;
}

AudioBlock& AudioBlock::zero(){
	std::memset(mData, 0, samples()*sizeof(value_type));
	return *this;
}

void AudioBlock::clear(){
	if(mData && mOwner) delete[] mData;
	mData = nullptr;
	mFrames = mChannels = 0;
	mOwner = false;
}

bool AudioIO::Device::nameMatches(const char * key) const {
	return name.find(key) != std::string::npos;
}

void AudioIO::Device::print() const {
	if(valid()){
		printf("[%2d] %s: %g Hz", id, name.c_str(), frameRate);
		if(channelsIn ) printf(", %d in", channelsIn);
		if(channelsOut) printf(", %d out", channelsOut);
		printf("\n");
	} else {
		printf("Invalid audio device\n");
	}
}


AudioIO::AudioIO()
:	mImpl(new Impl(*this)),
	mDevI(defaultDeviceIn()),
	mDevO(defaultDeviceOut())
{}

AudioIO::~AudioIO(){
	close();
	delete mImpl;
}

AudioIO& AudioIO::deviceIn(const Device& d){
	if(d.hasInput()) mDevI = d;
	return *this;
}
 
AudioIO& AudioIO::deviceOut(const Device& d){
	if(d.hasOutput()) mDevO = d;
	return *this;
}

AudioIO::Device AudioIO::findDevice(std::function<bool(AudioIO::Device d)> pred) const {
	for(int i=0; i<numDevices(); ++i){
		auto d = device(i);
		if(pred(d)) return d;
	}
	return {};
}

AudioIO& AudioIO::configure(int framesPerBuf, double framesPerSec, int chansOut, int chansIn){
	if(!mIsOpen){
		if(chansIn <0) chansIn  = mDevI.channelsIn;
		if(chansOut<0) chansOut = mDevO.channelsOut;

		#ifdef AL_LINUX
		/* The default device can report an insane number of max channels,
	  presumably because it's being remapped through a software mixer;
	  Opening all of them can cause an assertion dump in snd_pcm_area_copy
	  so we limit "all channels" to a reasonable number.*/
		if(chansIn  >= 128) chansIn  = 2;
		if(chansOut >= 128) chansOut = 2;
		#endif
		#ifdef AL_AUDIO_RTAUDIO
		// Prevent glitchy output when specifying SR less than that of input stream
		if(chansIn && framesPerSec < mDevI.frameRate){
			framesPerSec = mDevI.frameRate;
		}
		#endif
		mBufI.resize(framesPerBuf, chansIn);
		mBufI.zero(); // zero in case device has fewer channels than requested
		mBufO.resize(framesPerBuf, chansOut);
		mFramesPerBuffer = framesPerBuf;
		mFramesPerSecond = framesPerSec;
	}
	return *this;
}

bool AudioIO::open(){
	if(!mIsOpen && !mIsRunning){
		mIsOpen = mImpl->open();
		if(mIsOpen){
			mBufI.mLocked = true;
			mBufO.mLocked = true;
		}
		return mIsOpen;
	}
	return false;
}

bool AudioIO::start(){
	if(!mIsOpen) open();
	if(mIsOpen){
		mIsRunning = mImpl->start();
		return mIsRunning;
	}
	return false;
}

bool AudioIO::stop(){
	if(!mIsOpen) return false;
	if(mIsRunning){
		mIsRunning = !mImpl->stop();
		return !mIsRunning;
	}
	return false;
}

bool AudioIO::close(){
	stop();
	if(mIsOpen && !mIsRunning){
		mIsOpen = !mImpl->close();
		if(!mIsOpen){
			mBufI.mLocked = false;
			mBufO.mLocked = false;
		}
		return !mIsOpen;
	}
	return false;
}

AudioIO& AudioIO::processAudio(){

	if(mZeroOut) bufferOut().zero();

	if(mCallback){
		frame(0);
		mCallback(*this);
	}

	for(auto& cb : mCallbacks){
		frame(0);
		cb(*this);
	}

	if(bufferOut().channels()){
		// Kill pesky nans so we don't hurt anyone's ears
		if(mZeroNANs){
			for(int i=0; i<bufferOut().samples(); ++i){
				auto& s = bufferOut()[i];
				if(s != s) s = 0.f;  // portable isnan; only nans do not equal themselves
			}
		}

		// Clip output to [-1,1]
		if(mClipOut){
			for(int i=0; i<bufferOut().samples(); ++i){
				auto& s = bufferOut()[i];
				if(s < -1.f) s = -1.f;
				else if(s > 1.f) s = 1.f;
			}
		}

		// Apply gain using linear ramp
		if(mGain != 1.f || mGainPrev != 1.f){
			auto dg = (mGain - mGainPrev) / mFramesPerBuffer;
			auto g = mGainPrev;
			for(int i=0; i<mFramesPerBuffer; ++i){
				for(int j=0; j<bufferOut().channels(); ++j){
					bufferOut().at(i,j) *= g;
				}
				g += dg;
			}
			mGainPrev = mGain;
		}
	}

	return *this;
}

AudioIO& AudioIO::append(const Callback& cb){
	mCallbacks.push_back(cb);
	return *this;
}

AudioIO& AudioIO::prepend(const Callback& cb){
	mCallbacks.insert(mCallbacks.begin(), cb);
	return *this;
}

AudioIO& AudioIO::remove(const Callback& cb){
	for(auto it = mCallbacks.begin(); it != mCallbacks.end(); ++it){
		if(it->target_type() == cb.target_type()){
			mCallbacks.erase(it);
			break;
		}
	}
	return *this;
}

void AudioIO::print() const {
	if(mDevI.id == mDevO.id){
		printf("I/O Device: "); mDevO.print();
	}
	else{
		printf("Device In:  "); mDevI.print();
		printf("Device Out: "); mDevO.print();
	}
		printf("Chans I/O:  %d/%d\n", mBufI.channels(), mBufO.channels());
		printf("Frames/sec: %g Hz\n", mFramesPerSecond);
		printf("Frames/buf: %d\n", mFramesPerBuffer);
}

void AudioIO::printDevices() const {
	for(int i=0; i<numDevices(); ++i) device(i).print();
}
