/*	Alloaudio --
    Audio facilities for large multichannel systems

    Copyright (C) 2014. AlloSphere Research Group, Media Arts & Technology, UCSB.
    Copyright (C) 2014. The Regents of the University of California.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the University of California nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


    File description:
    Audio output control offering gain, bass management, room compensation
    filtering and metering. OSC control of parameters.

    File author(s):
    Andres Cabrera, mantaraya36@gmail.com
*/


#ifndef INC_AL_OUTPUTMASTER_HPP
#define INC_AL_OUTPUTMASTER_HPP

#include <vector>

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/types/al_SingleRWRingBuffer.hpp"
#include "allocore/types/al_MsgQueue.hpp"

typedef struct BUTTER_ BUTTER;

namespace al {

typedef enum {
    BASSMODE_NONE = 0,
    BASSMODE_MIX = 1,
    BASSMODE_LOWPASS = 2,
    BASSMODE_HIGHPASS = 3,
    BASSMODE_FULL = 4,
    BASSMODE_COUNT
} bass_mgmt_mode_t;

typedef enum {
    CHANNEL_GAIN = 0,
    MASTER_GAIN,
    MUTE_ALL,
    CLIPPER,
    FILTERS_ACTIVE,
    BASS_MNGMT_FREQ,
    BASS_MNGMT_MODE,
    SW_INDECES,
    METER_ON,
    METER_UPDATE_SAMPLES,
    PARAMETER_COUNT,
} parameter_t;

class OutputMaster
{
public:
    OutputMaster(int num_chnls, double sampleRate);
    ~OutputMaster();

    void setFilters(double **irs, int filter_len);
    void setGlobalGain(double gain);
    void setGain(int channelIndex, double gain);
    void setMuteAll(bool m_muteAll);
    void setClipperOn(bool m_clipperOn);
    void setRoomCompensationOn(bool on);
    void setMeterUpdateFreq(double freq);

    /* set frequency to 0 to skip bass management cross-over filters, but still have signals added to subwoofers.
       set to -1 skip bass_management completely */
    void setBassManagementFreq(double frequency);
    void setBassManagementMode(bass_mgmt_mode_t mode);
    void setSwIndeces(int i1, int i2, int i3, int i4);
    void setMeterOn(bool meterOn);

    int getMeterValues(float *values);
    int getNumChnls();

    void processBlock(AudioIOData &io);

private:
    int m_numChnls;

    /* parameters */
    std::vector<double> m_gains;
    bool m_muteAll; // 0=no 1=yes
    double m_masterGain;
    bool m_clipperOn;
    bool m_filtersActive;
    bool m_meterOn;
    int m_meterUpdateSamples; /* number of samples between level updates */

    bass_mgmt_mode_t m_BassManagementMode; /* -1 no management, 0 SW routing without filters, >0 cross-over freq. in Hz. */
    int swIndex[4]; /* support for 4 SW max */

    MsgQueue m_parameterQueue;
    pthread_mutex_t m_paramMutex;

    /* output data */
    std::vector<float> m_meters;
    SingleRWRingBuffer m_meterBuffer;
    int m_meterCounter; /* count samples for level updates */

    /* DRC (output) filters */
//    FIRFILTER **filters;

    /* bass management filters */
    std::vector<BUTTER *> m_lopass1, m_lopass2, m_hipass1, m_hipass2;

    double m_framesPerSec; // Sample rate

    void setParameter(al_sec time, parameter_t param, double value);
    void setParameter(al_sec time, parameter_t param, bool value);
    void setParameter(al_sec time, parameter_t param, int value1, double value2);
    int chanIsSubwoofer(int index);
    void initializeData();
    void allocateChannels(int numChnls);
};

} // al::

#endif
