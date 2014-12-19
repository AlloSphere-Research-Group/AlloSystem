
#include <stdio.h>
#include <vector>
#include "math.h"
#include <iostream>
#include <CUnit/Basic.h>

#include "alloaudio/al_Convolver.hpp"
#include "allocore/io/al_AudioIO.hpp"
#define IR_SIZE 1024
#define BLOCK_SIZE 16 //min 16, max 8192

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
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);

    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float));
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float));
    IR1[1] = 1.0f;IR1[2] = 0.25f;
	vector<float *> IRs;
	IRs.push_back(IR1);
    IRs.push_back(IR2);
    vector<int> IRLengths;
    IRLengths.push_back(IR_SIZE);
    IRLengths.push_back(IR_SIZE);

	int ret = conv.configure(io, IRs, IRLengths);
	CU_ASSERT(ret == 0);
	ret = conv.processBlock(io);
	CU_ASSERT(ret == 0);
}


void test_basic(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);
    io.channelsBus(2);

    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float)*IR_SIZE);
    //IR1[0] = 2.0f;//IR1[3] = 0.5f;
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float)*IR_SIZE);
    //IR2[1] = 2.0f;//IR2[2] = 0.25f;
    IR2[1] = 1.0f;IR2[2] = 0.25f;
	vector<float *> IRs;
	IRs.push_back(IR1);
    IRs.push_back(IR2);
    vector<int> IRLengths;
    IRLengths.push_back(IR_SIZE);
    IRLengths.push_back(IR_SIZE);
    
    float * busBuffer1 = io.busBuffer(0);
    memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer1[0] = 1.0f;
    float * busBuffer2 = io.busBuffer(1);
    memset(busBuffer2, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer2[0] = 1.0f;
    
    
    unsigned int maxsize = 2048, minpartition = 64, maxpartition = IR_SIZE;
    conv.configure(io, IRs, IRLengths, 0, true, vector<int>(), maxsize, minpartition, maxpartition);
	conv.processBlock(io);
    std::cout << endl;
	for(int i = 0; i < BLOCK_SIZE; i++) {
        std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
        std::cout << "Y2: " << io.out(0, i) << ", H2: " << IR2[i] << std::endl;
        CU_ASSERT(io.out(0, i) == IR1[i]);
        CU_ASSERT(io.out(1, i) == IR2[i]);
	}
}


void test_disabled_channels(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);

    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float));
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float));
    IR1[1] = 1.0f;IR1[2] = 0.25f;
	vector<float *> IRs;
	IRs.push_back(IR1);
	IRs.push_back(IR2);
    vector<int> IRLengths;
    IRLengths.push_back(IR_SIZE);
    IRLengths.push_back(IR_SIZE);

	vector<int> disabledOuts;

    int nOutputs = io.channels(true);
	unsigned int maxsize = 2048, minpartition = 64, maxpartition = IR_SIZE;
	conv.configure(io, IRs, IRLengths, -1, true, disabledOuts, maxsize, minpartition, maxpartition);
	conv.processBlock(io);
    
    std::vector<int>::iterator it;
    for(int i = 0; i < nOutputs; i++) {
        it = find (disabledOuts.begin(), disabledOuts.end(), i);
        if (it != disabledOuts.end()){
            CU_ASSERT(io.out(0, i) == 0.0f);
            CU_ASSERT(io.out(1, i) == 0.0f);
        }
    }
}

/*void test_vector_mode(void)
{
    al::Convolver conv;
    al::AudioIO io(BLOCK_SIZE, 44100.0);
    
    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float));
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float));
    IR1[1] = 1.0f;IR1[2] = 0.25f;
    vector<float *> IRs;
    IRs.push_back(IR1);
 IRs.push_back(IR2);
 vector<int> IRLengths;
 IRLengths.push_back(IR_SIZE);
 IRLengths.push_back(IR_SIZE);
 
    unsigned int maxsize = 2048, minpartition = 64, maxpartition = IR_SIZE;
    int ret = conv.configure(io, IRs, IRLenghts, -1, true, disabledOuts, maxsize, minpartition, maxpartition, 1);
    CU_ASSERT(ret == 0);
    ret = conv.processBlock(io);
    CU_ASSERT(ret == 0);
}*/


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
