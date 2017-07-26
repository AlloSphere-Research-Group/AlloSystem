
#include <cstdio>
#include <string>
#include <sstream>
#include <cassert>
//#include <iostream>

#include "alloaudio/al_OutputMaster.hpp"
#include "allocore/system/al_Time.hpp"



void ut_class_test(void)
{
    al::OutputMaster outmaster(8, 44100);
	assert(outmaster.getNumChnls() == 8);
	assert(outmaster.background());
    al::OutputMaster outmaster2(16, 44100);
	assert(outmaster2.getNumChnls() == 16);
	al::OutputMaster outmaster3(32, 44100, "", -1);
	assert(outmaster3.getNumChnls() == 32);
}

void ut_gains(void)
{
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0);
	al::OutputMaster outmaster(io.channelsOut(), io.framesPerSecond());
	outmaster.setClipperOn(false);
	io.append(outmaster);

    float *in_0 = io.outBuffer(0);
    float *in_1 = io.outBuffer(1);

	io.processAudio();

	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		assert(out_0[i] == 0.0f);
		assert(out_1[i] == 0.0f);
	}
	outmaster.setMasterGain(0.56);
	outmaster.setGain(0, 1.0);
	outmaster.setGain(1, 1.1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 1.0/(i + 1);
		in_1[i] = i/4.0;
	}
	io.processAudio();

	out_0 = io.outBuffer(0);
	out_1 = io.outBuffer(1);
    for (int i = 0; i < 4; i++) {
		assert(out_0[i] - 0.56/(i + 1)  < 1e-07f);
		assert(out_1[i] - 1.1 * 0.56* i/4.0 < 1e-07f);
    }
}

void ut_meter_values(void)
{
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0);
	al::OutputMaster outmaster(io.channelsOut(), io.framesPerSecond());
	io.append(outmaster);

	outmaster.setMasterGain(1.0);
    outmaster.setGain(0, 1.0);
    outmaster.setGain(1, 1.0);
    outmaster.setMeterOn(true);
	outmaster.setClipperOn(false);
	outmaster.setMeterUpdateFreq(11025); // 4 samples

    float *in_0 = io.outBuffer(0);
    float *in_1 = io.outBuffer(1);
    for (int i = 0; i < 4; i++) {
        in_0[i] = 1.0/(i + 2);
        in_1[i] = i/4.0;
    }
	io.processAudio();

    float meterValues[2];
    outmaster.getMeterValues(meterValues);

	assert(meterValues[0] == 0.5);
	assert(meterValues[1] == 0.75);
}

void ut_clipper(void)
{
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0);
	al::OutputMaster outmaster(io.channelsOut(), io.framesPerSecond());
	io.append(outmaster);

	outmaster.setClipperOn(true);
	outmaster.setMasterGain(0.5);
	outmaster.setGain(0, 1.0);
	outmaster.setGain(1, 1.0);

	float *in_0 = io.outBuffer(0);
	float *in_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 0.8 * (i + 1);
		in_1[i] = 0.6 * (i + 1);
	}
	io.processAudio();

	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);

	assert(out_0[0] - 0.4 < 1e-07f);
	assert(out_1[0] - 0.3 < 1e-07f);
	for (int i = 1; i < 4; i++) {
		assert(out_0[i] - 0.5 < 1e-07f);
		assert(out_1[i] - 0.5 < 1e-07f);
	}
}

void ut_osc_gain(void)
{
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	al::OutputMaster outmaster(io.channelsOut(), io.framesPerSecond(), "localhost", 9001);
	io.append(outmaster);
	outmaster.setClipperOn(false);

	al::osc::Send s(9001, "localhost");
	s.send("/Alloaudio/global_gain", 0.1f);
	s.send("/Alloaudio/gain", 0, 0.9f);
	s.send("/Alloaudio/gain", 1, 0.8f);
	s.send("/Alloaudio/gain", 2, 0.7f); // Should do nothing
	al_sleep_nsec(100000); // Wait for messages to arrive

	float *in_0 = io.outBuffer(0);
	float *in_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 0.5 * (i + 1);
		in_1[i] = 0.4 * (i + 1);
	}
	io.processAudio();
	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		assert(out_0[i] - 0.1 * 0.9 * 0.5 * (i + 1) < 1e-07f);
		assert(out_1[i] - 0.1 * 0.8 * 0.4 * (i + 1) < 1e-07f);
	}

	s.send("/Alloaudio/clipper_on", 1);
	outmaster.setMasterGain(0.1);
	outmaster.setGain(0, 0.9);
	outmaster.setGain(1, 0.8);
	outmaster.setClipperOn(false);
	al_sleep_nsec(100000); // Wait for messages to arrive
	for (int i = 0; i < 4; i++) {
		in_0[i] = 1.0 * (i + 1);
		in_1[i] = 1.0 * (i + 1);
	}
	io.processAudio();
	assert(out_0[0] - 0.09 < 1e-07f);
	assert(out_1[0] - 0.08 < 1e-07f);
	for (int i = 1; i < 4; i++) {
		assert(out_0[i] - 0.1  < 1e-07f);
		assert(out_1[i] - 0.1  < 1e-07f);
	}

	outmaster.setClipperOn(false);
	outmaster.setMasterGain(1.0);
	outmaster.setGain(0, 1.0);
	outmaster.setGain(1, 1.0);
	s.send("/Alloaudio/mute_all", 1);
	al_sleep_nsec(100000); // Wait for messages to arrive
	for (int i = 0; i < 4; i++) {
		in_0[i] = 1.0 * (i + 1);
		in_1[i] = 1.0 * (i + 1);
	}
	io.processAudio();
	for (int i = 0; i < 4; i++) {
		assert(out_0[i] == 0.0);
		assert(out_1[i] == 0.0);
	}
}

float meterValues[2] = {1.0f, 1.0f};
float meterValues2[2] = {1.0f, 1.0f};
struct OSCHandler : public al::osc::PacketHandler{
	void onMessage(al::osc::Message& m){
		std::string addressPattern = m.addressPattern();
		if(m.typeTags().size() == 1) {
			std::string patternPrefix = "/Alloaudio/meterdb/";
			assert(addressPattern.find(patternPrefix) == 0);
			assert(m.typeTags() == "f");
			std::stringstream convert(addressPattern.substr(patternPrefix.length()));
			int index = 0;
			if ( !(convert >> index) ) {
				index = -1;
			}
			assert(index >=0);
			float dbvalue;
			m >> dbvalue;
			meterValues[index - 1] = powf(10.0, dbvalue/20.0);
			meterValues2[index - 1] = 0.0;
		} else {
			assert(addressPattern == "/Alloaudio/meterdb");
			assert(m.typeTags().size() == 2);
			assert(m.typeTags() == "if");
			int index;
			m >> index;
			assert(index >=0);
			float dbvalue;
			m >> dbvalue;
			meterValues[index] = 0.0;
			meterValues2[index] = powf(10.0, dbvalue/20.0);
		}
	}
} handler;

void ut_osc_meters()
{
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	al::OutputMaster outmaster(io.channelsOut(), io.framesPerSecond(), "localhost", 9001, "localhost", 9002);
	io.append(outmaster);
	outmaster.setClipperOn(false);

	al::osc::Send s(9001, "localhost");
	s.send("/Alloaudio/meter_on", 1);
	s.send("/Alloaudio/meter_update_freq", 11025.0f);
	s.send("/Alloaudio/global_gain", 1.0f);
	s.send("/Alloaudio/gain", 0, 1.0f);
	s.send("/Alloaudio/gain", 1, 1.0f);

	float *in_0 = io.outBuffer(0);
	float *in_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 1.0/(i + 2);
		in_1[i] = i/4.0;
	}

	al::osc::Recv r(9002);
	r.handler(handler);
	r.timeout(0.1);
	r.start();

	io.processAudio();
	al_sleep_nsec(1000000); // Wait for messages to arrive
	r.stop();
	assert(meterValues[0] == 0.0);
	assert(meterValues[1] == 0.0);
	assert(meterValues2[0] == 0.5);
	assert(meterValues2[1] == 0.75);

	for (int i = 0; i < 4; i++) {
		in_0[i] = i/4.0;
		in_1[i] = 1.0/(i + 2);
	}
	outmaster.setMeterAddrHasChannel(true);
	r.start();
	io.processAudio();
	al_sleep_nsec(1000000); // Wait for messages to arrive
	r.stop();
	assert(meterValues[0] == 0.75);
	assert(meterValues[1] == 0.5);
	assert(meterValues2[0] == 0.0);
	assert(meterValues2[1] == 0.0);
}


#define RUNTEST(Name)\
	printf("%s ", #Name);\
	ut_##Name();\
	for(size_t i=0; i<32-strlen(#Name); ++i) printf(".");\
	printf(" pass\n")

int main()
{
	RUNTEST(class_test);
	RUNTEST(gains);
	RUNTEST(meter_values);
	RUNTEST(clipper);
	RUNTEST(osc_gain);
	RUNTEST(osc_meters);

	return 0;
}
