
#include <stdio.h>
//#include <iostream>

#include "alloaudio/al_OutputMaster.hpp"

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
    al::OutputMaster outmaster2(16, 44100);
    CU_ASSERT_EQUAL(outmaster2.getNumChnls(), 16);
    al::OutputMaster outmaster3(32, 44100);
    CU_ASSERT_EQUAL(outmaster3.getNumChnls(), 32);
}

void test_gains(void)
{
    al::OutputMaster outmaster(2, 44100);
    outmaster.setGlobalGain(0.56);
    outmaster.setGain(0, 1.0);
    outmaster.setGain(1, 1.1);
//    AudioIO(int framesPerBuf=64, double framesPerSec=44100.0,
//		void (* callback)(AudioIOData &) = 0, void * userData = 0,
//		int outChans = 2, int inChans = 0 )
    al::AudioIO io(4, 44100.0, 0, 0, 2, 2);
    float *in_0 = io.outBuffer(0);
    float *in_1 = io.outBuffer(1);
    for (int i = 0; i < 4; i++) {
        in_0[i] = 1.0/(i + 1);
        in_1[i] = i/4.0;
    }
    outmaster.processBlock(io);

    float *out_0 = io.outBuffer(0);
    float *out_1 = io.outBuffer(1);
    for (int i = 0; i < 4; i++) {
        CU_ASSERT_DOUBLE_EQUAL(out_0[i], 0.56/(i + 1), 0.000001);
        CU_ASSERT_DOUBLE_EQUAL(out_1[i], 1.1 * 0.56* i/4.0, 0.000001);
    }
}


void test_meter_values(void)
{
    al::OutputMaster outmaster(2, 44100);
    outmaster.setGlobalGain(1.0);
    outmaster.setGain(0, 1.0);
    outmaster.setGain(1, 1.0);
    outmaster.setMeterOn(true);
    outmaster.setMeterUpdateFreq(11025); // 4 samples
//    AudioIO(int framesPerBuf=64, double framesPerSec=44100.0,
//		void (* callback)(AudioIOData &) = 0, void * userData = 0,
//		int outChans = 2, int inChans = 0 )
    al::AudioIO io(4, 44100.0, 0, 0, 2, 2);
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
