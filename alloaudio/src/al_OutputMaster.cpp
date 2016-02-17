
#include <iostream>
#include <sstream>

#include "alloaudio/al_OutputMaster.hpp"
#include "allocore/system/al_Time.hpp"

#include "alloaudio/butter.h"

//#include "firfilter.h"

using namespace al;

OutputMaster::OutputMaster(int numChnls, double sampleRate,
                           const char *address, int port,
                           const char *sendAddress, int sendPort,
                           al_sec msg_timeout):
    mNumChnls(numChnls),
    mMeterBuffer(1024 * sizeof(float)), mFramesPerSec(sampleRate),
    mUseDb(false),
    osc::Recv(port, address, msg_timeout),
    mSendAddress(sendAddress), mSendPort(sendPort)
{
	allocateChannels(mNumChnls);
	initializeData();

	if (port < 0) {
		std::cout << "OutputMaster: Not using OSC input." << std::endl;
	} else {
		msghandler.outputmaster = this;
		if (!opened()) {
			std::cout << "Failed to open socket on UDP port " << port << " for OSC receive." << std::endl;
			std::cout << "Probably another program is already listening on that port." << std::endl;
		} else {
			handler(OutputMaster::msghandler);
			timeout(0.1); // set receiver to block with timeout
			start();
		}
	}
	if (mSendPort > 0 && strlen(sendAddress) > 1) {
		mRunMeterThread = 1;
		mMeterThread.start(OutputMaster::meterThreadFunc, (void *) this);
	}
}

OutputMaster::~OutputMaster()
{
	stop(); /* Stops OSC listener */
	mRunMeterThread = 0;
	mMeterCond.notify_all();
	mMeterThread.join();
}


void OutputMaster::setMasterGain(double gain)
{
	mMasterGain = gain;
}

void OutputMaster::setGain(int channelIndex, double gain)
{
	if (channelIndex >= 0 && channelIndex < mNumChnls) {
		mGains[channelIndex] = gain;
	} else {
		//        printf("Alloaudio error: set_gain() for invalid channel %i", channelIndex);
	}
}

void OutputMaster::setMuteAll(bool muteAll)
{
	mMuteAll = muteAll;
}

void OutputMaster::setClipperOn(bool clipperOn)
{
	mClipperOn = clipperOn;
}

void OutputMaster::setMeterUpdateFreq(double freq)
{
	mMeterUpdateSamples = (int)(mFramesPerSec/freq);
}

void OutputMaster::setMeterOn(bool meterOn)
{
	mMeterOn = meterOn;
}

int OutputMaster::getMeterValues(float *values)
{
	return mMeterBuffer.read((char *) values, mNumChnls * sizeof(float));
}

int OutputMaster::getNumChnls()
{
	return mNumChnls;
}

void OutputMaster::onAudioCB(AudioIOData &io)
{
	int i, chan = 0;
	int nframes = io.framesPerBuffer();
	double in_buf[nframes];
	double master_gain;

	mParameterQueue.update(0);
	master_gain = mMasterGain * (mMuteAll ? 0.0 : 1.0);
	for (chan = 0; chan < mNumChnls; chan++) {
		double gain = master_gain * mGains[chan];
		const float *in = io.outBuffer(chan);  // Yes, the input here is the output from previous runs for the io object
		float *out = io.outBuffer(chan);

		for (i = 0; i < nframes; i++) {
			in_buf[i] = *in++;
		}

		for (i = 0; i < nframes; i++) {
			*out = in_buf[i] * gain;
			if (mClipperOn && *out > master_gain) {
				*out = master_gain;
			}
			out++;
		}
	}

	if (mMeterOn) {
		for (chan = 0; chan < mNumChnls; chan++) {
			float *out = io.outBuffer(chan);
			for (i = 0; i < nframes; i++) {
				if (mMeters[chan] < *out) {
					mMeters[chan] = *out;
				}
				out++;
			}
		}
		mMeterCounter += nframes;
		if (mMeterCounter >= mMeterUpdateSamples) {
			mMeterBuffer.write( (char *) mMeters.data(), sizeof(float) * mNumChnls);
			memset(mMeters.data(), 0, sizeof(float) * mNumChnls);
			mMeterCounter = 0; // A little jitter but efficient
			mMeterCond.notify_all();
		}
	}
}

void OutputMaster::setGainTimestamped(al_sec until, int channelIndex, double gain)
{
	setGain(channelIndex, gain);
}

void OutputMaster::setMasterGainTimestamped(al_sec until, double gain)
{
	setMasterGain(gain);
}

void OutputMaster::setClipperOnTimestamped(al_sec until, bool on)
{
	setClipperOn(on);
}

void OutputMaster::setMuteAllTimestamped(al_sec until, bool on)
{
	setMuteAll(on);
}

void OutputMaster::setMeterOnTimestamped(al_sec until, bool on)
{
	setMeterOn(on);
}

void OutputMaster::setMeterupdateFreqTimestamped(al_sec until, double freq)
{
	setMeterUpdateFreq(freq);
}

std::string OutputMaster::addressPrefix() const
{
	return mAddressPrefix;
}

void OutputMaster::setAddressPrefix(const std::string &addressPrefix)
{
	mAddressPrefix = addressPrefix;
}

bool OutputMaster::meterAddrHasChannel() const
{
	return mMeterAddrHasChannel;
}

void OutputMaster::setMeterAddrHasChannel(bool meterAddrHasChannel)
{
	mMeterAddrHasChannel = meterAddrHasChannel;
}

void OutputMaster::initializeData()
{
	mMasterGain = 0.0;
	mMuteAll = false;
	mClipperOn = true;
	mAddressPrefix = "/Alloaudio";
	mMeterCounter = 0;
	mMeterOn = false;
	mMeterAddrHasChannel = false;

	setMeterUpdateFreq(10.0);
}

void OutputMaster::allocateChannels(int numChnls)
{
	mGains.resize(numChnls);
	mMeters.resize(numChnls);

	for (int i = 0; i < numChnls; i++) {
		mGains[i] = 1.0;
		mMeters[i] = 0;
	}
}

void *OutputMaster::meterThreadFunc(void *arg) {
	int chanindex = 0;
	OutputMaster *om = static_cast<OutputMaster *>(arg);
	float meter_levels[om->mNumChnls];

	al::osc::Send s(om->mSendPort, om->mSendAddress.c_str());
	while(om->mRunMeterThread) {
		om->mMeterMutex.lock();
		std::unique_lock<std::mutex> lk(om->mMeterCondMutex);
  	om->mMeterCond.wait(lk);
		int bytes_read = om->mMeterBuffer.read((char *) meter_levels, om->mNumChnls * sizeof(float));
		if (bytes_read) {
			if (bytes_read !=  om->mNumChnls * sizeof(float)) {
				std::cerr << "Alloaudio: Warning. Meter values underrun." << std::endl;
			}
			for (int i = 0; i < bytes_read/sizeof(float); i++) {
				float value;
				if (om->mUseDb) {
					value = 20.0 * log10(meter_levels[i]);
				} else {
					value = meter_levels[i];
				}
				if (om->mMeterAddrHasChannel) {
					std::stringstream addr;
					addr << om->mAddressPrefix << "/meterdb/" <<  chanindex + 1;
					s.send(addr.str(), value);
				} else {
					s.send(om->mAddressPrefix + "/meterdb", chanindex, value);
				}

				chanindex++;
				if (chanindex == om->mNumChnls) {
					chanindex = 0;
				}
			}
		}
		om->mMeterMutex.unlock();
	}
	return NULL;
}

void OutputMaster::OSCHandler::onMessage(osc::Message &m)
{
	if (m.addressPattern() == outputmaster->mAddressPrefix + "/gain") {
		if (m.typeTags() == "if") {
			int chan;
			float gain;
			m >> chan >> gain;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster,
												&OutputMaster::setGainTimestamped,
												chan, (double) gain);
		} else {
			std::cerr << "Alloaudio: Wrong type tags for " << outputmaster->mAddressPrefix << "/gain message: "
					  << m.typeTags() << std::endl;
		}
	} else if (m.addressPattern() == outputmaster->mAddressPrefix + "/global_gain") {
		if (m.typeTags() == "f") {
			float gain;
			m >> gain;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster,
												&OutputMaster::setMasterGainTimestamped,
												(double) gain);
		} else {
			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->mAddressPrefix + "/global_gain message: "
					  << m.typeTags() << std::endl;
		}
	} else if (m.addressPattern() == outputmaster->mAddressPrefix + "/clipper_on") {
		if (m.typeTags() == "i") {
			int clipper_on;
			m >> clipper_on;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster,
												&OutputMaster::setClipperOnTimestamped,
												(bool) clipper_on != 0);
		} else if (m.typeTags() == "f") {
			float clipper_on;
			m >> clipper_on;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster,
												&OutputMaster::setClipperOnTimestamped,
												(bool) clipper_on != 0);
		} else {
			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->mAddressPrefix + "/clipper_on: "
					  << m.typeTags() << std::endl;
		}
	} else if (m.addressPattern() == outputmaster->mAddressPrefix + "/mute_all") {
		if (m.typeTags() == "i") {
			int mute_all;
			m >> mute_all;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster, &OutputMaster::setMuteAllTimestamped,
												(bool) mute_all != 0);
		} else if (m.typeTags() == "f") {
			float mute_all;
			m >> mute_all;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster, &OutputMaster::setMuteAllTimestamped,
												(bool) mute_all != 0);
		} else {
			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->mAddressPrefix + "/mute_all: "
					  << m.typeTags() << std::endl;
		}
	} else if (m.addressPattern() == outputmaster->mAddressPrefix + "/meter_on") {
		if (m.typeTags() == "i") {
			int on;
			m >> on;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster, &OutputMaster::setMeterOnTimestamped,
												(bool) on != 0);
		} else if (m.typeTags() == "f") {
			float on;
			m >> on;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster, &OutputMaster::setMeterOnTimestamped,
												(bool) on != 0);
		} else {
			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->mAddressPrefix + "/meter_update_freq: "
					 << m.typeTags() << std::endl;
		}
	} else if (m.addressPattern() == outputmaster->mAddressPrefix + "/meter_update_freq") {
		if (m.typeTags() == "f") {
			float freq;
			m >> freq;
			outputmaster->mParameterQueue.send(outputmaster->mParameterQueue.now(),
												outputmaster, &OutputMaster::setMeterupdateFreqTimestamped,
												(double) freq);
		} else {
			std::cerr << "Alloaudio: Wrong type tags for " + outputmaster->mAddressPrefix + "/meter_update_freq: "
					 << m.typeTags() << std::endl;
		}
	} else {
		std::cout << "Alloaudio: Unrecognized address pattern: " << m.addressPattern() << std::endl;
	}
}
