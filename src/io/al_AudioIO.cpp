#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "io/al_AudioIO.hpp"

//#define SAFE_FREE(ptr) if(ptr){ free(ptr); ptr = 0; }

namespace al{

/*
static void err(const char * msg, const char * src, bool exits){
	fprintf(stderr, "%s%serror: %s\n", src, src[0]?" ":"", msg);
	if(exits) exit(EXIT_FAILURE);
}
*/

static void warn(const char * msg, const char * src){
	fprintf(stderr, "%s%swarning: %s\n", src, src[0]?" ":"", msg);
}

template <class T>
void deleteBuf(T *& buf){ delete buf; buf=0; }

int min(int x, int y){ return x<y?x:y; }

template <class T>
int resize(T *& buf, int n){
	deleteBuf(buf);
	buf = new T[n];
	return n;
}

template <class T>
void zero(T * buf, int n){ bzero(buf, n*sizeof(T)); }

template <class T>
void deinterleave(T * dst, const T * src, int numFrames, int numChannels){
	int numSamples = numFrames * numChannels;
	for(int c=0; c < numChannels; c++){
		for(int i=c; i < numSamples; i+=numChannels){
			*dst++ = src[i];
		}
	}
}

template <class T>
void interleave(T * dst, const T * src, int numFrames, int numChannels){
	int numSamples = numFrames * numChannels;
	for(int c=0; c < numChannels; c++){
		for(int i=c; i < numSamples; i+=numChannels){
			dst[i] = *src++;
		}
	}
}



AudioIOData::AudioIOData(void * userData) :
	user(userData),
	mStream(0),
	mFramesPerBuffer(0), mFramesPerSecond(0),
	mBufI(0), mBufO(0), mBufA(0), mBufT(0), mNumI(0), mNumO(0), mNumA(0)
{
}

AudioIOData::~AudioIOData(){
	deleteBuf(mBufI);
	deleteBuf(mBufO);
	deleteBuf(mBufA);
	deleteBuf(mBufT);
}


AudioDevice::AudioDevice(int deviceNum)
:	mID(-1), mImpl(0)
{
	setImpl(deviceNum);
}

AudioDevice::AudioDevice(const std::string& nameKeyword, bool input, bool output)
:	mID(-1), mImpl(0)
{
	for(int i=0; i<numDevices(); ++i){
		AudioDevice d(i);
		std::string n = d.name();
		if(	((input & d.hasInput()) || (output & d.hasOutput())) &&
			n.find(nameKeyword) != std::string::npos
		){
			setImpl(i);
			break;
		}
	}
}

AudioDevice::~AudioDevice(){}

const char * AudioDevice::name() const { return ((const PaDeviceInfo*)mImpl)->name; }
int AudioDevice::channelsInMax() const { return ((const PaDeviceInfo*)mImpl)->maxInputChannels; }
int AudioDevice::channelsOutMax() const { return ((const PaDeviceInfo*)mImpl)->maxOutputChannels; }
double AudioDevice::defaultSampleRate() const { return ((const PaDeviceInfo*)mImpl)->defaultSampleRate; }
bool AudioDevice::hasInput() const { return channelsInMax()>0; }
bool AudioDevice::hasOutput() const { return channelsOutMax()>0; }
void AudioDevice::setImpl(int deviceNum){ initDevices(); mImpl = Pa_GetDeviceInfo(deviceNum); mID=deviceNum; }
AudioDevice AudioDevice::defaultInput(){ return AudioDevice(Pa_GetDefaultInputDevice()); }
AudioDevice AudioDevice::defaultOutput(){ return AudioDevice(Pa_GetDefaultOutputDevice()); }

struct InitSingleton{
	InitSingleton(): mCleanUp(paNoError == Pa_Initialize()){}
	~InitSingleton(){ if(mCleanUp){ Pa_Terminate(); } }
	bool mCleanUp;
};

void AudioDevice::initDevices(){
	static InitSingleton dummy;
}

int AudioDevice::numDevices(){ initDevices(); return Pa_GetDeviceCount(); }

void AudioDevice::print() const{

	//if(deviceNum == paNoDevice){ printf("No device\n"); return; }

	//const AudioDevice dev(deviceNum);
	if(!valid()){ printf("Invalid device\n"); return; }

	printf("[%2d] %s, ", id(), name());
	
	int chans = channelsInMax();
	if(chans > 0) printf("%2i in, ", chans);
	chans = channelsOutMax();
	if(chans > 0) printf("%2i out, ", chans);

	printf("%.0f Hz\n", defaultSampleRate());
	
//	PaSampleFormat sampleFormats = info->nativeSampleFormats;
//	
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




//void (* AudioIO::callback)(AudioIOData &) = 0;

AudioIO::AudioIO(
	int framesPerBuf, double framesPerSec, void (* callbackA)(AudioIOData &), void * userData,
	int outChansA, int inChansA )
:	AudioIOData(userData),
	callback(callbackA),
	mErrNum(0),
	mInDevice(AudioDevice::defaultInput()), mOutDevice(AudioDevice::defaultOutput()),
	mIsOpen(false), mIsRunning(false), mInResizeDeferred(false), mOutResizeDeferred(false),
	mZeroNANs(true), mClipOut(true)
{
	init();
	this->framesPerBuffer(framesPerBuf);
	channels(inChansA, false);
	channels(outChansA, true);
	this->framesPerSecond(framesPerSec);
}

		
AudioIO::~AudioIO(){
	close();
}


void AudioIO::init(){

	// Choose default devices for now...
	deviceIn(AudioDevice::defaultInput());
	deviceOut(AudioDevice::defaultOutput());
	
//	inDevice(defaultInDevice());
//	outDevice(defaultOutDevice());
//	
//	// Setup input stream parameters
//	const PaDeviceInfo * dInfo = Pa_GetDeviceInfo(mInParams.device);	
//	if(dInfo) mInParams.suggestedLatency = dInfo->defaultLowInputLatency; // for RT
//	mInParams.sampleFormat = paFloat32;// | paNonInterleaved;
//	//mInParams.sampleFormat = paInt16;
//	mInParams.hostApiSpecificStreamInfo = NULL;
//
//	// Setup output stream parameters
//	dInfo = Pa_GetDeviceInfo(mOutParams.device);
//	if(dInfo) mOutParams.suggestedLatency = dInfo->defaultLowOutputLatency; // for RT
//	mOutParams.sampleFormat = paFloat32;// | paNonInterleaved;
//	mOutParams.hostApiSpecificStreamInfo = NULL;

	setInDeviceChans(0);
	setOutDeviceChans(0);
}

void AudioIO::deviceIn(const AudioDevice& v){

	if(v.valid() && v.hasInput()){
		inDevice(v.id());
		const PaDeviceInfo * dInfo = Pa_GetDeviceInfo(mInParams.device);	
		if(dInfo) mInParams.suggestedLatency = dInfo->defaultLowInputLatency; // for RT
		mInParams.sampleFormat = paFloat32;// | paNonInterleaved;
		//mInParams.sampleFormat = paInt16;
		mInParams.hostApiSpecificStreamInfo = NULL;
	}
	else{
		warn("attempt to set input device to a device without inputs", "io::AudioIO");
	}
}

void AudioIO::deviceOut(const AudioDevice& v){
	if(v.valid() && v.hasOutput()){
		outDevice(v.id());
		const PaDeviceInfo * dInfo = Pa_GetDeviceInfo(mOutParams.device);
		if(dInfo) mOutParams.suggestedLatency = dInfo->defaultLowOutputLatency; // for RT
		mOutParams.sampleFormat = paFloat32;// | paNonInterleaved;
		mOutParams.hostApiSpecificStreamInfo = NULL;
	}
	else{
		warn("attempt to set output device to a device without outputs", "io::AudioIO");
	}
}


void AudioIO::channelsAux(int num){
	mNumA = resize(mBufA, num * mFramesPerBuffer);
}


void AudioIO::channels(int num, bool forOutput){
	
	PaStreamParameters * params = forOutput ? &mOutParams : &mInParams;
	
	if(num == 0){
		//params->device = paNoDevice;
		params->channelCount = 0;
		return;
	}

	const PaDeviceInfo * info = Pa_GetDeviceInfo(params->device);
	if(0 == info){
		if(forOutput)	warn("attempt to set number of channels on invalid output device", "io::AudioIO");
		else			warn("attempt to set number of channels on invalid input device", "io::AudioIO");
		return;	// this particular device is not open, so return
	}


	// compute number of channels to give PortAudio
	int maxChans = 
		(int)(forOutput ? info->maxOutputChannels : info->maxInputChannels);
	
	// -1 means open all channels
	if(num == -1){
		num = maxChans;
	}
	
	int currentNum = channels(forOutput);
	
	if(num != currentNum){

		params->channelCount = min(num, maxChans);

		forOutput ? mNumO = num : mNumI = num;
		
		deferBufferResize(forOutput);
	}
}


bool AudioIO::close(){
	mErrNum = paNoError;
	
	if(mIsOpen) mErrNum = Pa_CloseStream(mStream);	
	
	if(paNoError == mErrNum){
		mIsOpen = false;
		mIsRunning = false;
	}
	
	return paNoError == mErrNum;
}


void AudioIO::deferBufferResize(bool forOutput){
	if(forOutput)	mOutResizeDeferred = true;
	else			mInResizeDeferred = true;
}


bool AudioIO::open(){

	mErrNum = paNoError;

	if(!(mIsOpen || mIsRunning)){

		resizeBuffer(false);
		resizeBuffer(true);

		resize(mBufT, mFramesPerBuffer);
		
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
			mFramesPerSecond,	// frames/sec (double)
			mFramesPerBuffer,	// frames/buffer (unsigned long)
            paNoFlag,			// paNoFlag, paClipOff, paDitherOff
			paCallback,			// static callback function (PaStreamCallback *)
			this
		);

		mIsOpen = paNoError == mErrNum;
	}
	//printf("AudioIO::open()\n"); printError();
	return paNoError == mErrNum;
}


int AudioIO::paCallback(const void *input,
						void *output,
						unsigned long frameCount,
						const PaStreamCallbackTimeInfo* timeInfo,
						PaStreamCallbackFlags statusFlags,
						void * userData )
{

	AudioIO& io = *(AudioIO *)userData;

	const float * paI = (const float *)input;
	float * paO = (float *)output;

	bool deinterleave = true;

	if(deinterleave){
		al::deinterleave((float *)io.in(0),  paI, io.framesPerBuffer(), io.channelsInDevice() );
		al::deinterleave(io.out(0), paO, io.framesPerBuffer(), io.channelsOutDevice());
	}

	io();	// call callback

	// kill pesky nans so we don't hurt anyone's ears
	if(io.zeroNANs()){
		for(int i=0; i<io.framesPerBuffer()*io.channelsOutDevice(); ++i){
			float& s = io.out(0)[i];
			if(isnan(s)) s = 0.f;
		}
	}
	
	if(io.clipOut()){
		for(int i=0; i<io.framesPerBuffer()*io.channelsOutDevice(); ++i){
			float& s = io.out(0)[i];
			if		(s<-1.f) s =-1.f;
			else if	(s> 1.f) s = 1.f;
		}		
	}

	if(deinterleave){
		interleave(paO, io.out(0), io.framesPerBuffer(), io.channelsOutDevice());
	}

	return 0;
}


void AudioIO::reopen(){
	if(mIsRunning)  { close(); start(); }
	else if(mIsOpen){ close(); open(); }
}

void AudioIO::resizeBuffer(bool forOutput){

	float *& buffer = forOutput ? mBufO : mBufI;
	int& chans   = forOutput ? mNumO : mNumI;
	bool& deferred = forOutput ? mOutResizeDeferred : mInResizeDeferred;

	if(deferred){
		if(chans > 0){			
			int n = resize(buffer, chans * mFramesPerBuffer);
			if(n){	deferred = false; }
			else{	chans = 0; }
		}
		else{
			deleteBuf(buffer);
			deferred = false;
		}
	}
}


void AudioIO::framesPerSecond(double v){	//printf("AudioIO::fps(%f)\n", v);
	if(AudioIOData::framesPerSecond() != v){
                
		if(!supportsFPS(v)) v = mOutDevice.defaultSampleRate();

		mFramesPerSecond = v;
		reopen();
	}
}


void AudioIO::framesPerBuffer(int n){
	if(framesPerBuffer() != n){
		mFramesPerBuffer = n;
		channelsAux(AudioIOData::channelsAux());
		reopen();
	}
}


bool AudioIO::start(){
	mErrNum = paNoError;
	
	if(!mIsOpen) open();
	if(mIsOpen && !mIsRunning)	mErrNum = Pa_StartStream(mStream);
	if(paNoError == mErrNum)	mIsRunning = true;
	
	return paNoError == mErrNum;
}


bool AudioIO::stop(){
	mErrNum = paNoError;
	
	if(mIsRunning)				mErrNum = Pa_StopStream(mStream);
	if(paNoError == mErrNum)	mIsRunning = false;
	
	return paNoError == mErrNum;
}


bool AudioIO::supportsFPS(double fps){

	PaStreamParameters * pi = AudioIOData::channelsInDevice() == 0 ? 0 : &mInParams;
	PaStreamParameters * po = AudioIOData::channelsOutDevice() == 0 ? 0 : &mOutParams;	
	mErrNum = Pa_IsFormatSupported(pi, po, fps);
	
	if(error()){ printf("AudioIO error: "); printError(); }
	
	return paFormatIsSupported == mErrNum;
}

//void AudioIO::virtualChans(int num, bool forOutput){
//
//	int * currNum = forOutput ? &mVOChans : &mVIChans;
//
//	if(num != *currNum){
//		*currNum = num;
//		deferBufferResize(forOutput);
//	}
//}

//void assignBufferAccessors(){
//	for(int i=0; i<mVIChans; ++i) mAIBufs[i + mDIChans] = mVIBufs + i * mFramesPerBuffer;
//	for(int i=0; i<mVOChans; ++i) mAOBufs[i + mDOChans] = mVOBufs + i * mFramesPerBuffer;
//}


void AudioIO::print(){
	if(mInDevice.id() == mOutDevice.id()){
		printf("I/O Device:  "); mInDevice.print();
	}
	else{
		printf("Device In:   "); mInDevice.print();
		printf("Device Out:  "); mOutDevice.print();
	}

		printf("Chans In:    %d (%dD + %dV)\n", channelsIn(), channelsInDevice(), channelsIn() - channelsInDevice());
		printf("Chans Out:   %d (%dD + %dV)\n", channelsOut(), channelsOutDevice(), channelsOut() - channelsOutDevice());

	const PaStreamInfo * sInfo = Pa_GetStreamInfo(mStream);
	if(sInfo){
		printf("In Latency:  %.0f ms\nOut Latency: %0.f ms\nSample Rate: %0.f Hz\n",
			sInfo->inputLatency * 1000., sInfo->outputLatency * 1000., sInfo->sampleRate);
	}
	printf("Frames/Buf:  %d\n", mFramesPerBuffer);
}


void AudioIO::printError(){
	printf("%s \n", errorText(mErrNum));
}



void AudioIOData::zeroAux(){ zero(mBufA, framesPerBuffer() * mNumA); }
void AudioIOData::zeroOut(){ zero(mBufO, channelsOut() * framesPerBuffer()); }

float *       AudioIOData::aux(int num){ return mBufA + num * framesPerBuffer(); }
const float * AudioIOData::in (int chn){ return mBufI + chn * framesPerBuffer(); }
float *       AudioIOData::out(int chn){ return mBufO + chn * framesPerBuffer(); }
float *       AudioIOData::temp(){ return mBufT; }

int AudioIOData::channelsIn () const { return mNumI; }
int AudioIOData::channelsOut() const { return mNumO; }
int AudioIOData::channelsAux() const { return mNumA; }
int AudioIOData::channelsInDevice() const { return (int)mInParams.channelCount; }
int AudioIOData::channelsOutDevice() const { return (int)mOutParams.channelCount; }

double AudioIOData::framesPerSecond() const { return mFramesPerSecond; }
double AudioIOData::time() const { return Pa_GetStreamTime(mStream); }
double AudioIOData::time(int frame) const { return (double)frame / framesPerSecond() + time(); }
int AudioIOData::framesPerBuffer() const { return mFramesPerBuffer; }
double AudioIOData::secondsPerBuffer() const { return (double)framesPerBuffer() / framesPerSecond(); }

void AudioIO::operator()(){ if(callback) callback(*this); }

int AudioIO::channels(bool forOutput) const { return forOutput ? channelsOut() : channelsIn(); }
double AudioIO::cpu() const { return Pa_GetStreamCpuLoad(mStream); }
bool AudioIO::zeroNANs() const { return mZeroNANs; }
const char * AudioIO::errorText(int errNum){ return Pa_GetErrorText(errNum); }

bool AudioIO::error() const { return mErrNum != paNoError; }
void AudioIO::inDevice(PaDeviceIndex index){ mInParams.device = index; }
void AudioIO::outDevice(PaDeviceIndex index){ mOutParams.device = index; }
void AudioIO::setInDeviceChans(int num){ mInParams.channelCount = num; }
void AudioIO::setOutDeviceChans(int num){ mOutParams.channelCount = num; }

PaDeviceIndex AudioIO::defaultInDevice(){ return Pa_GetDefaultInputDevice(); }
PaDeviceIndex AudioIO::defaultOutDevice(){ return Pa_GetDefaultOutputDevice(); }


} // al::
