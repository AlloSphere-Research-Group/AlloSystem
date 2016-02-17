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
#include <mutex>
#include <condition_variable>

#include <pthread.h>

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/types/al_SingleRWRingBuffer.hpp"
#include "allocore/types/al_MsgQueue.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "allocore/system/al_Thread.hpp"

#include "alloaudio/al_BassManager.hpp"

namespace al {

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


/** \addtogroup alloaudio
 *  @{
 */

/** Control of audio output. This class is designed to be used as the last class in the
 * audio callback, after any synthesis and spatialization.
 *
 * It can be controlled by OSC over UDP.
 *
 */
class OutputMaster : public osc::Recv, public al::AudioCallback
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
	OutputMaster(int numChnls, double sampleRate,
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

	/** Process a block of audio data. This can be called by itself passing an AudioIOData
	 * object or the OutputMaster object can be appended to the
	 * \code
	al::AudioIO io(4, 44100.0, NULL, NULL, 2, 2, 0, al::AudioIO::DUMMY);
	al::OutputMaster outmaster(io.channelsOut(), io.framesperSecond());
	io.append(outmaster);
	 *
	 * \endcode
	 */
	void onAudioCB(AudioIOData &io);

	/** Sets whether the meter messages are sent as /Alloaudio/meterdb if 0 -6.02
	 * or /Alloaudio/meterdb/1 -6.02. The latter includes the channel number in the OSC
	 * address and counts from 1.
	 */
	void setMeterAddrHasChannel(bool meterAddrHasChannel);

	bool meterAddrHasChannel() const;

	void setUseDb(bool use) {mUseDb = use;}

	/** Sets the prefix for OSC messages. By default "/Alloaudio" is used. The address
	 * prefix should be given without the trailing '/'.
	 */
	void setAddressPrefix(const std::string &addressPrefix);
	std::string addressPrefix() const;

protected:
	void setGainTimestamped(al_sec until,
	                        int channelIndex, double gain);
	void setMasterGainTimestamped(al_sec until, double gain);
	void setClipperOnTimestamped(al_sec until, bool on);
	void setMuteAllTimestamped(al_sec until, bool on);
	void setMeterOnTimestamped(al_sec until, bool on);
	void setMeterupdateFreqTimestamped(al_sec until, double freq);

private:
	const int mNumChnls;
	const double mFramesPerSec; // Sample rate

	/* parameters */
	std::string mAddressPrefix;
	std::vector<double> mGains;
	bool mMuteAll; // 0=no 1=yes
	double mMasterGain;
	bool mClipperOn;
	bool mMeterOn;
	bool mMeterAddrHasChannel;
	int mMeterUpdateSamples; /* number of samples between level updates */

	MsgQueue mParameterQueue;

	/* output data */
	std::vector<float> mMeters;
	SingleRWRingBuffer mMeterBuffer;
	int mMeterCounter; /* count samples for level updates */
	std::string mSendAddress;
	int mSendPort;
	int mRunMeterThread;
	bool mUseDb;
	al::Thread mMeterThread;
	std::mutex mMeterMutex;
	std::mutex mMeterCondMutex;
	std::condition_variable mMeterCond;

	void initializeData();
	void allocateChannels(int numChnls);
	static void *meterThreadFunc(void *arg);

	struct OSCHandler : public osc::PacketHandler{
		OutputMaster *outputmaster;
		void onMessage(osc::Message& m);
	} msghandler;
};

/** @} */

} // al::

#endif
