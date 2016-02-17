/*	Alloaudio --
    Audio facilities for large multichannel systems

    Copyright (C) 2016. AlloSphere Research Group, Media Arts & Technology, UCSB.
    Copyright (C) 2016. The Regents of the University of California.
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
    Bass Management audio node

    File author(s):
    Andres Cabrera, mantaraya36@gmail.com
*/


#ifndef INC_AL_BASSMANAGER_HPP
#define INC_AL_BASSMANAGER_HPP

#include <vector>

#include "allocore/io/al_AudioIO.hpp"

// FIXME use AlloSystem crossover filters (will they be performant enough?)
typedef struct BUTTER_ BUTTER;

/** \addtogroup alloaudio
 *  @{
 */

namespace al {

/** Bass Manager Class
 */
class BassManager : public AudioCallback
{
public:

	typedef enum {
		BASSMODE_NONE = 0,
		BASSMODE_MIX = 1,
		BASSMODE_LOWPASS = 2,
		BASSMODE_HIGHPASS = 3,
		BASSMODE_FULL = 4,
		BASSMODE_COUNT
	} bass_mgmt_mode_t;

	BassManager();

	~BassManager();

	void configure(int numChnls, double sampleRate,
	               int framesPerBuffer, float frequency,
	               BassManager::bass_mgmt_mode_t mode);


	/** Set bass management cross-over frequency. The signal from all channels will be run
	 * through a pair of linear-phase cross-over filters, and the signal from the low pass
	 * filters is sent to the subwoofers specified using setSwIndeces().
	*/
	void setBassManagementFreq(double frequency);

	void setBassManagementMode(bass_mgmt_mode_t mode);

	/** Specify which channel indeces are subwoofers for the purpose of bass management.
	 * Currently a maximum of 4 subwoofers are supported. An index of -1 indicates no
	 * subwoofer.
	 */
	void setSwIndeces(int i1, int i2 = -1, int i3 = -1, int i4 = -1);

	/** Process a block of audio data.
	 */
	void onAudioCB(AudioIOData &io);

protected:


private:
	int mNumChnls;
	double mFramesPerSec; // Sample rate
	int mFramesPerBuffer;

	/* parameters */
	bass_mgmt_mode_t mBassManagementMode;
	int swIndex[4]; /* support for 4 SW max */
	double *mBass_buf;
	double *mFilt_out;
	double *mFilt_low;
	double *mIn_buf;

	/* bass management filters */
	std::vector<BUTTER *> mLopass1, mLopass2, mHipass1, mHipass2;

	int chanIsSubwoofer(int index);
	void initializeData();
	void allocateChannels(int numChnls);
};

/** @} */

} // al::

#endif
