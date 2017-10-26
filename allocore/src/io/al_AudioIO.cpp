#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring> /* memset() */
#include <cstring>
#include <iostream>
#include <string>

#include "allocore/system/al_Printing.hpp"
#include "allocore/io/al_AudioIO.hpp"

#ifdef AL_AUDIO_RTAUDIO
#include "RtAudio.h"
#endif

#ifdef AL_AUDIO_PORTAUDIO
#include "portaudio.h"
#endif

namespace al {
#ifdef AL_AUDIO_DUMMY

struct AudioBackendData {
	int numOutChans, numInChans;
};

AudioBackend::AudioBackend() {
	mBackendData = std::make_shared<AudioBackendData>();
	static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = 2;
	static_cast<AudioBackendData *>(mBackendData.get())->numInChans = 2;
	mOpen = false;
	mRunning = false;
}

bool AudioBackend::isOpen() const { return mOpen; }

bool AudioBackend::isRunning() const { return mRunning; }

bool AudioBackend::error() const { return false; }

void AudioBackend::printError(const char *text) const {
	if (error()) {
		fprintf(stderr, "%s: Dummy error.\n", text);
	}
}

void AudioBackend::printInfo() const {
	printf("Using dummy backend (no audio).\n");
}

bool AudioBackend::supportsFPS(double fps) { return true; }

void AudioBackend::inDevice(int index) { return; }

void AudioBackend::outDevice(int index) { return; }

void AudioBackend::channels(int num, bool forOutput) {
	if (forOutput) {
		setOutDeviceChans(num);
	} else {
		setInDeviceChans(num);
	}
}

int AudioBackend::inDeviceChans() { return 2; }

int AudioBackend::outDeviceChans() { return 2; }

void AudioBackend::setInDeviceChans(int num) {
	static_cast<AudioBackendData *>(mBackendData.get())->numInChans = num;
}

void AudioBackend::setOutDeviceChans(int num) {
	static_cast<AudioBackendData *>(mBackendData.get())->numOutChans = num;
}

double AudioBackend::time() { return 0.0; }

bool AudioBackend::open(int framesPerSecond, int framesPerBuffer,
                        void *userdata) {
	mOpen = true;
	return true;
}

bool AudioBackend::close() {
	mOpen = false;
	return true;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
	mRunning = true;
	return true;
}

bool AudioBackend::stop() {
	mRunning = false;
	return true;
}

double AudioBackend::cpu() { return 0.0; }

AudioDevice AudioBackend::defaultInput() { return AudioDevice(0); }

AudioDevice AudioBackend::defaultOutput() { return AudioDevice(0); }

int AudioBackend::numDevices() { return 1; }

int AudioBackend::deviceMaxInputChannels(int num) { return 2; }

int AudioBackend::deviceMaxOutputChannels(int num) { return 2; }

double AudioBackend::devicePreferredSamplingRate(int num) { return 44100; }

std::string AudioBackend::deviceName(int num) { return "dummy_device"; }

#endif

//==============================================================================
#ifdef AL_AUDIO_PORTAUDIO

struct InitSingleton {
	InitSingleton() { mCleanUp = paNoError == Pa_Initialize(); }
	~InitSingleton() {
		if (mCleanUp) {
			Pa_Terminate();
		}
	}
	bool mCleanUp;
};

static InitSingleton dummy;

struct AudioBackendData {
	PaStreamParameters mInParams,
	mOutParams;           // Input and output stream parameters
	PaStream *mStream;        // i/o stream
	mutable PaError mErrNum;  // Most recent error number
};

AudioBackend::AudioBackend() {
	mBackendData = std::make_shared<AudioBackendData>();
	static_cast<AudioBackendData *>(mBackendData.get())->mStream = nullptr;
	static_cast<AudioBackendData *>(mBackendData.get())->mErrNum = paNoError;
}
bool AudioBackend::isOpen() const { return mOpen; }

bool AudioBackend::isRunning() const { return mRunning; }

bool AudioBackend::error() const {
	return static_cast<AudioBackendData *>(mBackendData.get())->mErrNum !=
	        paNoError;
}

void AudioBackend::printError(const char *text) const {
	if (error()) {
		fprintf(stderr, "%s: %s\n", text,
		        Pa_GetErrorText(
		            static_cast<AudioBackendData *>(mBackendData.get())->mErrNum));
	}
}

void AudioBackend::printInfo() const {
	const PaStreamInfo *sInfo = Pa_GetStreamInfo(
	            static_cast<AudioBackendData *>(mBackendData.get())->mStream);
	if (sInfo) {
		printf("In Latency:  %.0f ms\nOut Latency: %0.f ms\nSample Rate: %0.f Hz\n",
		       sInfo->inputLatency * 1000., sInfo->outputLatency * 1000.,
		       sInfo->sampleRate);
	}
}

bool AudioBackend::supportsFPS(double fps) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	const PaStreamParameters *pi =
	        data->mInParams.channelCount == 0 ? 0 : &data->mInParams;
	const PaStreamParameters *po =
	        data->mOutParams.channelCount == 0 ? 0 : &data->mOutParams;
	data->mErrNum = Pa_IsFormatSupported(pi, po, fps);
	printError("AudioIO::Impl::supportsFPS");
	return paFormatIsSupported == data->mErrNum;
}

void AudioBackend::inDevice(int index) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mInParams.device = index;
	const PaDeviceInfo *dInfo = Pa_GetDeviceInfo(data->mInParams.device);
	if (dInfo)
		data->mInParams.suggestedLatency = dInfo->defaultLowInputLatency;  // for RT
	data->mInParams.sampleFormat = paFloat32 | paNonInterleaved;
	data->mInParams.hostApiSpecificStreamInfo = NULL;
}

void AudioBackend::outDevice(int index) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mOutParams.device = index;
	const PaDeviceInfo *dInfo = Pa_GetDeviceInfo(data->mOutParams.device);
	if (dInfo)
		data->mOutParams.suggestedLatency =
		        dInfo->defaultLowOutputLatency;  // for RT
	data->mOutParams.sampleFormat = paFloat32 | paNonInterleaved;
	data->mOutParams.hostApiSpecificStreamInfo = NULL;
}

void AudioBackend::channels(int num, bool forOutput) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	if (isOpen()) {
		AL_WARN("the number of channels cannnot be set with the stream open");
		return;
	}

	PaStreamParameters *params = forOutput ? &data->mOutParams : &data->mInParams;

	if (num == 0) {
		// params->device = paNoDevice;
		params->channelCount = 0;
		return;
	}

	const PaDeviceInfo *info = Pa_GetDeviceInfo(params->device);
	if (0 == info) {
		if (forOutput)
			AL_WARN("attempt to set number of channels on invalid output device");
		else
			AL_WARN("attempt to set number of channels on invalid input device");
		return;  // this particular device is not open, so return
	}

	// compute number of channels to give PortAudio
	int maxChans =
	        (int)(forOutput ? info->maxOutputChannels : info->maxInputChannels);

	// -1 means open all channels
	if (-1 == num) {
		num = maxChans;
#ifdef AL_LINUX
		/* The default device can report an insane number of max channels,
			presumably because it's being remapped through a software mixer;
			Opening all of them can cause an assertion dump in snd_pcm_area_copy
			so we limit "all channels" to a reasonable number.*/
		if (num >= 128) num = 2;
#endif
	} else {
		num = std::min(num, maxChans);
	}

	params->channelCount = num;
}

int AudioBackend::inDeviceChans() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return (int)data->mInParams.channelCount;
}

int AudioBackend::outDeviceChans() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return (int)data->mOutParams.channelCount;
}

void AudioBackend::setInDeviceChans(int num) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mInParams.channelCount = num;
}

void AudioBackend::setOutDeviceChans(int num) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mOutParams.channelCount = num;
}

double AudioBackend::time() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return (double)Pa_GetStreamTime(data->mStream);
}

static int paCallback(const void *input, void *output, unsigned long frameCount,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData);

bool AudioBackend::open(int framesPerSecond, int framesPerBuffer,
                        void *userdata) {
	assert(framesPerBuffer != 0 && framesPerSecond != 0 && userdata != NULL);
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());

	data->mErrNum = paNoError;

	if (!(isOpen() || isRunning())) {
		PaStreamParameters *inParams = &data->mInParams;
		PaStreamParameters *outParams = &data->mOutParams;

		// Must pass in 0s for input- or output-only streams.
		// Stream will not be opened if no device or channel count is zero
		if ((paNoDevice == inParams->device) || (0 == inParams->channelCount))
			inParams = 0;
		if ((paNoDevice == outParams->device) || (0 == outParams->channelCount))
			outParams = 0;

		data->mErrNum = Pa_OpenStream(
		            &data->mStream,   // PortAudioStream **
		            inParams,         // PaStreamParameters * in
		            outParams,        // PaStreamParameters * out
		            framesPerSecond,  // frames/sec (double)
		            framesPerBuffer,  // frames/buffer (unsigned long)
		            paNoFlag,         // paNoFlag, paClipOff, paDitherOff
		            paCallback,       // static callback function (PaStreamCallback *)
		            userdata);

		mOpen = paNoError == data->mErrNum;
	}

	printError("Error in al::AudioIO::open()");
	return paNoError == data->mErrNum;
}

bool AudioBackend::close() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mErrNum = paNoError;
	if (mOpen) data->mErrNum = Pa_CloseStream(data->mStream);
	if (paNoError == data->mErrNum) {
		mOpen = false;
		mRunning = false;
	}
	return paNoError == data->mErrNum;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mErrNum = paNoError;
	if (!isOpen()) {
		open(framesPerSecond, framesPerBuffer, userdata);
	}
	if (isOpen() && !isRunning()) data->mErrNum = Pa_StartStream(data->mStream);
	if (paNoError == data->mErrNum) mRunning = true;
	printError("Error in AudioIO::start()");
	return paNoError == data->mErrNum;
}

bool AudioBackend::stop() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->mErrNum = paNoError;
	if (mRunning) {
		data->mErrNum = Pa_StopStream(data->mStream);
	}
	if (paNoError == data->mErrNum) mRunning = false;
	return paNoError == data->mErrNum;
}

double AudioBackend::cpu() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return Pa_GetStreamCpuLoad(data->mStream);
}

AudioDevice AudioBackend::defaultInput() {
	return AudioDevice(Pa_GetDefaultInputDevice());
}

AudioDevice AudioBackend::defaultOutput() {
	return AudioDevice(Pa_GetDefaultOutputDevice());
}

bool AudioBackend::deviceIsValid(int num) {
	return Pa_GetDeviceInfo(num) != nullptr;
}

int AudioBackend::deviceMaxInputChannels(int num) {
	const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
	return info->maxInputChannels;
}

int AudioBackend::deviceMaxOutputChannels(int num) {
	const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
	return info->maxOutputChannels;
}

double AudioBackend::devicePreferredSamplingRate(int num) {
	const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
	return info->defaultSampleRate;
}

std::string AudioBackend::deviceName(int num) {
	const PaDeviceInfo *info = Pa_GetDeviceInfo(num);
	return info->name;
}

int AudioBackend::numDevices() { return Pa_GetDeviceCount(); }

static int paCallback(const void *input, void *output, unsigned long frameCount,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags, void *userData) {
	AudioIO &io = *(AudioIO *)userData;

	assert(frameCount == (unsigned)io.framesPerBuffer());
	const float **inBuffers = (const float **)input;
	for (int i = 0; i < io.channelsInDevice(); i++) {
		memcpy(const_cast<float *>(&io.in(i, 0)), inBuffers[i],
		       frameCount * sizeof(float));
	}

	if (io.autoZeroOut()) io.zeroOut();

	io.processAudio();  // call callback

	// apply smoothly-ramped gain to all output channels
	if (io.usingGain()) {
		float dgain = (io.mGain - io.mGainPrev) / frameCount;

		for (int j = 0; j < io.channelsOutDevice(); ++j) {
			float *out = io.outBuffer(j);
			float gain = io.mGainPrev;

			for (unsigned i = 0; i < frameCount; ++i) {
				out[i] *= gain;
				gain += dgain;
			}
		}

		io.mGainPrev = io.mGain;
	}

	// kill pesky nans so we don't hurt anyone's ears
	if (io.zeroNANs()) {
		for (unsigned i = 0; i < unsigned(frameCount * io.channelsOutDevice());
		     ++i) {
			float &s = (&io.out(0, 0))[i];
			// if(isnan(s)) s = 0.f;
			if (s != s) s = 0.f;  // portable isnan; only nans do not equal themselves
		}
	}

	if (io.clipOut()) {
		for (unsigned i = 0; i < unsigned(frameCount * io.channelsOutDevice());
		     ++i) {
			float &s = (&io.out(0, 0))[i];
			if (s < -1.f)
				s = -1.f;
			else if (s > 1.f)
				s = 1.f;
		}
	}

	float **outBuffers = (float **)output;
	for (int i = 0; i < io.channelsOutDevice(); i++) {
		memcpy(outBuffers[i], const_cast<float *>(&io.out(i, 0)),
		       frameCount * sizeof(float));
	}

	return 0;
}

#endif  // AL_USE_PORTAUDIO

#ifdef AL_AUDIO_RTAUDIO

struct AudioBackendData {
	RtAudio audio;
	RtAudio::StreamParameters iParams, oParams;
};

AudioBackend::AudioBackend() {
	mBackendData = std::make_shared<AudioBackendData>();
}

bool AudioBackend::isOpen() const {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return data->audio.isStreamOpen();
}

bool AudioBackend::isRunning() const {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return data->audio.isStreamRunning();
}

bool AudioBackend::error() const { return false; }

void AudioBackend::printError(const char *text) const {
	//    if(error()){
	//      fprintf(stderr, "%s: %s\n", text, Pa_GetErrorText(mErrNum));
	//    }
}

void AudioBackend::printInfo() const {
	//    audio.getVersion()
	//    const PaStreamInfo * sInfo = Pa_GetStreamInfo(mStream);
	//    if(sInfo){
	//      printf("In Latency:  %.0f ms\nOut Latency: %0.f ms\nSample Rate:
	//      %0.f Hz\n",
	//             sInfo->inputLatency * 1000., sInfo->outputLatency * 1000.,
	//             sInfo->sampleRate);
	//    }
}

bool AudioBackend::supportsFPS(double fps) {
	// const RtAudio::StreamParameters * pi = iParams.nChannels  == 0 ? nullptr
	// : &iParams;
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	const RtAudio::StreamParameters *po =
	        data->oParams.nChannels == 0 ? nullptr : &data->oParams;

	if (!po) return false;

	unsigned int f = fps;
	auto const &supported = data->audio.getDeviceInfo(po->deviceId).sampleRates;
	for (auto const &r : supported) {
		if (r == f) {
			// std::cout << "RtAudioBackend::supportsFPS, rate " << f << "
			// supported" << std::endl;
			return true;
		}
	}

	// std::cout << "rate " << f << " not supported" << std::endl;
	return false;

	//    mErrNum = Pa_IsFormatSupported(pi, po, fps);
	//    printError("AudioIO::Impl::supportsFPS");
	//    return paFormatIsSupported == mErrNum;
	// return true; // FIXME return correct value...
}

void AudioBackend::inDevice(int index) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->iParams.deviceId = index;
	data->iParams.firstChannel = 0;
	if (data->iParams.nChannels < 1) {
		data->iParams.nChannels = AudioDevice(index).channelsInMax();
	}
}

void AudioBackend::outDevice(int index) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->oParams.deviceId = index;
	data->oParams.firstChannel = 0;
	if (data->oParams.nChannels < 1) {
		data->oParams.nChannels = AudioDevice(index).channelsOutMax();
	}
}

int AudioBackend::inDeviceChans() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return (int)data->iParams.nChannels;
}

int AudioBackend::outDeviceChans() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return (int)data->oParams.nChannels;
}

void AudioBackend::setInDeviceChans(int num) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->iParams.nChannels = num;
}

void AudioBackend::setOutDeviceChans(int num) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	data->oParams.nChannels = num;
}

double AudioBackend::time() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	return data->audio.getStreamTime();
}

static int rtaudioCallback(void *output, void *input, unsigned int frameCount,
                           double streamTime, RtAudioStreamStatus status,
                           void *userData);

bool AudioBackend::open(int framesPerSecond, int framesPerBuffer,
                        void *userdata) {
	assert(framesPerBuffer != 0 && framesPerSecond != 0 && userdata != NULL);
	// Set the same number of channels for both input and output.
	//    unsigned int bufferBytes, bufferFrames = 512;

	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	unsigned int deviceBufferSize = framesPerBuffer;
	auto *ip = data->iParams.nChannels > 0 ? &data->iParams : nullptr;
	auto *op = data->oParams.nChannels > 0 ? &data->oParams : nullptr;
	try {
		data->audio.openStream(op, ip, RTAUDIO_FLOAT32, framesPerSecond,
		                       &deviceBufferSize, rtaudioCallback, userdata);
	} catch (RtAudioError &e) {
		e.printMessage();
		return false;
	}

	if (deviceBufferSize != framesPerBuffer) {
		printf("WARNING: Device opened with buffer size: %d", deviceBufferSize);
	}
	return true;
}

bool AudioBackend::close() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	if (data->audio.isStreamOpen()) {
		data->audio.closeStream();
	}

	return true;
}

bool AudioBackend::start(int framesPerSecond, int framesPerBuffer,
                         void *userdata) {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	try {
		data->audio.startStream();
	} catch (RtAudioError &e) {
		e.printMessage();
		//          goto cleanup;
		return false;
	}
	return true;
}

bool AudioBackend::stop() {
	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	try {
		data->audio.stopStream();
	} catch (RtAudioError &e) {
		return false;
		//        e.printMessage();
		//        goto cleanup;
	}
	return true;
}

double AudioBackend::cpu() { return -1.0; }

AudioDevice AudioBackend::defaultInput() {
	RtAudio audio_;
	return AudioDevice(audio_.getDefaultInputDevice());
}

AudioDevice AudioBackend::defaultOutput() {
	RtAudio audio_;
	return AudioDevice(audio_.getDefaultOutputDevice());
}

int AudioBackend::numDevices() {
	RtAudio audio_;
	return audio_.getDeviceCount();
}

static int rtaudioCallback(void *output, void *input, unsigned int frameCount,
                           double streamTime, RtAudioStreamStatus status,
                           void *userData) {
	if (status) {
		std::cout << "Stream underflow detected!" << std::endl;
	}

	AudioIO &io = *(AudioIO *)userData;

	assert(frameCount == (unsigned)io.framesPerBuffer());
	const float *inBuffers = (const float *)input;
	float *hwInBuffer = const_cast<float *>(io.inBuffer(0));
	for (int frame = 0; frame < io.framesPerBuffer(); frame++) {
		for (int i = 0; i < io.channelsInDevice(); i++) {
			hwInBuffer[i * frameCount + frame] = *inBuffers++;
		}
	}
	if (io.autoZeroOut()) io.zeroOut();

	io.processAudio();  // call callback

	// apply smoothly-ramped gain to all output channels
	if (io.usingGain()) {
		float dgain = (io.mGain - io.mGainPrev) / frameCount;

		for (int j = 0; j < io.channelsOutDevice(); ++j) {
			float *out = io.outBuffer(j);
			float gain = io.mGainPrev;

			for (unsigned i = 0; i < frameCount; ++i) {
				out[i] *= gain;
				gain += dgain;
			}
		}

		io.mGainPrev = io.mGain;
	}

	// kill pesky nans so we don't hurt anyone's ears
	if (io.zeroNANs()) {
		for (unsigned i = 0; i < unsigned(frameCount * io.channelsOutDevice());
		     ++i) {
			float &s = (&io.out(0, 0))[i];
			// if(isnan(s)) s = 0.f;
			if (s != s) s = 0.f;  // portable isnan; only nans do not equal themselves
		}
	}

	if (io.clipOut()) {
		for (unsigned i = 0; i < unsigned(frameCount * io.channelsOutDevice());
		     ++i) {
			float &s = (&io.out(0, 0))[i];
			if (s < -1.f)
				s = -1.f;
			else if (s > 1.f)
				s = 1.f;
		}
	}

	float *outBuffers = (float *)output;

	float *finalOutBuffer = const_cast<float *>(io.outBuffer(0));
	for (int frame = 0; frame < io.framesPerBuffer(); frame++) {
		for (int i = 0; i < io.channelsOutDevice(); i++) {
			*outBuffers++ = finalOutBuffer[i * frameCount + frame];
		}
	}

	return 0;
}

void AudioBackend::channels(int num, bool forOutput) {
	if (isOpen()) {
		AL_WARN("the number of channels cannnot be set with the stream open");
		return;
	}

	AudioBackendData *data = static_cast<AudioBackendData *>(mBackendData.get());
	RtAudio::StreamParameters *params =
	        forOutput ? &data->oParams : &data->iParams;

	if (num == 0) {
		// params->device = paNoDevice;
		params->nChannels = 0;
		return;
	}
	RtAudio::DeviceInfo info;
	try {
		info = data->audio.getDeviceInfo(params->deviceId);
	} catch (RtAudioError &e) {
		e.printMessage();
	}
	if (!info.probed) {
		if (forOutput)
			AL_WARN("attempt to set number of channels on invalid output device");
		else
			AL_WARN("attempt to set number of channels on invalid input device");
		return;  // this particular device is not open, so return
	}

	// compute number of channels to give PortAudio
	int maxChans = (int)(forOutput ? info.outputChannels : info.inputChannels);

	// -1 means open all channels
	if (-1 == num) {
		num = maxChans;
#ifdef AL_LINUX
		/* The default device can report an insane number of max channels,
	  presumably because it's being remapped through a software mixer;
	  Opening all of them can cause an assertion dump in snd_pcm_area_copy
	  so we limit "all channels" to a reasonable number.*/
		if (num >= 128) num = 2;
#endif
	} else {
		num = std::min(num, maxChans);
	}

	params->nChannels = num;
}

std::string AudioBackend::deviceName(int num) {
	RtAudio rt;
	RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
	return info.name;
}

int AudioBackend::deviceMaxInputChannels(int num) {
	RtAudio rt;
	RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
	return info.inputChannels;
}

int AudioBackend::deviceMaxOutputChannels(int num) {
	RtAudio rt;
	RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
	return info.outputChannels;
}

double AudioBackend::devicePreferredSamplingRate(int num) {
	RtAudio rt;
	RtAudio::DeviceInfo info = rt.getDeviceInfo(num);
	return info.preferredSampleRate;
}

#endif

//==============================================================================

AudioDevice::AudioDevice(int deviceNum) : AudioDeviceInfo(deviceNum) {
	if (deviceNum < 0) {
		deviceNum = defaultOutput().id();
	}
	setImpl(deviceNum);
}

AudioDevice::AudioDevice(const std::string& nameKeyword, StreamMode stream)
:	AudioDeviceInfo(0)
{
	auto devNum = findDeviceNumber(nameKeyword, stream);
	if(devNum >= 0) setImpl(devNum);
}

AudioDevice::AudioDevice(std::initializer_list<std::string> nameKeywordList, StreamMode stream)
:	AudioDeviceInfo(0)
{
	auto devNum = findDeviceNumber(nameKeywordList, stream);
	if(devNum >= 0) setImpl(devNum);
}

int AudioDevice::findDeviceNumber(const std::string& nameKeyword, StreamMode stream){
 	for(int i=0; i<numDevices(); ++i){
 		AudioDevice d(i);
 		bool bi = (stream &  INPUT) && d.hasInput();
 		bool bo = (stream & OUTPUT) && d.hasOutput();
 		std::string n = d.name();
 		if(	(bi || bo) && n.find(nameKeyword) != std::string::npos){
			return i;
 		}
 	}
	return -1;
}

int AudioDevice::findDeviceNumber(std::initializer_list<std::string> nameKeywordList, StreamMode stream){
	for(auto& nameKeyword : nameKeywordList){
		auto devNum = findDeviceNumber(nameKeyword, stream);
		if(devNum >= 0) return devNum;
	}
	return -1;
}

AudioDevice AudioDevice::defaultInput() { return AudioBackend::defaultInput(); }

AudioDevice AudioDevice::defaultOutput() {
	return AudioBackend::defaultOutput();
}

void AudioDevice::initDevices() {}

int AudioDevice::numDevices() { return AudioBackend::numDevices(); }

void AudioDevice::print() const {
	if (!valid()) {
		printf("Invalid device\n");
		return;
	}

	printf("[%2d] %s, ", id(), name().c_str());

	int chans = channelsInMax();
	if (chans > 0) printf("%2i in, ", chans);
	chans = channelsOutMax();
	if (chans > 0) printf("%2i out, ", chans);

	printf("%.0f Hz\n", defaultSampleRate());

	//	PaSampleFormat sampleFormats = info->nativeSampleFormats;

	//	printf("[ ");
	//	if(0 != sampleFormats & paFloat32)		printf("f32 ");
	//	if(0 != sampleFormats & paInt32)		printf("i32 ");
	//	if(0 != sampleFormats & paInt24)		printf("i24 ");
	//	if(0 != sampleFormats & paInt16)		printf("i16 ");
	//	if(0 != sampleFormats & paInt8)			printf("i8 ");
	//	if(0 != sampleFormats & paUInt8)		printf("ui8 ");
	//	printf("], ");

	//	if(info->numSampleRates != -1){
	//		printf("[");
	//		for(int i=0; i<info->numSampleRates; i++){
	//			printf("%f ", info->sampleRates[i]);
	//		}
	//		printf("] Hz");
	//	}
	//	else{
	//		printf("[%.0f <-> %.0f] Hz", info->sampleRates[0],
	// info->sampleRates[1]);
	//	}
	//	printf("\n");
}

void AudioDevice::printAll() {
	for (int i = 0; i < numDevices(); i++) {
		printf("[%2d] ", i);
		AudioDevice dev(i);
		dev.print();
	}
}

void AudioDevice::setImpl(int deviceNum) {
	if (deviceNum >= 0) {
		initDevices();
		mID = deviceNum;
		mChannelsInMax = AudioBackend::deviceMaxInputChannels(deviceNum);
		mChannelsOutMax = AudioBackend::deviceMaxOutputChannels(deviceNum);
		mDefaultSampleRate = AudioBackend::devicePreferredSamplingRate(deviceNum);
		mName = AudioBackend::deviceName(deviceNum);
		mValid = true;
	} else {
		mValid = false;
	}
}

//==============================================================================

//AudioIO::AudioIO()
//    : AudioIOData(nullptr),
//      callback(nullptr),
//      mZeroNANs(true),
//      mClipOut(true),
//      mAutoZeroOut(true) { mBackend = std::make_shared<AudioBackend>(); }

AudioIO::~AudioIO() { close(); }

void AudioIO::init(int framesPerBuf, double framesPerSec,
                   void (*callbackA)(AudioIOData &), void *userData,
                   int outChansA, int inChansA) {
	callback = callbackA;
	user(userData);
	deviceIn(AudioDevice::defaultInput());
	deviceOut(AudioDevice::defaultOutput());
	//	init(outChansA, inChansA);
	channels(inChansA, false);
	channels(outChansA, true);
	this->framesPerBuffer(framesPerBuf);
	this->framesPerSecond(framesPerSec);
}

void AudioIO::initWithDefaults(void (*callback)(AudioIOData &), void *userData,
                               bool use_out, bool use_in,
                               int framesPerBuffer  // default 256
                               ) {
	bool use_both = use_out & use_in;
	bool use_either = use_out | use_in;

	auto default_in = AudioDevice::defaultInput();
	auto default_out = AudioDevice::defaultOutput();

	int in_channels = use_in ? default_in.channelsInMax() : 0;
	int out_channels = use_out ? default_out.channelsOutMax() : 0;

	double out_sampling_rate = default_out.defaultSampleRate();
	double in_sampling_rate = default_in.defaultSampleRate();
	double sampling_rate = 0;
	if (use_both) {
		if (out_sampling_rate != in_sampling_rate) {
			std::cout
			        << "default sampling rate different for in device and out device\n"
			        << "using only out device" << std::endl;
			in_channels = 0;
		}
		sampling_rate = out_sampling_rate;
	} else if (use_either) {
		sampling_rate = use_out ? out_sampling_rate : in_sampling_rate;
	} else {
		std::cout << "not initializing any audio device" << std::endl;
		return;
	}

	std::cout << "AudioIO: using default with\n"
	          << "in : [" << default_in.id() << "] " << in_channels
	          << " channels \n"
	          << "out: [" << default_out.id() << "] " << out_channels
	          << " channels \n"
	          << "buffer size: " << framesPerBuffer
	          << ", sampling rate: " << sampling_rate << std::endl;
	init(framesPerBuffer, sampling_rate, callback, userData,
	     out_channels, in_channels);
}

AudioIO &AudioIO::append(AudioCallback &v) {
	mAudioCallbacks.push_back(&v);
	return *this;
}

AudioIO &AudioIO::prepend(AudioCallback &v) {
	mAudioCallbacks.insert(mAudioCallbacks.begin(), &v);
	return *this;
}

AudioIO &AudioIO::insertBefore(AudioCallback &v) {
	std::vector<AudioCallback *>::iterator pos =
	        std::find(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v);
	if (pos == mAudioCallbacks.begin()) {
		prepend(v);
	} else {
		mAudioCallbacks.insert(--pos, 1, &v);
	}
	return *this;
}

AudioIO &AudioIO::insertAfter(AudioCallback &v) {
	std::vector<AudioCallback *>::iterator pos =
	        std::find(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v);
	if (pos == mAudioCallbacks.end()) {
		append(v);
	} else {
		mAudioCallbacks.insert(pos, 1, &v);
	}
	return *this;
}

AudioIO &AudioIO::remove(AudioCallback &v) {
	// the proper way to do it:
	mAudioCallbacks.erase(
	            std::remove(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v),
	            mAudioCallbacks.end());
	return *this;
}

void AudioIO::deviceIn(const AudioDevice &v) {
	if (v.valid() && v.hasInput()) {
		//		printf("deviceIn: %s, %d\n", v.name(), v.id());
		mInDevice = v;
		mBackend->inDevice(v.id());
		channelsIn(v.channelsInMax());
	} else {
		AL_WARN("attempt to set input device to a device without inputs");
	}
}

void AudioIO::deviceOut(const AudioDevice &v) {
	if (v.valid() && v.hasOutput()) {
		mOutDevice = v;
		mBackend->outDevice(v.id());
		channelsOut(v.channelsOutMax());
	} else {
		AL_WARN("attempt to set output device to a device without outputs");
	}
}

void AudioIO::device(const AudioDevice &v) {
	deviceIn(v);
	deviceOut(v);
}

void AudioIO::channelsBus(int num) {
	if (mBackend->isOpen()) {
		AL_WARN("the number of channels cannnot be set with the stream open");
		return;
	}

	resizeBuf(mBufB, num * mFramesPerBuffer);
	mNumB = num;
}

void AudioIO::channels(int num, bool forOutput) {
	// printf("Requested %d %s channels\n", num, forOutput?"output":"input");

	mBackend->channels(num, forOutput);

	// Open all device channels?
	if (num == -1) {
		num = (forOutput ? channelsOutDevice() : channelsInDevice());
	}

	const int oldChans = channels(forOutput);

	if (oldChans != num) {
		forOutput ? mNumO = num : mNumI = num;
		resizeBuffer(forOutput);
	}
	// printf("Set %d %s channels\n", forOutput?mNumO:mNumI,
	// forOutput?"output":"input");
}

void AudioIO::channelsIn(int n) { channels(n, false); }
void AudioIO::channelsOut(int n) { channels(n, true); }

int AudioIO::channelsInDevice() const { return (int)mBackend->inDeviceChans(); }
int AudioIO::channelsOutDevice() const {
	return (int)mBackend->outDeviceChans();
}

bool AudioIO::close() {
	if (mBackend != nullptr) {
		return mBackend->close();
	} else {
		return true;
	}
}

bool AudioIO::open() {
	return mBackend->open(mFramesPerSecond, mFramesPerBuffer, this);
}

void AudioIO::reopen() {
	if (mBackend->isRunning()) {
		close();
		start();
	} else if (mBackend->isOpen()) {
		close();
		open();
	}
}

void AudioIO::resizeBuffer(bool forOutput) {
	float *&buffer = forOutput ? mBufO : mBufI;
	int &chans = forOutput ? mNumO : mNumI;

	if (chans > 0 && mFramesPerBuffer > 0) {
		int n = resizeBuf(buffer, chans * mFramesPerBuffer);
		if (0 == n) chans = 0;
	} else {
		deleteBuf(buffer);
	}
}

void AudioIO::framesPerSecond(double v) {  // printf("AudioIO::fps(%f)\n", v);
	if (framesPerSecond() != v) {
		if (!supportsFPS(v)) v = mOutDevice.defaultSampleRate();
		mFramesPerSecond = v;
		reopen();
	}
}

void AudioIO::framesPerBuffer(int n) {
	if (mBackend->isOpen()) {
		AL_WARN("the number of frames/buffer cannnot be set with the stream open");
		return;
	}

	if (framesPerBuffer() != n) {
		mFramesPerBuffer = n;
		resizeBuffer(true);
		resizeBuffer(false);
		channelsBus(AudioIOData::channelsBus());
		resizeBuf(mBufT, mFramesPerBuffer);
	}
}

bool AudioIO::start() {
	return mBackend->start(mFramesPerSecond, mFramesPerBuffer, this);
}

bool AudioIO::stop() { return mBackend->stop(); }

bool AudioIO::supportsFPS(double fps) { return mBackend->supportsFPS(fps); }

void AudioIO::print() const {
	if (mInDevice.id() == mOutDevice.id()) {
		printf("I/O Device:  ");
		mInDevice.print();
	} else {
		printf("Device In:   ");
		mInDevice.print();
		printf("Device Out:  ");
		mOutDevice.print();
	}

	printf("Chans In:    %d (%dD + %dV)\n", channelsIn(), channelsInDevice(),
	       channelsIn() - channelsInDevice());
	printf("Chans Out:   %d (%dD + %dV)\n", channelsOut(), channelsOutDevice(),
	       channelsOut() - channelsOutDevice());

	mBackend->printInfo();
	printf("Frames/Buf:  %d\n", mFramesPerBuffer);
}

// void AudioIO::processAudio(){ frame(0); if(callback) callback(*this); }
void AudioIO::processAudio() {
	frame(0);
	if (callback) callback(*this);

	std::vector<AudioCallback *>::iterator iter = mAudioCallbacks.begin();
	while (iter != mAudioCallbacks.end()) {
		frame(0);
		(*iter++)->onAudioCB(*this);
	}
}

int AudioIO::channels(bool forOutput) const {
	return forOutput ? channelsOut() : channelsIn();
}

double AudioIO::cpu() const { return mBackend->cpu(); }
bool AudioIO::zeroNANs() const { return mZeroNANs; }

double AudioIO::time() const {
	assert(mBackend);
	return mBackend->time();
}
double AudioIO::time(int frame) const {
	return (double)frame / framesPerSecond() + time();
}

}  // al::
