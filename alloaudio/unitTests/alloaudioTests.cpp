
#include <cstdio>
#include <string>
#include <sstream>
//#include <iostream>

#include "alloaudio/al_OutputMaster.hpp"
#include "allocore/system/al_Time.hpp"

#include "CUnit/Basic.h"

int init_suite1(void)
{
    return 0;
}

int clean_suite1(void)
{
    return 0;
}

void test_class(void)
{
    al::OutputMaster outmaster(8, 44100);
    CU_ASSERT_EQUAL(outmaster.getNumChnls(), 8);
	CU_ASSERT(outmaster.background());
    al::OutputMaster outmaster2(16, 44100);
	CU_ASSERT_EQUAL(outmaster2.getNumChnls(), 16);
	al::OutputMaster outmaster3(32, 44100, "", -1);
	CU_ASSERT_EQUAL(outmaster3.getNumChnls(), 32);
}

void test_gains(void)
{
	al::OutputMaster outmaster(2, 44100);
	outmaster.setClipperOn(false);
//    AudioIO(int framesPerBuf=64, double framesPerSec=44100.0,
//		void (* callback)(AudioIOData &) = 0, void * userData = 0,
//		int outChans = 2, int inChans = 0 )
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
    float *in_0 = io.outBuffer(0);
    float *in_1 = io.outBuffer(1);

    outmaster.processBlock(io);

	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		CU_ASSERT_EQUAL(out_0[i], 0.0);
		CU_ASSERT_EQUAL(out_1[i], 0.0);
	}
	outmaster.setMasterGain(0.56);
	outmaster.setGain(0, 1.0);
	outmaster.setGain(1, 1.1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 1.0/(i + 1);
		in_1[i] = i/4.0;
	}
	outmaster.processBlock(io);

	out_0 = io.outBuffer(0);
	out_1 = io.outBuffer(1);
    for (int i = 0; i < 4; i++) {
        CU_ASSERT_DOUBLE_EQUAL(out_0[i], 0.56/(i + 1), 0.000001);
        CU_ASSERT_DOUBLE_EQUAL(out_1[i], 1.1 * 0.56* i/4.0, 0.000001);
    }
}

void test_meter_values(void)
{
    al::OutputMaster outmaster(2, 44100);
	outmaster.setMasterGain(1.0);
    outmaster.setGain(0, 1.0);
    outmaster.setGain(1, 1.0);
    outmaster.setMeterOn(true);
	outmaster.setClipperOn(false);
	outmaster.setMeterUpdateFreq(11025); // 4 samples
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
    float *in_0 = io.outBuffer(0);
    float *in_1 = io.outBuffer(1);
    for (int i = 0; i < 4; i++) {
        in_0[i] = 1.0/(i + 2);
        in_1[i] = i/4.0;
    }
    outmaster.processBlock(io);

    float meterValues[2];
    outmaster.getMeterValues(meterValues);

    CU_ASSERT_DOUBLE_EQUAL(meterValues[0], 0.5, 0.000001);
    CU_ASSERT_DOUBLE_EQUAL(meterValues[1], 0.75, 0.000001);
}

void test_clipper(void)
{
	al::OutputMaster outmaster(2, 44100, "localhost", 9001);
	outmaster.setClipperOn(true);
	outmaster.setMasterGain(0.5);
	outmaster.setGain(0, 1.0);
	outmaster.setGain(1, 1.0);

	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	float *in_0 = io.outBuffer(0);
	float *in_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 0.8 * (i + 1);
		in_1[i] = 0.6 * (i + 1);
	}
	outmaster.processBlock(io);

	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);

	CU_ASSERT_DOUBLE_EQUAL(out_0[0], 0.4, 0.000001);
	CU_ASSERT_DOUBLE_EQUAL(out_1[0], 0.3, 0.000001);
	for (int i = 1; i < 4; i++) {
		CU_ASSERT_DOUBLE_EQUAL(out_0[i], 0.5, 0.000001);
		CU_ASSERT_DOUBLE_EQUAL(out_1[i], 0.5, 0.000001);
	}
}

void test_osc_gain(void)
{
	al::OutputMaster outmaster(2, 44100, "localhost", 9001);
	outmaster.setClipperOn(false);
	al::osc::Send s(9001, "localhost");
	s.send("/Alloaudio/global_gain", 0.1f);
	s.send("/Alloaudio/gain", 0, 0.9f);
	s.send("/Alloaudio/gain", 1, 0.8f);
	s.send("/Alloaudio/gain", 2, 0.7f); // Should do nothing
	al_sleep_nsec(100000); // Wait for messages to arrive
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
	float *in_0 = io.outBuffer(0);
	float *in_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		in_0[i] = 0.5 * (i + 1);
		in_1[i] = 0.4 * (i + 1);
	}
	outmaster.processBlock(io);
	float *out_0 = io.outBuffer(0);
	float *out_1 = io.outBuffer(1);
	for (int i = 0; i < 4; i++) {
		CU_ASSERT_DOUBLE_EQUAL(out_0[i], 0.1 * 0.9 * 0.5 * (i + 1), 0.000001);
		CU_ASSERT_DOUBLE_EQUAL(out_1[i], 0.1 * 0.8 * 0.4 * (i + 1), 0.000001);
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
	outmaster.processBlock(io);
	CU_ASSERT_DOUBLE_EQUAL(out_0[0], 0.09, 0.00001);
	CU_ASSERT_DOUBLE_EQUAL(out_1[0], 0.08, 0.00001);
	for (int i = 1; i < 4; i++) {
		CU_ASSERT_DOUBLE_EQUAL(out_0[i], 0.1, 0.00001);
		CU_ASSERT_DOUBLE_EQUAL(out_1[i], 0.1, 0.00001);
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
	outmaster.processBlock(io);
	for (int i = 0; i < 4; i++) {
		CU_ASSERT_DOUBLE_EQUAL(out_0[i], 0.0, 0.000001);
		CU_ASSERT_DOUBLE_EQUAL(out_1[i], 0.0, 0.000001);
	}
}

float meterValues[2] = {1.0f, 1.0f};
struct OSCHandler : public al::osc::PacketHandler{
	void onMessage(al::osc::Message& m){
		std::string addressPattern = m.addressPattern();
		std::string patternPrefix = "/Alloaudio/meterdb/";
		CU_ASSERT(addressPattern.find(patternPrefix) == 0);
		CU_ASSERT(m.typeTags().size() == 1);
		CU_ASSERT(m.typeTags().at(0) == 'f');
		std::stringstream convert(addressPattern.substr(patternPrefix.length()));
		int index = 0;
		if ( !(convert >> index) ) {
			index = -1;
		}
		CU_ASSERT(index >=0);
		float dbvalue;
		m >> dbvalue;
		meterValues[index - 1] = powf(10.0, dbvalue/20.0);
	}
} handler;

void test_osc_meters()
{
	al::OutputMaster outmaster(2, 44100, "localhost", 9001, "localhost", 9002);
	outmaster.setClipperOn(false);
	al::osc::Send s(9001, "localhost");
	s.send("/Alloaudio/meter_on", 1);
	s.send("/Alloaudio/meter_update_freq", 11025.0f);
	s.send("/Alloaudio/global_gain", 1.0f);
	s.send("/Alloaudio/gain", 0, 1.0f);
	s.send("/Alloaudio/gain", 1, 1.0f);

	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2);
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

	outmaster.processBlock(io);
	al_sleep_nsec(1000000); // Wait for messages to arrive
	r.stop();

	CU_ASSERT_DOUBLE_EQUAL(meterValues[0], 0.5, 0.000001);
	CU_ASSERT_DOUBLE_EQUAL(meterValues[1], 0.75, 0.000001);

}

void test_osc_other()
{
	//    lo_server_thread_add_method(od->st, "/Alloaudio/room_compensation_on", "i", room_compensation_handler, pp);
	//    lo_server_thread_add_method(od->st, "/Alloaudio/bass_management_freq", "f", bass_management_handler, pp);
	//    lo_server_thread_add_method(od->st, "/Alloaudio/bass_management_mode", "i", bass_management_mode_handler, pp);
	//    lo_server_thread_add_method(od->st, "/Alloaudio/sw_indeces", "iiii", sw_indeces_handler, pp);
}

int main()
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("alloaudio tests", init_suite1, clean_suite1);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ( (NULL == CU_add_test(pSuite, "Test Class", test_class))
         ||  (NULL == CU_add_test(pSuite, "Test Gains", test_gains))
         ||  (NULL == CU_add_test(pSuite, "Test Meter Values", test_meter_values))
		 ||  (NULL == CU_add_test(pSuite, "Test Clipper", test_clipper))
		 ||  (NULL == CU_add_test(pSuite, "Test OSC Gain control", test_osc_gain))
		 ||  (NULL == CU_add_test(pSuite, "Test OSC Meters", test_osc_meters))
		 ||  (NULL == CU_add_test(pSuite, "Test OSC Other messages", test_osc_other))
         )
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
