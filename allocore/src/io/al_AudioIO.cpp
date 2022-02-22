#include <cstdio>
#include <cstring> // memcpy, memset
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp" // AL_WARN
using namespace al;


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
	dev.defSampleRate = info->defaultSampleRate;
	dev.chanIMax = info->maxInputChannels;
	dev.chanOMax = info->maxOutputChannels;
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
	#define __WINDOWS_WASAPI__
	//#define __WINDOWS_DS__
	//#define __WINDOWS_ASIO__
#elif defined(AL_LINUX)
	#define __LINUX_PULSE__
	//#define __LINUX_ALSA__
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

	// Copy input buffer to AudioIO
	std::memcpy(io.bufferIn().data(), input, io.bufferIn().samples()*sizeof(float));

	io.processAudio();

	// Copy AudioIO to output buffer
	std::memcpy(output, io.bufferOut().data(), io.bufferOut().samples()*sizeof(float));

	return 0;
}

struct AudioIO::Impl{

	Impl(AudioIO& aio)
	:	mAudioIO(aio)
	{}

	bool open(){
		RtAudio::StreamParameters pi, po;
		{
			const auto& dev = mAudioIO.mDevI;
			/*dev.print();
			if(mAudioIO.mDevI.chanIMax <= 0){
				printf("Attempt to open input stream on device without input\n");
			}*/
			auto& P = pi;
			P.deviceId = dev.id;
			P.nChannels = mAudioIO.mBufI.channels();
		}
		{
			const auto& dev = mAudioIO.mDevO;
			auto& P = po;
			P.deviceId = dev.id;
			P.nChannels = mAudioIO.mBufO.channels();
		}

		// Must pass in nullptrs for input- or output-only streams.
		// Stream will not be opened if no device or channel count is zero
		auto * ppi = &pi;
		auto * ppo = &po;
		if((0 == pi.nChannels)) ppi = nullptr;
		if((0 == po.nChannels)) ppo = nullptr;

		unsigned int framesPerBuf = mAudioIO.mFramesPerBuffer;

		RtAudio::StreamOptions opts;
		opts.flags = RTAUDIO_SCHEDULE_REALTIME; // | RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_NONINTERLEAVED

		auto fps = (unsigned int)(mAudioIO.mFramesPerSecond+0.5);

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

		if(framesPerBuf != mAudioIO.mFramesPerBuffer){
			// openStream changed frames/buffer
		}

		return RTAUDIO_NO_ERROR == err;
	}

	bool start(){ return RTAUDIO_NO_ERROR == mRtAudio.startStream(); }
	bool stop(){ return RTAUDIO_NO_ERROR == mRtAudio.stopStream(); }
	bool close(){ mRtAudio.closeStream(); return true; }

	AudioIO::Device deviceFromImplID(int devID){
		AudioIO::Device dev;
		auto info = mRtAudio.getDeviceInfo(devID);
		dev.id = devID;
		dev.name = info.name;
		//dev.defSampleRate = info.currentSampleRate;
		dev.defSampleRate = info.preferredSampleRate;
		dev.chanIMax = info.inputChannels;
		dev.chanOMax = info.outputChannels;
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

#define SDL_MAIN_HANDLED
#ifdef AL_EMSCRIPTEN
	#include <SDL/SDL.h>
#else
	#include <SDL2/SDL.h>
#endif

void initSDLAudio(){
	static bool needsInit = true;
	if(needsInit){
		if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0){
			AL_WARN("Failed to init SDL audio");
		}
		else{
			needsInit = false;
		}
	}
}

#define AL_SDL_OUT SDL_FALSE
#define AL_SDL_IN SDL_TRUE

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
		want.format = AUDIO_F32;
		want.samples = std::max(512, mAudioIO.mFramesPerBuffer); // values < 512 tend to create problems
		want.userdata = &mAudioIO;

		if(mAudioIO.mDevO.hasOutput()){
			auto& dev = mAudioIO.mDevO;
			want.channels = mAudioIO.mBufO.channels();
			want.callback = audioCallback;
			SDL_AudioSpec got;

			devOut = SDL_OpenAudioDevice(
				dev.name.c_str(), AL_SDL_OUT, &want, &got,
				SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE
			);

			if(devOut){
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

		return devOut;
	}

	bool start(){
		if(devOut){
			SDL_PauseAudioDevice(devOut, SDL_FALSE);
			return true;
		}
		return false;
	}
	bool stop(){
		if(devOut){
			SDL_PauseAudioDevice(devOut, SDL_TRUE);
			return true;
		}
		return false;
	}

	bool close(){
		if(devIn){
			SDL_CloseAudioDevice(devIn);
		}
		if(devOut){
			SDL_CloseAudioDevice(devOut);
		}
		return true;
	}

	AudioIO& mAudioIO;
	SDL_AudioDeviceID devIn=0, devOut=0; // valid devices are >0
};

int AudioIO::numDevices() const { return SDL_GetNumAudioDevices(AL_SDL_OUT); }
AudioIO::Device AudioIO::defaultDeviceIn() const { return {}; }
AudioIO::Device AudioIO::defaultDeviceOut() const { return device(0); }
AudioIO::Device AudioIO::device(int i) const {
	// The only way to get detailed device information seems to be to call
	// SDL_OpenAudioDevice. But, we don't want to open devices here, so just
	// choose reasonable defaults.
	AudioIO::Device dev;
	dev.id = i;
	dev.name = SDL_GetAudioDeviceName(i, AL_SDL_OUT);
	dev.defSampleRate = 44100;
	dev.chanIMax = 0;
	dev.chanOMax = 2;
	return dev;
}

//---- End SDL2 backend
#endif


AudioBlock::~AudioBlock(){ clear(); }

AudioBlock& AudioBlock::resize(int frames, int chans){
	int newSize = frames*chans;
	if(samples() != newSize || !mOwner){
		clear();
		mFrames = frames;
		mChannels = chans;
		mData = new value_type[newSize];
		mOwner = true;
	}
	return *this;
}

AudioBlock& AudioBlock::ref(float * src, int frames, int chans){
	clear();
	mData = src;
	mFrames = frames;
	mChannels = chans;
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


void AudioIO::Device::print() const {
	if(valid()){
		printf("[%2d] %s: %g Hz", id, name.c_str(), defSampleRate);
		if(chanIMax) printf(", %d in", chanIMax);
		if(chanOMax) printf(", %d out", chanOMax);
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
		if(chansIn <0) chansIn  = mDevI.chanIMax;
		if(chansOut<0) chansOut = mDevO.chanOMax;

#ifdef AL_LINUX
		/* The default device can report an insane number of max channels,
	  presumably because it's being remapped through a software mixer;
	  Opening all of them can cause an assertion dump in snd_pcm_area_copy
	  so we limit "all channels" to a reasonable number.*/
		if(chansIn  >= 128) chansIn  = 2;
		if(chansOut >= 128) chansOut = 2;
#endif

		mBufI.resize(framesPerBuf, chansIn);
		mBufO.resize(framesPerBuf, chansOut);
		mFramesPerBuffer = framesPerBuf;
		mFramesPerSecond = framesPerSec;
	}
	return *this;
}

bool AudioIO::open(){
	if(!mIsOpen && !mIsRunning){
		mIsOpen = mImpl->open();
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
		return mIsOpen;
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
		printf("Frames/Buf: %d\n", mFramesPerBuffer);
}

void AudioIO::printDevices() const {
	for(int i=0; i<numDevices(); ++i) device(i).print();
}
