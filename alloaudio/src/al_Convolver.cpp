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
                         int input, bool inputsAreBuses,
                         vector<int> disabledChannels, unsigned int maxsize,
                         unsigned int minpartition, unsigned int maxpartition)
{
    assert(minpartition>=64);
    assert(maxpartition<=8192);
    //assert()
    //TODO check to make sure number of inputs matches buses/io inputs
    m_input = input;
    m_inputsAreBuses = inputsAreBuses;
    m_disabledChannels = disabledChannels;
    for (int i = 0; i < io.channels(true); i++) {
        if (std::find(disabledChannels.begin(), disabledChannels.end(), i)
                != disabledChannels.end()) {
            m_activeChannels.push_back(i);
        }
    }

    if(m_Convproc != NULL) {
        delete m_Convproc;
    }
    m_Convproc = new Convproc;
    // TODO configure Convproc
    int bufferSize = io.framesPerBuffer(), configResult;
    if(m_input < 0){//many to many
        configResult = m_Convproc->configure(io.channels(false), io.channels(true)-disabledChannels.size(), maxsize, bufferSize, minpartition, maxpartition);
    }
    else{//one to many
        configResult = m_Convproc->configure(1, io.channels(true)-disabledChannels.size(), maxsize, bufferSize, minpartition, maxpartition);
    }
    if(configResult != 0){
        std::cout << "Config failed" << std::endl;
    }
    return 0;
}

int Convolver::processBlock(al::AudioIO &io)
{
    int framesBuffer = io.framesPerBuffer();

    // TODO Output from disabled channels is set to 0.
    // many to many
    if(m_input < 0){
        for(vector<int>::iterator it = m_activeChannels.begin();
            it != m_activeChannels.end(); ++it) {
            const float *inbuf;
            if (m_input < 0) {
                if (m_inputsAreBuses){
                    inbuf = io.busBuffer(*it);
                }
                else{
                    inbuf = io.inBuffer(*it);
                }
            } else {
                 // TODO implement the one to many case
                if (m_inputsAreBuses){
                    inbuf = io.busBuffer(m_input);
                }
                else{
                    inbuf = io.inBuffer(m_input);
                }

            }
            memcpy(m_Convproc->inpdata(*it), inbuf, sizeof(float) * framesBuffer);

            float *outbuf = io.outBuffer(*it);
            // TODO process Convproc
        }
    }
    else{
        //one to many
        // TODO Output from disabled channels is set to 0.
        const float *inbuf;
        if (m_inputsAreBuses){
            inbuf = io.busBuffer(m_input);
        }
        else{
            inbuf = io.inBuffer(m_input);
        }
        //for(vector<int>::iterator it = m_activeChannels.begin();
        //    it != m_activeChannels.end(); ++it) {
        float * inpdata = m_Convproc->inpdata(0);
        memcpy(m_Convproc->inpdata(0), inbuf, sizeof(float) * framesBuffer);
    }

    m_Convproc->process(true);
    for(vector<int>::iterator it = m_disabledChannels.begin();
        it != m_disabledChannels.end(); ++it) {
        memset(io.outBuffer(*it), 0, framesBuffer * sizeof(float));
    }

	return 0;
}
