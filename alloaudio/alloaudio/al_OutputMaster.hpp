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

#include <pthread.h>

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/types/al_SingleRWRingBuffer.hpp"
#include "allocore/types/al_MsgQueue.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "allocore/system/al_Thread.hpp"


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


/** Control of audio output. This class is designed to be used as the last class in the
 * audio callback, after any synthesis and spatialization.
 *
 * It can be controlled by OSC over UDP.
 */
class OutputMaster : public osc::Recv
{
    public:
    /**
     * @brief OutputMaster
     * @param num_chnls number of channels that will be processed by the OutputMaster object
     * @param sampleRate the audio sampling rate
     * @param address the IP address to bind for receiving messages. For local address use "localhost"
     * @param inport The port number for input
     * @param sendAddress The IP address to which messages will be sent
     * @param sendPort The port to which messages will be sent
     * @param msg_timeout Time out for the socket listener (see documentation for al::osc::Recv)
     */
    OutputMaster(int num_chnls, double sampleRate,
                 const char * address = "", int inport = 19375,
                 const char * sendAddress = "localhost", int sendPort = -1,
                 al_sec msg_timeout = 0);
    ~OutputMaster();

    /** Set master output gain. This gain is applied after individual channel gains, and
     * determines the value at which signals are clipped if the clipper is set with
     * setClipperOn()
     */
    void setMasterGain(double gain);

    /** Set the channel gain for channel channelIndex. Note that channel indeces count
     *  from 0
     */
    void setGain(int channelIndex, double gain);

    /** Mute the system if muteAll is true. When unmuted, all gains will return to their
     * previous state.
     */
    void setMuteAll(bool muteAll);

    /** If clipperOn is true, the output signal for a channel is clipped if it is greater
     * than the global gain set with setGlobalGain(). If false, there is no clipping.
     * It is recommended that for systems with large numbers of channels you set this to
     * to avoid loud surprises.
     */
    void setClipperOn(bool clipperOn);

    /** Set the frequency at which peak meter data is updated. During the update period,
     * a single value (maximum sample peak) is stored, and will only be avialable once
     * the period is completed, as this is passed to the non-audio context through a
     * lock-free ring buffer.
     */
    void setMeterUpdateFreq(double freq);

    /** Set bass management cross-over frequency. The signal from all channels will be run
     * through a pair of linear-phase cross-over filters, and the signal from the low pass
     * filters is sent to the subwoofers specified using setSwIndeces().
    */
    void setBassManagementFreq(double frequency);

    void setBassManagementMode(bass_mgmt_mode_t mode);

    /** Specify which channel indeces are subwoofers for the purpose of bass management.
     * Currently a maximum of 4 subwoofers are supported.
     */
    void setSwIndeces(int i1, int i2, int i3, int i4);

    /** Enable peak metering. If set to false, no meter values will be sent out via OSC,
     * and no values will be provided by getMeterValues()
     */
    void setMeterOn(bool meterOn);

    /** Fill the values array with the peak meter values. Note that because the values
     * are stored in a ringbuffer, this function cannot be used together with OSC meters
     * as the OSC thread will empty the values before they are read here.
     *
     * @return returns the number of meter values read.
     */
    int getMeterValues(float *values);

    /** Get the number of channels processed by this OutputMaster object */
    int getNumChnls();

    /** Process a block of audio data. The io object should come from a previous process,
     * like an Ambisonics or VBAP panner, as processBlock() processes the output buffer in
     * the io object in place.
     */
    void processBlock(AudioIOData &io);

    void setGainTimestamped(al_sec until,
                            int channelIndex, double gain);
    void setMasterGainTimestamped(al_sec until, double gain);
    void setClipperOnTimestamped(al_sec until, bool on);
    void setMuteAllTimestamped(al_sec until, bool on);
    void setMeterOnTimestamped(al_sec until, bool on);
    void setMeterupdateFreqTimestamped(al_sec until, double freq);
    void setBassManagementFreqTimestamped(al_sec until, double freq);
    void setBassManagementModeTimestamped(al_sec until, int mode);

private:
    const int m_numChnls;

    /* parameters */
    std::vector<double> m_gains;
    bool m_muteAll; // 0=no 1=yes
    double m_masterGain;
    bool m_clipperOn;
    bool m_meterOn;
    int m_meterUpdateSamples; /* number of samples between level updates */

    bass_mgmt_mode_t m_BassManagementMode;
    int swIndex[4]; /* support for 4 SW max */

    MsgQueue m_parameterQueue;

    /* output data */
    std::vector<float> m_meters;
    SingleRWRingBuffer m_meterBuffer;
    int m_meterCounter; /* count samples for level updates */
    std::string m_sendAddress;
    int m_sendPort;
    int m_runMeterThread;
    al::Thread m_meterThread;
    pthread_mutex_t m_meterMutex;
    pthread_cond_t m_meterCond;

    /* bass management filters */
    std::vector<BUTTER *> m_lopass1, m_lopass2, m_hipass1, m_hipass2;

    double m_framesPerSec; // Sample rate

    int chanIsSubwoofer(int index);
    void initializeData();
    void allocateChannels(int numChnls);
    static void *meterThreadFunc(void *arg);

    struct OSCHandler : public osc::PacketHandler{
        OutputMaster *outputmaster;
        void onMessage(osc::Message& m);
    } msghandler;
};

} // al::

#endif
