
#include <stdio.h>
#include <vector>
#include <CUnit/Basic.h>

#include "alloaudio/al_Convolver.hpp"
#include "allocore/io/al_AudioIO.hpp"
#define IR_SIZE 1024

using namespace std;

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
	al::Convolver conv;
	al::AudioIO io(16, 44100.0);

    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float));
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float));
    IR1[1] = 1.0f;IR1[2] = 0.25f;
	vector<float *> IRs;
	IRs.push_back(IR1);
	IRs.push_back(IR2);

	int ret = conv.configure(io, IRs);
	CU_ASSERT(ret == 0);
	ret = conv.processBlock(io);
	CU_ASSERT(ret == 0);
}


void test_basic(void)
{
	al::Convolver conv;
	al::AudioIO io(16, 44100.0, NULL, NULL, 2, 2);
	io.channelsBus(1);

    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float));
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float));
    IR1[1] = 1.0f;IR1[2] = 0.25f;
	vector<float *> IRs;
	IRs.push_back(IR1);
	IRs.push_back(IR2);
	io.bus(0, 0) = 1.0;
	io.bus(0, 1) = 0.0;
	io.bus(0, 2) = 0.0;
	io.bus(0, 3) = 0.0;

	conv.configure(io, IRs, 0, true);
	conv.processBlock(io);

	for(int i = 0; i < 16; i++) {
		CU_ASSERT(io.out(0, i) == IR1[i]);
		CU_ASSERT(io.out(1, i) == IR2[i]);
	}
}


void test_disabled_channels(void)
{
	al::Convolver conv;
	al::AudioIO io(16, 44100.0, NULL, NULL, 2, 2);

    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float));
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float));
    IR1[1] = 1.0f;IR1[2] = 0.25f;
	vector<float *> IRs;
	IRs.push_back(IR1);
	IRs.push_back(IR2);

	vector<int> disabledOuts;


	unsigned int maxsize = 2048, minpartition = 64, maxpartition = IR_SIZE;
	conv.configure(io, IRs, -1, true, disabledOuts, maxsize, minpartition, maxpartition);
	conv.processBlock(io);
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
		 ||  (NULL == CU_add_test(pSuite, "Test Basic Convolution", test_basic))
		 ||  (NULL == CU_add_test(pSuite, "Test Disabled Channels", test_disabled_channels))
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
