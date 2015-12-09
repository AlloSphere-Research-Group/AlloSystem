
#include <cstdio>
#include <string>
#include <sstream>
//#include <iostream>

#include "alloaudio/al_OutputMaster.hpp"
#include "allocore/system/al_Time.hpp"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

TEST_CASE( "Class tests", "[alloaudio]" ) {
    al::OutputMaster outmaster(8, 44100);
	REQUIRE(outmaster.getNumChnls() == 8);
	REQUIRE(outmaster.background());
    al::OutputMaster outmaster2(16, 44100);
	REQUIRE(outmaster2.getNumChnls() == 16);
	al::OutputMaster outmaster3(32, 44100, "", -1);
	REQUIRE(outmaster3.getNumChnls() == 32);
}

TEST_CASE( "Gains", "[alloaudio]" ) {
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2, al::AudioIO::DUMMY);
	REQUIRE(io.channelsOut() == 2);
	REQUIRE(io.framesPerSecond() == 44100.0);
	al::OutputMaster outmaster(io.channelsOut(), io.framesPerSecond());
	outmaster.setClipperOn(false);
	io.append(outmaster);

    float *in_0 = io.outBuffer(0);
    float *in_1 = io.outBuffer(1);

	io.processAudio();

	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		REQUIRE(out_0[i] == 0.0f);
		REQUIRE(out_1[i] == 0.0f);
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
		REQUIRE(out_0[i] - 0.56/(i + 1)  < 1e-07f);
		REQUIRE(out_1[i] - 1.1 * 0.56* i/4.0 < 1e-07f);
    }
}

TEST_CASE( "Meter values", "[alloaudio]" ) {
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2, al::AudioIO::DUMMY);
	REQUIRE(io.channelsOut() == 2);
	REQUIRE(io.framesPerSecond() == 44100.0);
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

	REQUIRE(meterValues[0] == 0.5);
	REQUIRE(meterValues[1] == 0.75);
}

TEST_CASE( "Clipper", "[alloaudio]" ) {
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2, al::AudioIO::DUMMY);
	REQUIRE(io.channelsOut() == 2);
	REQUIRE(io.framesPerSecond() == 44100.0);
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

	REQUIRE(out_0[0] - 0.4 < 1e-07f);
	REQUIRE(out_1[0] - 0.3 < 1e-07f);
	for (int i = 1; i < 4; i++) {
		REQUIRE(out_0[i] - 0.5 < 1e-07f);
		REQUIRE(out_1[i] - 0.5 < 1e-07f);
	}
}

TEST_CASE( "OSC gain", "[alloaudio]" ) {
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2, al::AudioIO::DUMMY);
	REQUIRE(io.channelsOut() == 2);
	REQUIRE(io.framesPerSecond() == 44100.0f);
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
		REQUIRE(out_0[i] - 0.1 * 0.9 * 0.5 * (i + 1) < 1e-07f);
		REQUIRE(out_1[i] - 0.1 * 0.8 * 0.4 * (i + 1) < 1e-07f);
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
	REQUIRE(out_0[0] - 0.09 < 1e-07f);
	REQUIRE(out_1[0] - 0.08 < 1e-07f);
	for (int i = 1; i < 4; i++) {
		REQUIRE(out_0[i] - 0.1  < 1e-07f);
		REQUIRE(out_1[i] - 0.1  < 1e-07f);
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
		REQUIRE(out_0[i] == 0.0);
		REQUIRE(out_1[i] == 0.0);
	}
}

float meterValues[2] = {1.0f, 1.0f};
float meterValues2[2] = {1.0f, 1.0f};
struct OSCHandler : public al::osc::PacketHandler{
	void onMessage(al::osc::Message& m){
		std::string addressPattern = m.addressPattern();
		if(m.typeTags().size() == 1) {
			std::string patternPrefix = "/Alloaudio/meterdb/";
			REQUIRE(addressPattern.find(patternPrefix) == 0);
			REQUIRE(m.typeTags() == "f");
			std::stringstream convert(addressPattern.substr(patternPrefix.length()));
			int index = 0;
			if ( !(convert >> index) ) {
				index = -1;
			}
			REQUIRE(index >=0);
			float dbvalue;
			m >> dbvalue;
			meterValues[index - 1] = powf(10.0, dbvalue/20.0);
			meterValues2[index - 1] = 0.0;
		} else {
			REQUIRE(addressPattern == "/Alloaudio/meterdb");
			REQUIRE(m.typeTags().size() == 2);
			REQUIRE(m.typeTags() == "if");
			int index;
			m >> index;
			REQUIRE(index >=0);
			float dbvalue;
			m >> dbvalue;
			meterValues[index] = 0.0;
			meterValues2[index] = powf(10.0, dbvalue/20.0);
		}
	}
} handler;

TEST_CASE( "OSC Meters", "[alloaudio]" ) {
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2, al::AudioIO::DUMMY);
	REQUIRE(io.channelsOut() == 2);
	REQUIRE(io.framesPerSecond() == 44100.0f);
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
	REQUIRE(meterValues[0] == 0.0);
	REQUIRE(meterValues[1] == 0.0);
	REQUIRE(meterValues2[0] == 0.5);
	REQUIRE(meterValues2[1] == 0.75);

	for (int i = 0; i < 4; i++) {
		in_0[i] = i/4.0;
		in_1[i] = 1.0/(i + 2);
	}
	outmaster.setMeterAddrHasChannel(true);
	r.start();
	io.processAudio();
	al_sleep_nsec(1000000); // Wait for messages to arrive
	r.stop();
	REQUIRE(meterValues[0] == 0.75);
	REQUIRE(meterValues[1] == 0.5);
	REQUIRE(meterValues2[0] == 0.0);
	REQUIRE(meterValues2[1] == 0.0);
}
