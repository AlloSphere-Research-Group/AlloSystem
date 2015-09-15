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

int Convolver::configure(al::AudioIO &io, vector<float *> IRs, int IRlength,
						 int inputChannel, bool inputsAreBuses,
						 vector<int> disabledChannels, unsigned int basePartitionSize, unsigned int options)
{
	int bufferSize = io.framesPerBuffer(), nActiveOutputs = io.channels(true) - disabledChannels.size(),
			nActiveInputs, configResult;
	m_inputChannel = inputChannel;
	m_inputsAreBuses = inputsAreBuses;
	m_disabledChannels = disabledChannels;
	m_activeChannels.clear();
	for(int i = 0; i < io.channels(true); i++) {
		if (std::find(disabledChannels.begin(), disabledChannels.end(), i)
				== disabledChannels.end()) {
			m_activeChannels.push_back(i);
		}
	}
	assert(bufferSize >= Convproc::MINQUANT);
	assert(bufferSize <= Convproc::MAXQUANT);
	assert(IRlength <= MAXSIZE);
	assert(IRlength >= Convproc::MINPART);
	assert(nActiveOutputs > 0);
	assert(nActiveOutputs <= Convproc::MAXOUT);
	if(m_inputChannel < 0){//many to many
		if (m_inputsAreBuses) {
			assert(io.channelsBus() >= m_activeChannels.size());
		} else {
			assert(io.channels(false) >= m_activeChannels.size());
		}
		nActiveInputs = nActiveOutputs;
	}
	else{//one to many
		assert(io.channels(false) >= 1);
		nActiveInputs = 1;
	}
	assert(basePartitionSize >= Convproc::MINPART);
	assert(basePartitionSize <= Convproc::MAXPART);

	if(m_Convproc != NULL) {
		delete m_Convproc;
	}
	m_Convproc = new Convproc;
	m_Convproc->set_options(options);
	m_Convproc->set_density(0.0f);
	configResult = m_Convproc->configure(nActiveInputs, nActiveOutputs,
										 IRlength, bufferSize, basePartitionSize, (IRlength/2 < Convproc::MAXPART)?IRlength:Convproc::MAXPART);
	if(configResult != 0){
		std::cout << "Config failed" << std::endl;
	}
	//create IRs
	if(m_inputChannel < 0){//many to many
		for(int i = 0; i < nActiveOutputs; i++){
			m_Convproc->impdata_create(i, i, 1, IRs[i], 0, IRlength);
		}
	}
	else{//one to many
		for(int i = 0; i < nActiveOutputs; i++){
			m_Convproc->impdata_create(0, i, 1, IRs[i], 0, IRlength);
		}
	}
	m_Convproc->start_process(0, 0);
	return 0;
}

void Convolver::onAudioCB(al::AudioIOData &io)
{
	int blockSize = io.framesPerBuffer();

	//fill the input buffers
	if(m_inputChannel < 0){
		// many to many
		int i = 0;
		for(vector<int>::iterator it = m_activeChannels.begin();
			it != m_activeChannels.end(); ++it, ++i){
			const float *inbuf;
			float *dest = m_Convproc->inpdata(i);
			if (m_inputsAreBuses){
				inbuf = io.busBuffer(*it);
			}
			else{
				inbuf = io.inBuffer(*it);
			}
			memcpy(dest, inbuf, sizeof(float) * blockSize);
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
		memcpy(m_Convproc->inpdata(0), inbuf, sizeof(float) * blockSize);
	}

	//process
	int ret = m_Convproc->process(false);

	//fill the output buffers
	int i = 0;
	for(vector<int>::iterator it = m_activeChannels.begin();
		it != m_activeChannels.end(); ++it, ++i) {
		float *outbuf = io.outBuffer(*it);
		memcpy(outbuf, m_Convproc->outdata(i), sizeof(float) * blockSize);
	}

	//clear output for disabled channels
	for(vector<int>::iterator it = m_disabledChannels.begin();
		it != m_disabledChannels.end(); ++it) {
		memset(io.outBuffer(*it), 0, sizeof(float) * blockSize);
	}
}

int Convolver::shutdown(void){
	if(m_Convproc->stop_process()){
		cout << "Warning: could not stop process" << endl;
	}
	if(m_Convproc->cleanup()){
		cout << "Warning: cleanup failed" << endl;
	}
	return 0;
}

