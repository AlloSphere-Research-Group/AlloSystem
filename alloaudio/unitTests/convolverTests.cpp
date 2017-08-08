
#include <cstdio>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "alloaudio/al_Convolver.hpp"
#include "allocore/io/al_AudioIO.hpp"

#define IR_SIZE 1024
#define BLOCK_SIZE 64 //min 64, max 8192

using namespace std;


void ut_class_construction(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	io.append(conv);

    //create dummy IRs
    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float)*IR_SIZE);
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float)*IR_SIZE);
    IR2[1] = 1.0f;IR2[2] = 0.25f;
    vector<float *> IRs;
    IRs.push_back(IR1);
    IRs.push_back(IR2);
    int IRlength = IR_SIZE;
    
	int ret = conv.configure(io, IRs, IRlength);
	assert(ret == 0);
	io.processAudio();
    conv.shutdown();
}


void ut_many_to_many(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	io.append(conv);
    io.channelsBus(2);

    //create dummy IRs
    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float)*IR_SIZE);
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float)*IR_SIZE);
    IR2[1] = 1.0f;IR2[2] = 0.25f;
    vector<float *> IRs;
    IRs.push_back(IR1);
    IRs.push_back(IR2);
    int IRlength = IR_SIZE;
    
    //create dummy input buffers
    float * busBuffer1 = io.busBuffer(0);
    memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer1[0] = 1.0f;
    float * busBuffer2 = io.busBuffer(1);
    memset(busBuffer2, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer2[0] = 1.0f;
    
	unsigned int basePartitionSize = BLOCK_SIZE, options = 1;
    options = 1; //FFTW MEASURE
    //many to many mode
    conv.configure(io, IRs, IRlength, -1, true, vector<int>(), basePartitionSize, options);
	io.processAudio();

	for(int i = 0; i < BLOCK_SIZE; i++) {
        //std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
        //std::cout << "Y2: " << io.out(1, i) << ", H2: " << IR2[i] << std::endl;
		assert(fabs(io.out(0, i) - IR1[i]) < 1e-07f);
		assert(fabs(io.out(1, i) - IR2[i]) < 1e-07f);
	}
    conv.shutdown();
}

void ut_one_to_many(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	io.append(conv);
    io.channelsBus(1);
    
    //create dummy IRs
    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float)*IR_SIZE);
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float)*IR_SIZE);
    IR2[1] = 1.0f;IR2[2] = 0.25f;
    vector<float *> IRs;
    IRs.push_back(IR1);
    IRs.push_back(IR2);
    int IRlength = IR_SIZE;
    
    //create dummy input buffers
    float * busBuffer1 = io.busBuffer(0);
    memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer1[0] = 1.0f;
    
    unsigned int basePartitionSize = BLOCK_SIZE, options = 1;
    options = 1; //FFTW MEASURE
    //one to many mode
    conv.configure(io, IRs, IRlength, 0, true, vector<int>(), basePartitionSize, options);
	io.processAudio();

    for(int i = 0; i < BLOCK_SIZE; i++) {
        //std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
        //std::cout << "Y2: " << io.out(1, i) << ", H2: " << IR2[i] << std::endl;
		assert(fabs(io.out(0, i) - IR1[i]) < 1e-07f);
		assert(fabs(io.out(1, i) - IR2[i]) < 1e-07f);
    }
    conv.shutdown();
}


void ut_disabled_channels(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	io.append(conv);

    //create dummy IRs
    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float)*IR_SIZE);
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float)*IR_SIZE);
    IR2[1] = 1.0f;IR2[2] = 0.25f;
    vector<float *> IRs;
    IRs.push_back(IR1);
    IRs.push_back(IR2);
    int IRlength = IR_SIZE;

	vector<int> disabledOuts;

    int nOutputs = io.channels(true);
	unsigned int basePartitionSize = BLOCK_SIZE, options = 0;
    conv.configure(io, IRs, IRlength, -1, true, disabledOuts, basePartitionSize, options);
	io.processAudio();
    
    std::vector<int>::iterator it;
    for(int i = 0; i < nOutputs; i++) {
        it = find (disabledOuts.begin(), disabledOuts.end(), i);
        if (it != disabledOuts.end()){
			assert(io.out(0, i) == 0.0f);
			assert(io.out(1, i) == 0.0f);
        }
    }
    conv.shutdown();
}

void ut_vector_mode(void)
{
	al::Convolver conv;
	al::AudioIO io(BLOCK_SIZE, 44100.0, NULL, NULL, 2, 2);
	assert(io.channelsOut() == 2);
	assert(io.framesPerSecond() == 44100.0f);
	io.append(conv);
    io.channelsBus(2);
    
    //create dummy IRs
    float IR1[IR_SIZE];
    memset(IR1, 0, sizeof(float)*IR_SIZE);
    IR1[0] = 1.0f;IR1[3] = 0.5f;
    float IR2[IR_SIZE];
    memset(IR2, 0, sizeof(float)*IR_SIZE);
    IR2[1] = 1.0f;IR2[2] = 0.25f;
    vector<float *> IRs;
    IRs.push_back(IR1);
    IRs.push_back(IR2);
    int IRlength = IR_SIZE;
    
    //create dummy input buffers
    float * busBuffer1 = io.busBuffer(0);
    memset(busBuffer1, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer1[0] = 1.0f;
    float * busBuffer2 = io.busBuffer(1);
    memset(busBuffer2, 0, sizeof(float) * BLOCK_SIZE);
    busBuffer2[0] = 1.0f;
    
    unsigned int basePartitionSize = BLOCK_SIZE, options = 1;
    options |= 2; //vector mode
    conv.configure(io, IRs, IRlength, -1, true, vector<int>(), basePartitionSize, options);
	io.processAudio();
    for(int i = 0; i < BLOCK_SIZE; i++) {
        //std::cout << "Y1: " << io.out(0, i) << ", H1: " << IR1[i] << std::endl;
        //std::cout << "Y2: " << io.out(1, i) << ", H2: " << IR2[i] << std::endl;
		assert(fabs(io.out(0, i) - IR1[i]) < 1e-07f);
		assert(fabs(io.out(1, i) - IR2[i]) < 1e-07f);
    }
    conv.shutdown();
}

#define RUNTEST(Name)\
	printf("%s ", #Name);\
	ut_##Name();\
	for(size_t i=0; i<32-strlen(#Name); ++i) printf(".");\
	printf(" pass\n")

int main()
{
	RUNTEST(class_construction);
	RUNTEST(many_to_many);
	RUNTEST(one_to_many);
	RUNTEST(disabled_channels);
	RUNTEST(vector_mode);
	return 0;
}
