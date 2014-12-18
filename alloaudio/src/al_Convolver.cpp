#include <iostream>
#include <algorithm>
#include <zita-convolver.h>

#include <string.h>
#include <assert.h>
#include "alloaudio/al_Convolver.hpp"

using namespace al;

Convolver::Convolver() :
    m_Convproc(NULL)
{
}

int Convolver::configure(al::AudioIO &io, vector<float *> IRs,
                         int inputChannel, bool inputsAreBuses,
                         vector<int> disabledChannels, unsigned int maxsize,
                         unsigned int minpartition, unsigned int maxpartition, unsigned int options)
{
    assert(minpartition>=64);
    assert(maxpartition<=8192);
    //TODO check to make sure number of inputs matches buses/io inputs
    m_inputChannel = inputChannel;
    m_inputsAreBuses = inputsAreBuses;
    m_disabledChannels = disabledChannels;
    for (int i = 0; i < io.channels(true); i++) {
        if (std::find(disabledChannels.begin(), disabledChannels.end(), i)
                == disabledChannels.end()) {
            m_activeChannels.push_back(i);
        }
    }

    if(m_Convproc != NULL) {
        delete m_Convproc;
    }
    m_Convproc = new Convproc;
    m_Convproc->set_options(options);
    int bufferSize = io.framesPerBuffer(), nActiveOutputs = io.channels(true) - disabledChannels.size(), nActiveInputs,configResult;
    if(m_inputChannel < 0){//many to many
        nActiveInputs = io.channels(false);    }
    else{//one to many
        nActiveInputs = 1;
    }
    configResult = m_Convproc->configure(nActiveInputs, nActiveOutputs, maxsize, bufferSize, minpartition, maxpartition);
    if(configResult != 0){
        std::cout << "Config failed" << std::endl;
    }
    return 0;
}

int Convolver::processBlock(al::AudioIO &io)
{
    int blockSize = io.framesPerBuffer();
    //fill the input buffers
    if(m_inputChannel < 0){
        // many to many
        for(vector<int>::iterator it = m_activeChannels.begin();
            it != m_activeChannels.end(); ++it) {
            const float *inbuf;
            if (m_inputsAreBuses){
                inbuf = io.busBuffer(m_activeChannels[*it]);
            }
            else{
                inbuf = io.inBuffer(m_activeChannels[*it]);
            }
            std::copy(inbuf[0], inbuf[blockSize+1]), m_Convproc->inpdata(*it));
        }
    }
    else{
        //one to many
        const float *inbuf;
        if (m_inputsAreBuses){
            inbuf = io.busBuffer(m_inputChannel);
        }
        else{
            inbuf = io.inBuffer(m_inputChannel);
        }
        //TODO: figure out if passing a single input has the expected result
        /*for(vector<int>::iterator it = m_activeChannels.begin();
            it != m_activeChannels.end(); ++it) {
            memcpy(m_Convproc->inpdata(0), inbuf, sizeof(float) * blockSize);
        }*/
        memcpy(m_Convproc->inpdata(0), inbuf, sizeof(float) * blockSize);
    }
    //process
    int ret = m_Convproc->process();
    //fill the output buffers
    for(vector<int>::iterator it = m_activeChannels.begin();
        it != m_activeChannels.end(); ++it) {
        float *outbuf = io.outBuffer(m_activeChannels[*it]);
        memcpy(m_Convproc->outdata(*it), outbuf, sizeof(float) * blockSize);
    }
        
    //clear output for disabled channels
    m_Convproc->process(true);
    for(vector<int>::iterator it = m_disabledChannels.begin();
        it != m_disabledChannels.end(); ++it) {
        memset(io.outBuffer(*it), 0, blockSize * sizeof(float));
    }

	return ret;
}
