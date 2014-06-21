
#include "alloaudio/al_OutputMaster.hpp"

#include "src/butter.c"

//#include "firfilter.h"

using namespace al;



OutputMaster::OutputMaster(int num_chnls, double sampleRate):
	m_meterBuffer(1024), m_framesPerSec(sampleRate)
{
	pthread_mutex_init(&m_paramMutex, NULL);

	allocateChannels(num_chnls);

	initializeData();
}

// FIXME fix leaks  and issues when changing channel numbers (due to C pointers)

OutputMaster::~OutputMaster()
{
	/* FIXME close ports and deallocate filters before clearing memory */
	for (int i = 0; i < m_numChnls; i++) {
		butter_free(m_lopass1[i]);
		butter_free(m_lopass2[i]);
		butter_free(m_hipass1[i]);
		butter_free(m_hipass2[i]);
	}
	//    free(filters); //FIR filters
}

void OutputMaster::setFilters(double **irs, int filter_len)
{
	if (!irs) { /* if NULL, leave filtering off */
		m_filtersActive = false;
		return;
	}
	for (int i = 0; i < m_numChnls; i++) {
		//        FIRFILTER *new_filter = firfilter_create(irs[i], filter_len);
		//        FIRFILTER *old_filter = pp->filters[i];
		//        pp->filters[i] = new_filter;
		//        if (old_filter) {
		//            firfilter_free(old_filter);
		//        }
	}

	//    filtersActive = true;
}

// FIXME adjust parameter passing model. Should only block when passing the filter irs
void OutputMaster::setGlobalGain(double gain)
{
	pthread_mutex_lock(&m_paramMutex);
	m_masterGain = gain;
	pthread_mutex_unlock(&m_paramMutex);
}

void OutputMaster::setGain(int channelIndex, double gain)
{
	pthread_mutex_lock(&m_paramMutex);
	if (channelIndex >= 0 && channelIndex < m_numChnls) {
		m_gains[channelIndex] = gain;
	} else {
		//        printf("Alloaudio error: set_gain() for invalid channel %i", channelIndex);
	}
	pthread_mutex_unlock(&m_paramMutex);

}

void OutputMaster::setMuteAll(bool muteAll)
{
	pthread_mutex_lock(&m_paramMutex);
	m_muteAll = muteAll;
	pthread_mutex_unlock(&m_paramMutex);

}

void OutputMaster::setClipperOn(bool clipperOn)
{
	pthread_mutex_lock(&m_paramMutex);
	m_clipperOn = clipperOn;
	pthread_mutex_unlock(&m_paramMutex);
}

void OutputMaster::setRoomCompensationOn(bool on)
{
	pthread_mutex_lock(&m_paramMutex);
	m_filtersActive = on;
	pthread_mutex_unlock(&m_paramMutex);

}

void OutputMaster::setMeterUpdateFreq(double freq)
{
	m_meterUpdateSamples = (int)(m_framesPerSec/freq);
}

void OutputMaster::setBassManagementFreq(double frequency)
{
	int i;
	if (frequency > 0) {
		pthread_mutex_lock(&m_paramMutex);
		for (i = 0; i < m_numChnls; i++) {
			butter_set_fc(m_lopass1[i], frequency);
			butter_set_fc(m_lopass2[i], frequency);
			butter_set_fc(m_hipass1[i], frequency);
			butter_set_fc(m_hipass2[i], frequency);
		}
		pthread_mutex_unlock(&m_paramMutex);
	}
}

void OutputMaster::setBassManagementMode(bass_mgmt_mode_t mode)
{
	pthread_mutex_lock(&m_paramMutex);
	m_BassManagementMode = mode;
	pthread_mutex_unlock(&m_paramMutex);
}

void OutputMaster::setSwIndeces(int i1, int i2, int i3, int i4)
{
	pthread_mutex_lock(&m_paramMutex);
	swIndex[0] = i1;
	swIndex[1] = i1;
	swIndex[2] = i1;
	swIndex[3] = i1;
	pthread_mutex_unlock(&m_paramMutex);
}

void OutputMaster::setMeterOn(bool meterOn)
{
	pthread_mutex_lock(&m_paramMutex);
	m_meterOn = meterOn;
	pthread_mutex_unlock(&m_paramMutex);

}

int OutputMaster::getMeterValues(float *values)
{
	return m_meterBuffer.read((char *) values, m_numChnls * sizeof(float));
}

int OutputMaster::getNumChnls()
{
	return m_numChnls;
}

void OutputMaster::processBlock(AudioIOData &io)
{
	int i, chan = 0;
	int nframes = io.framesPerBuffer();
	double bass_buf[nframes];
	double filt_out[nframes];
	double filt_low[nframes];
	double in_buf[nframes];
	double master_gain;

	master_gain = m_masterGain * (m_muteAll ? 0.0 : 1.0);
	memset(bass_buf, 0, nframes * sizeof(double));
	for (chan = 0; chan < m_numChnls; chan++) {
		double gain = master_gain * m_gains[chan];
		const float *in = io.outBuffer(chan);  // Yes, the input here is the output from previous runs for the io object
		float *out = io.outBuffer(chan);
		double filt_temp[nframes];
		double *buf = bass_buf;

		for (i = 0; i < nframes; i++) {
			in_buf[i] = *in++;
		}
		switch (m_BassManagementMode) {
		case BASSMODE_NONE:
			break;
		case BASSMODE_MIX:
			for (i = 0; i < nframes; i++) {
				filt_low[i] = in_buf[i];
			}
			break;
		case BASSMODE_LOWPASS:
			butter_next(m_lopass1[chan], in_buf, filt_temp, nframes);
			butter_next(m_lopass2[chan], filt_temp, filt_low, nframes);
			break;
		case BASSMODE_HIGHPASS:
			for (i = 0; i < nframes; i++) {
				filt_low[i] = in_buf[i];
			}
			butter_next(m_hipass1[chan], in_buf, filt_temp, nframes);
			butter_next(m_hipass2[chan], filt_temp, filt_out, nframes);
			for (i = 0; i < nframes; i++) {
				in_buf[i] = filt_out[i];
			}
			break;
		case BASSMODE_FULL:
			butter_next(m_lopass1[chan], in_buf, filt_temp, nframes);
			butter_next(m_lopass2[chan], filt_temp, filt_low, nframes);
			butter_next(m_hipass1[chan], in_buf, filt_temp, nframes);
			butter_next(m_hipass2[chan], filt_temp, filt_out, nframes);
			for (i = 0; i < nframes; i++) {
				in_buf[i] = filt_out[i]; /* a bit inefficient to copy here, but makes code simpler below */
			}
			break;
		default:
			break;
		}
		for (i = 0; i < nframes; i++) { /* accumulate SW signal */
			*buf++ += filt_low[i];
		}
		if (m_filtersActive
				&& m_BassManagementMode == BASSMODE_NONE && !chanIsSubwoofer(chan)) { /* apply DRC filters */
			//            firfilter_next(pp->filters[chan],in_buf, filt_out, nframes, gain);
			//            for (i = 0; i < nframes; i++) {
			//                *out = filt_out[i];
			//                if (pp->clipper_on && *out > gain) {
			//                    *out = gain;
			//                }
			//                out++;
			//            }
		} else { /* No DRC filters, just apply gain */
			for (i = 0; i < nframes; i++) {
				*out = in_buf[i] * gain;
				if (m_clipperOn && *out > gain) {
					*out = gain;
				}
				out++;
			}
		}
	}
	if (m_BassManagementMode != BASSMODE_NONE) {
		int sw;
		for(sw = 0; sw < 4; sw++) {
			if (swIndex[sw] < 0) continue;
			float *out = io.outBuffer(swIndex[sw]);
			memset(out, 0, nframes * sizeof(float));
			for (i = 0; i < nframes; i++) {
				*out++ = bass_buf[i];
			}
		}
	}
	if (m_meterOn) {
		for (chan = 0; chan < m_numChnls; chan++) {
			float *out = io.outBuffer(chan);
			for (i = 0; i < nframes; i++) {
				if (m_meters[chan] < *out) {
					m_meters[chan] = *out;
				}
				out++;
			}
		}
		m_meterCounter += nframes;
		if (m_meterCounter >= m_meterUpdateSamples) {
			m_meterBuffer.write( (char *) m_meters.data(), sizeof(float) * m_numChnls);
			memset(m_meters.data(), 0, sizeof(float) * m_numChnls);
			m_meterCounter = 0; // A little jitter but efficient
		}
	}
}

int OutputMaster::chanIsSubwoofer(int index)
{
	int i;
	for (i = 0; i < 4; i++) {
		if (swIndex[i] == index && m_BassManagementMode != BASSMODE_NONE) return 1;
	}
	return 0;
}

void OutputMaster::initializeData()
{
	m_masterGain = 1.0;
	m_muteAll = false;
	m_clipperOn = true;
	m_filtersActive = false;

	m_meterCounter = 0;
	m_meterOn = false;

	setBassManagementMode(BASSMODE_NONE);
	setBassManagementFreq(150);
	setMeterUpdateFreq(10.0);
}

void OutputMaster::allocateChannels(int numChnls)
{
	m_gains.resize(numChnls);
	m_meters.resize(numChnls);
	//    filters = (FIRFILTER **) calloc(numChnls, sizeof(FIRFILTER *));
	m_lopass1.resize(numChnls);
	m_lopass2.resize(numChnls);
	m_hipass1.resize(numChnls);
	m_hipass2.resize(numChnls);
	swIndex[0] = numChnls - 1;
	swIndex[1] =  swIndex[2] = swIndex[3] = -1;

	for (int i = 0; i < numChnls; i++) {
		m_gains[i] = 0.2;
		m_meters[i] = 0;
		m_lopass1[i] = butter_create(m_framesPerSec, BUTTER_LP);
		m_lopass2[i] = butter_create(m_framesPerSec, BUTTER_LP);
		m_hipass1[i] = butter_create(m_framesPerSec, BUTTER_HP);
		m_hipass2[i] = butter_create(m_framesPerSec, BUTTER_HP);

	}
	m_numChnls = numChnls;
}
