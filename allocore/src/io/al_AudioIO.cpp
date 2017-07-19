#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>		/* memset() */
#include <cmath>
#include <cassert>

#include "portaudio.h"
#include "allocore/system/al_Config.h"
#include "allocore/system/al_Printing.hpp" // AL_WARN
#include "allocore/io/al_AudioIO.hpp"

namespace al{

class DummyAudioBackend : public AudioBackend{
public:
	DummyAudioBackend(): AudioBackend(), mNumOutChans(64), mNumInChans(64){}

	virtual bool isOpen() const {return mIsOpen;}
	virtual bool isRunning() const {return mIsRunning;}
	virtual bool error() const {return false;}

	virtual void printError(const char * text = "") const {
		if(error()){
			fprintf(stderr, "%s: Dummy error.\n", text);
		}
	}
	virtual void printInfo() const {
		printf("Using dummy backend (no audio).\n");
	}

	virtual bool supportsFPS(double fps) const {return true;}

	virtual void inDevice(int index) {return;}
	virtual void outDevice(int index) {return;}

	virtual void channels(int num, bool forOutput) {
		if (forOutput) {
			setOutDeviceChans(num);
		} else {
			setInDeviceChans(num);
		}
	}

	virtual int inDeviceChans() {return mNumInChans;}
	virtual int outDeviceChans() {return mNumOutChans;}
	virtual void setInDeviceChans(int num) {
		mNumInChans = num;
	}

	virtual void setOutDeviceChans(int num) {
		mNumOutChans = num;
	}

	virtual double time() {return 0.0;}

	virtual bool open(int framesPerSecond, int framesPerBuffer, void *userdata) { mIsOpen = true; return true; }

	virtual bool close() { mIsOpen = false; return true; }

	virtual bool start(int framesPerSecond, int framesPerBuffer, void *userdata) { mIsRunning = true; return true; }

	virtual bool stop() { mIsRunning = false; return true;}
	virtual double cpu() {return 0.0;}

protected:
	int mNumOutChans;
	int mNumInChans;
};


//==============================================================================

class PortAudioBackend : public AudioBackend{
public:
	PortAudioBackend(): AudioBackend(), mStream(0), mErrNum(0){ initialize(); }

	virtual bool isOpen() const { return mIsOpen;}
	virtual bool isRunning() const { return mIsRunning;}
	virtual bool error() const { return mErrNum != paNoError; }

	virtual void printError(const char * text = "") const {
		if(error()){
			fprintf(stderr, "%s: %s\n", text, Pa_GetErrorText(mErrNum));
		}
	}
	virtual void printInfo() const {
		const PaStreamInfo * sInfo = Pa_GetStreamInfo(mStream);
		if(sInfo){
			printf("In Latency:  %.0f ms\nOut Latency: %0.f ms\nSample Rate: %0.f Hz\n",
				   sInfo->inputLatency * 1000., sInfo->outputLatency * 1000., sInfo->sampleRate);
		}
	}

	virtual bool supportsFPS(double fps) const {
		const PaStreamParameters * pi = mInParams.channelCount  == 0 ? 0 : &mInParams;
		const PaStreamParameters * po = mOutParams.channelCount == 0 ? 0 : &mOutParams;	
		mErrNum = Pa_IsFormatSupported(pi, po, fps);
		printError("AudioIO::Impl::supportsFPS");
		return paFormatIsSupported == mErrNum;
	}

	virtual void inDevice(int index){
		mInParams.device = index;
		const PaDeviceInfo * dInfo = Pa_GetDeviceInfo(mInParams.device);
		if(dInfo) mInParams.suggestedLatency = dInfo->defaultLowInputLatency; // for RT
		mInParams.sampleFormat = paFloat32 | paNonInterleaved;
		mInParams.hostApiSpecificStreamInfo = NULL;
	}
	virtual void outDevice(int index){
		mOutParams.device = index;
		const PaDeviceInfo * dInfo = Pa_GetDeviceInfo(mOutParams.device);
		if(dInfo) mOutParams.suggestedLatency = dInfo->defaultLowOutputLatency; // for RT
		mOutParams.sampleFormat = paFloat32 | paNonInterleaved;
		mOutParams.hostApiSpecificStreamInfo = NULL;
	}

	virtual void channels(int num, bool forOutput) {
		if(isOpen()){
			AL_WARN("the number of channels cannnot be set with the stream open");
			return;
		}

		PaStreamParameters * params = forOutput ? &mOutParams : &mInParams;

		if(num == 0){
			//params->device = paNoDevice;
			params->channelCount = 0;
			return;
		}

		const PaDeviceInfo * info = Pa_GetDeviceInfo(params->device);
		if(0 == info){
			if(forOutput)	AL_WARN("attempt to set number of channels on invalid output device");
			else			AL_WARN("attempt to set number of channels on invalid input device");
			return;	// this particular device is not open, so return
		}


		// compute number of channels to give PortAudio
		int maxChans =
			(int)(forOutput ? info->maxOutputChannels : info->maxInputChannels);

		// -1 means open all channels
		if(-1 == num){
			num = maxChans;
			#ifdef AL_LINUX
			/* The default device can report an insane number of max channels, 
			presumably because it's being remapped through a software mixer;
			Opening all of them can cause an assertion dump in snd_pcm_area_copy
			so we limit "all channels" to a reasonable number.*/
			if(num >= 128) num = 2;
			#endif
		}
		else{
			num = num < maxChans ? num : maxChans;
		}

		params->channelCount = num;
	}

	virtual int inDeviceChans() { return (int) mInParams.channelCount; }
	virtual int outDeviceChans() { return (int) mOutParams.channelCount; }
	virtual void setInDeviceChans(int num){ mInParams.channelCount = num; }
	virtual void setOutDeviceChans(int num){mOutParams.channelCount = num; }

	virtual double time() {return (double) Pa_GetStreamTime(mStream); }

	virtual bool open(int framesPerSecond, int framesPerBuffer, void *userdata) {
		assert(framesPerBuffer != 0 && framesPerSecond != 0 && userdata != NULL);

		mErrNum = paNoError;

		if(!(isOpen() || isRunning())){

			PaStreamParameters * inParams = &mInParams;
			PaStreamParameters * outParams = &mOutParams;

			// Must pass in 0s for input- or output-only streams.
			// Stream will not be opened if no device or channel count is zero
			if((paNoDevice ==  inParams->device) || (0 ==  inParams->channelCount)) inParams  = 0;
			if((paNoDevice == outParams->device) || (0 == outParams->channelCount)) outParams = 0;

			mErrNum = Pa_OpenStream(
				&mStream,			// PortAudioStream **
				inParams,			// PaStreamParameters * in
				outParams,			// PaStreamParameters * out
				framesPerSecond,	// frames/sec (double)
				framesPerBuffer,	// frames/buffer (unsigned long)
				paNoFlag,			// paNoFlag, paClipOff, paDitherOff
				paCallback,			// static callback function (PaStreamCallback *)
				userdata
			);

			mIsOpen = paNoError == mErrNum;
		}

		printError("Error in al::AudioIO::open()");
		return paNoError == mErrNum;
	}

	virtual bool close(){
		mErrNum = paNoError;
		if(mIsOpen) mErrNum = Pa_CloseStream(mStream);
		if(paNoError == mErrNum){
			mIsOpen = false;
			mIsRunning = false;
		}
		return paNoError == mErrNum;
	}

	virtual bool start(int framesPerSecond, int framesPerBuffer, void *userdata){
		mErrNum = paNoError;
		if(!isOpen()) {
			open(framesPerSecond, framesPerBuffer, userdata);
		}
		if(isOpen() && !isRunning()) mErrNum = Pa_StartStream(mStream);
		if(paNoError == mErrNum) mIsRunning = true;
		printError("Error in AudioIO::start()");
		return paNoError == mErrNum;
	}

	virtual bool stop(){
		mErrNum = paNoError;
		if(mIsRunning)				mErrNum = Pa_StopStream(mStream);
		if(paNoError == mErrNum)	mIsRunning = false;
		return paNoError == mErrNum;
	}

	virtual double cpu() {
		return Pa_GetStreamCpuLoad(mStream);
	}

	static void initialize(){
		struct InitSingleton{
			InitSingleton(){ mCleanUp = paNoError == Pa_Initialize(); }
			~InitSingleton(){ if(mCleanUp){ Pa_Terminate(); } }
			bool mCleanUp;
		};
		static InitSingleton dummy;
	}
	static AudioDevice defaultInput() { initialize(); return AudioDevice(Pa_GetDefaultInputDevice()); }
	static AudioDevice defaultOutput() { initialize(); return AudioDevice(Pa_GetDefaultOutputDevice()); }
	static int numDevices() { initialize(); return Pa_GetDeviceCount(); }

protected:
	static int paCallback(  const void *input,
							void *output,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void * userData
	){
		AudioIO& io = *(AudioIO *)userData;

		assert(frameCount == (unsigned)io.framesPerBuffer());
		const float **inBuffers = (const float **) input;
		for (int i = 0; i < io.channelsInDevice(); i++) {
			memcpy(const_cast<float *>(&io.in(i,0)),  inBuffers[i], frameCount * sizeof(float));
		}

		if(io.autoZeroOut()) io.zeroOut();

		io.processAudio();	// call callback


		// apply smoothly-ramped gain to all output channels
		if(io.usingGain()){

			float dgain = (io.mGain-io.mGainPrev) / frameCount;

			for(int j=0; j<io.channelsOutDevice(); ++j){
				float * out = io.outBuffer(j);
				float gain = io.mGainPrev;

				for(unsigned i=0; i<frameCount; ++i){
					out[i] *= gain;
					gain += dgain;
				}
			}

			io.mGainPrev = io.mGain;
		}

		// kill pesky nans so we don't hurt anyone's ears
		if(io.zeroNANs()){
			for(unsigned i=0; i<unsigned(frameCount*io.channelsOutDevice()); ++i){
				float& s = (&io.out(0,0))[i];
				//if(isnan(s)) s = 0.f;
				if(s != s) s = 0.f; // portable isnan; only nans do not equal themselves
			}
		}

		if(io.clipOut()){
			for(unsigned i=0; i<unsigned(frameCount*io.channelsOutDevice()); ++i){
				float& s = (&io.out(0,0))[i];
				if		(s<-1.f) s =-1.f;
				else if	(s> 1.f) s = 1.f;
			}
		}

		float **outBuffers = (float **) output;
		for (int i = 0; i < io.channelsOutDevice(); i++) {
			memcpy(outBuffers[i], const_cast<float *>(&io.out(i,0)), frameCount * sizeof(float));
		}

		return 0;
	}

private:

	PaStreamParameters mInParams, mOutParams;	// Input and output stream parameters
	PaStream * mStream;					// i/o stream
	mutable PaError mErrNum;			// Most recent error number
};

//==============================================================================

AudioDevice::AudioDevice(int deviceNum)
:    AudioDeviceInfo(deviceNum), mImpl(0)
{
	if (deviceNum < 0) {
		deviceNum = defaultOutput().id();
	}
	setImpl(deviceNum);
}

AudioDevice::AudioDevice(const std::string& nameKeyword, StreamMode stream)
:	AudioDeviceInfo(0), mImpl(0)
{
	auto devNum = findDeviceNumber(nameKeyword, stream);
	if(devNum >= 0) setImpl(devNum);
}

AudioDevice::AudioDevice(std::initializer_list<std::string> nameKeywordList, StreamMode stream)
:	AudioDeviceInfo(0), mImpl(0)
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

bool AudioDevice::valid() const
{
	return 0!=mImpl;
}

void AudioDevice::setImpl(int deviceNum){
	if (deviceNum >= 0) {
		initDevices();
		mImpl = Pa_GetDeviceInfo(deviceNum);
		if (mImpl) {
			mID = deviceNum;
			strncpy(mName, ((const PaDeviceInfo*)mImpl)->name, 127);
			mName[127] = '\0';
			mChannelsInMax = ((const PaDeviceInfo*)mImpl)->maxInputChannels;
			mChannelsOutMax = ((const PaDeviceInfo*)mImpl)->maxOutputChannels;
			mDefaultSampleRate = ((const PaDeviceInfo*)mImpl)->defaultSampleRate;
		} else {
			printf("AudioDevice: Invalid device number %d\n", deviceNum);
		}
	}
}
AudioDevice AudioDevice::defaultInput(){ return PortAudioBackend::defaultInput(); }
AudioDevice AudioDevice::defaultOutput(){ return PortAudioBackend::defaultOutput(); }

void AudioDevice::initDevices(){
	PortAudioBackend::initialize();
}

bool AudioDevice::hasInput() const { return channelsInMax()>0; }
bool AudioDevice::hasOutput() const { return channelsOutMax()>0; }

int AudioDevice::numDevices(){ return PortAudioBackend::numDevices(); }

void AudioDevice::print() const{

	if(!valid()){ printf("Invalid device\n"); return; }

	printf("[%2d] %s, ", id(), name());

	int chans = channelsInMax();
	if(chans > 0) printf("%2i in, ", chans);
	chans = channelsOutMax();
	if(chans > 0) printf("%2i out, ", chans);

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
//		printf("[%.0f <-> %.0f] Hz", info->sampleRates[0], info->sampleRates[1]);
//	}
//	printf("\n");
}

void AudioDevice::printAll(){
	for(int i=0; i<numDevices(); i++){
		printf("[%2d] ", i);
		AudioDevice dev(i);
		dev.print();
		//print(i);
	}
}

//==============================================================================

AudioIO::AudioIO(int framesPerBuf, double framesPerSec, void (* callbackA)(AudioIOData &), void * userData,
	int outChansA, int inChansA, AudioIO::Backend backend)
:	AudioIOData(userData),
	callback(callbackA),
	mZeroNANs(true), mClipOut(true), mAutoZeroOut(true)
{
	switch(backend) {
	case PORTAUDIO:
		mImpl = new PortAudioBackend;
		mInDevice = AudioDevice::defaultInput();
		mOutDevice = AudioDevice::defaultOutput();
		break;
	case DUMMY:
		mImpl = new DummyAudioBackend;
		break;
	}
	init(outChansA, inChansA);
	this->framesPerBuffer(framesPerBuf);
	channels(inChansA, false);
	channels(outChansA, true);
	this->framesPerSecond(framesPerSec);
}


AudioIO::~AudioIO(){
	close();
	delete mImpl;
}


void AudioIO::init(int outChannels, int inChannels){
	// Choose default devices for now...
	deviceIn(AudioDevice::defaultInput());
	deviceOut(AudioDevice::defaultOutput());

	mImpl->setInDeviceChans(0);
	mImpl->setOutDeviceChans(0);
}

AudioIO& AudioIO::append(AudioCallback& v){
	mAudioCallbacks.push_back(&v);
	return *this;
}

AudioIO& AudioIO::prepend(AudioCallback& v){
	mAudioCallbacks.insert(mAudioCallbacks.begin(), &v);
	return *this;
}

AudioIO& AudioIO::insertBefore(AudioCallback& v){
	std::vector<AudioCallback *>::iterator pos
			= std::find(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v);
	if (pos == mAudioCallbacks.begin()) {
		prepend(v);
	} else {
		mAudioCallbacks.insert(--pos, 1, &v);
	}
	return *this;
}

AudioIO& AudioIO::insertAfter(AudioCallback& v){
	std::vector<AudioCallback *>::iterator pos
			= std::find(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v);
	if (pos == mAudioCallbacks.end()) {
		append(v);
	} else {
		mAudioCallbacks.insert(pos, 1, &v);
	}
	return *this;
}

AudioIO& AudioIO::remove(AudioCallback& v){
	// the proper way to do it:
	mAudioCallbacks.erase(std::remove(mAudioCallbacks.begin(), mAudioCallbacks.end(), &v), mAudioCallbacks.end());
	return *this;
}

void AudioIO::deviceIn(const AudioDevice& v){

	if(v.valid() && v.hasInput()){
//		printf("deviceIn: %s, %d\n", v.name(), v.id());
		mInDevice = v;
		mImpl->inDevice(v.id());
		channelsIn(v.channelsInMax());
	}
	else{
		AL_WARN("attempt to set input device to a device without inputs");
	}
}

void AudioIO::deviceOut(const AudioDevice& v){
	if(v.valid() && v.hasOutput()){
		mOutDevice = v;
		mImpl->outDevice(v.id());
		channelsOut(v.channelsOutMax());
	}
	else{
		AL_WARN("attempt to set output device to a device without outputs");
	}
}

void AudioIO::device(const AudioDevice& v){
	deviceIn(v); deviceOut(v);
}


void AudioIO::channelsBus(int num){

	if(mImpl->isOpen()){
		AL_WARN("the number of channels cannnot be set with the stream open");
		return;
	}

	resizeBuf(mBufB, num * mFramesPerBuffer);
	mNumB = num;
}


void AudioIO::channels(int num, bool forOutput){
	//printf("Requested %d %s channels\n", num, forOutput?"output":"input");

	mImpl->channels(num, forOutput);

	// Open all device channels?
	if(num == -1){
		num = (forOutput ? channelsOutDevice() : channelsInDevice());
	}

	const int oldChans = channels(forOutput);

	if(oldChans != num){
		forOutput ? mNumO = num : mNumI = num;
		resizeBuffer(forOutput);
	}
	//printf("Set %d %s channels\n", forOutput?mNumO:mNumI, forOutput?"output":"input");
}

void AudioIO::channelsIn(int n){
	channels(n,false);
}
void AudioIO::channelsOut(int n){
	channels(n,true);
}

int AudioIO::channelsInDevice() const { return (int)mImpl->inDeviceChans(); }
int AudioIO::channelsOutDevice() const { return (int)mImpl->outDeviceChans(); }


bool AudioIO::close(){ return mImpl->close(); }

bool AudioIO::open(){ return mImpl->open(mFramesPerSecond, mFramesPerBuffer, this); }

void AudioIO::reopen(){
	if(mImpl->isRunning())  { close(); start(); }
	else if(mImpl->isOpen()){ close(); open(); }
}

void AudioIO::resizeBuffer(bool forOutput){
	float *& buffer = forOutput ? mBufO : mBufI;
	int& chans      = forOutput ? mNumO : mNumI;

	if(chans > 0 && mFramesPerBuffer > 0){
		int n = resizeBuf(buffer, chans * mFramesPerBuffer);
		if(0 == n) chans = 0;
	}
	else{
		deleteBuf(buffer);
	}
}


void AudioIO::framesPerSecond(double v){	//printf("AudioIO::fps(%f)\n", v);
	if(framesPerSecond() != v){
		if(!supportsFPS(v)) v = mOutDevice.defaultSampleRate();
		mFramesPerSecond = v;
		reopen();
	}
}


void AudioIO::framesPerBuffer(int n){
	if(mImpl->isOpen()){
		AL_WARN("the number of frames/buffer cannnot be set with the stream open");
		return;
	}

	if(framesPerBuffer() != n){
		mFramesPerBuffer = n;
		resizeBuffer(true);
		resizeBuffer(false);
		channelsBus(AudioIOData::channelsBus());
		resizeBuf(mBufT, mFramesPerBuffer);
	}
}


bool AudioIO::start(){ return mImpl->start(mFramesPerSecond, mFramesPerBuffer, this); }

bool AudioIO::stop(){ return mImpl->stop(); }

bool AudioIO::supportsFPS(double fps) const { return mImpl->supportsFPS(fps); }

void AudioIO::print() const {
	if(mInDevice.id() == mOutDevice.id()){
		printf("I/O Device:  "); mInDevice.print();
	}
	else{
		printf("Device In:   "); mInDevice.print();
		printf("Device Out:  "); mOutDevice.print();
	}

	printf("Chans In:    %d (%dD + %dV)\n", channelsIn(), channelsInDevice(), channelsIn() - channelsInDevice());
	printf("Chans Out:   %d (%dD + %dV)\n", channelsOut(), channelsOutDevice(), channelsOut() - channelsOutDevice());

	mImpl->printInfo();
	printf("Frames/Buf:  %d\n", mFramesPerBuffer);
}


//void AudioIO::processAudio(){ frame(0); if(callback) callback(*this); }
void AudioIO::processAudio(){
	frame(0);
	if(callback) callback(*this);

	std::vector<AudioCallback *>::iterator iter = mAudioCallbacks.begin();
	while(iter != mAudioCallbacks.end()){
		frame(0);
		(*iter++)->onAudioCB(*this);
	}
}

int AudioIO::channels(bool forOutput) const {
	return forOutput ? channelsOut() : channelsIn();
}
double AudioIO::cpu() const { return mImpl->cpu(); }
bool AudioIO::zeroNANs() const { return mZeroNANs; }

} // al::
