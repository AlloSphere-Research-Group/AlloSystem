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

	Decorrelation for multichannel using FIR filters.

	Kendall, G. (1995). The Decorrelation of Audio Signals an Its Impact on Spatial Imagery. Computer Music Journal, 19:4, 71–87.

	Zotter, F., Frank, M., Marentakis, G., & Sontacchi, A. (2011). Phantom Source Widening with Deterministic Frequency Dependent Time Delays. DAFx-11, 307–312. Retrieved from http://iem.kug.ac.at/fileadmin/media/iem/projects/2011/dafx11_zotter_frank_marentakis_sontacchi.pdf

    File author(s):
    Andres Cabrera, mantaraya36@gmail.com
*/


#ifndef INC_AL_DECORRELATION_HPP
#define INC_AL_DECORRELATION_HPP

#include <allocore/io/al_AudioIO.hpp>
#include <alloaudio/al_Convolver.hpp>

namespace al {

/**
 */
class Decorrelation : public AudioCallback
{
public:
	/**
	 * @brief Decorrelation
	 * @param size Length of the decorrelation filter.
	 * @param inChannel index of the channel that will be decorrelated. Setting
	 * to -1 realizes parallel decorrelation, where an input will be mapped to
	 * an output directly with decorrelation applied.
	 * @param numOuts number of output channels to process
	 * @param seed Seed for initial generation of decorrelation IRs. If -1, then
	 * seed is taken from current time
	 */
	Decorrelation(int size = 1024, int inChannel = 0, int numOuts = 8,
				  bool inputsAreBuses = false);
	~Decorrelation();

	/**
	 * @brief configure() calculates the decorrelation IRs and configures the convolver engine
	 *
	 * This function calculates the IRs using the Kendall method for FIR random phase all-pass filters.
	 *
	 * @param io The AudioIO object for audio rendering
	 * @param seed The seed for the random number generator used to calculate random phase. A value of -1 means seed to current time.
	 * @param maxjump The maximum difference allowed (in radians) in the random phase between adjacent bins. A value of -1 means no limit.
	 */
	void configure(al::AudioIO &io, long seed = -1, float maxjump = -1);

	/**
	 * @brief Calculates deterministic phase decorrelation IRs
	 *
	 * The IRs are generated using the Zotter method
	 *
	 *
	 * @param io
	 * @param seed
	 * @param deltaFreq
	 * @param maxFreqDev
	 * @param maxTau
	 */
	void configureDeterministic(al::AudioIO &io, long seed = -1, float deltaFreq = 20, float maxFreqDev = 10, float maxTau = 1.0);

	long getCurrentSeed();

	virtual void onAudioCB(AudioIOData &io);

	float *getIR(int index);
	int getSize();

//	void processAudio(float *inputBuffer, float* outputBuffer, int index, int numSamples);

private:

	void freeIRs();
	void generateIRs(long seed = -1, float maxjump = -1.0);
	void generateDeterministicIRs(long seed = -1, float deltaFreq = 30, float maxFreqDev = 10, float maxTau = 1.0);

	vector<float *>mIRs;
	int mSize;
	int mInChannel;
	int mNumOuts;
	bool mInputsAreBuses;
	Convolver mConv;
	unsigned long mSeed;
};

} // al::

#endif
