#include <iostream>
#include <algorithm>
#include <zita-convolver.h>

#include <string.h>

#include "alloaudio/al_Convolver.hpp"

using namespace al;

Convolver::Convolver() :
    m_Convproc(NULL)
{
}

int Convolver::configure(al::AudioIO &io, vector<float *> IRs,
                         vector<int> inputs, bool inputsAreBuses,
                         vector<int> disabledChannels, unsigned int maxsize,
                         unsigned int minpartition, unsigned int maxpartition)
{
    m_inputs = inputs;
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
	return 0;
}

int Convolver::processBlock(al::AudioIO &io)
{
    int framesBuffer = io.framesPerBuffer();

    // TODO Output from disabled channels is set to 0.
    for(vector<int>::iterator it = m_activeChannels.begin();
        it != m_activeChannels.end(); ++it) {
        const float *inbuf;
        if (m_inputs.size() == 0) {
            inbuf = io.inBuffer(*it);
        } else {
             // TODO implement the one to many case
        }
        float *outbuf = io.outBuffer(*it);
        // TODO process Convproc
    }

    for(vector<int>::iterator it = m_disabledChannels.begin();
        it != m_disabledChannels.end(); ++it) {
        memset(io.outBuffer(*it), 0, framesBuffer * sizeof(float));
    }

	return 0;
}
