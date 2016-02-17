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

/** \addtogroup alloaudio
 *  @{
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
	 *
	 * If inputsAreBuses is false the input to the decorrelation is read from
	 * the output buffers of the io object.
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
	 * @param phaseFactor The random phase generated is multiplied by this factor. This will allow a different control of the amount of decorrelation. A phaseFactor of 0.0 will result in no decorrelation as there will be no phase shift, and a phaseFactor of 1.0 will use the unmodified random numbers for the bin's phase
	 */
	void configure(al::AudioIO &io, long seed = -1, float maxjump = -1, float phaseFactor = 1.0);

	/**
	 * @brief Calculates deterministic phase decorrelation IRs
	 *
	 * The IRs are generated using the Zotter method (DAFX 2011). This method
	 * constructs an all-pass filter whose phase traces a sinusoid. The number
	 * of oscillations of the phase within the Nyquist band is called here
	 * deltaFreq and the amplitude -i.e. the maximum variation of phase that
	 * the filter creates- is called maxTau. The original paper for the technique
	 * does not provide ideas on how to apply it for multichannel decorrelation
	 * so what is done here is to add two ways of generating varying (but
	 * related) impulse responses. The maxFreqDev parameter determines the
	 * maximum deviation that each IR will have with respect to deltaFreq. The
	 * higher this value, the more decorrelated the IRs will be. The second
	 * method is to have a phaseDev parameter that randomizes the starting
	 * phase of the sinuoisoidal phase response.
	 *
	 * @param io The AudioIO object
	 * @param seed The random seed for the creation of decorrelated IRs. seed < 0 means seed from current time.
	 * @param deltaFreq The number of cycles in the sinusoidal phase response
	 * @param maxFreqDev The maximum random deviation around deltaFreq for each IR.
	 * @param maxTau The amplitude of the sinusoidal phase response. A value of 1.0 means maximum deviation before wrapping the phase around pi.
	 * @param startPhase The phase for the sine function that determines the phase at bin 0.
	 * @param phaseDev The maximum random deviation around startPhase.
	 */
	void configureDeterministic(al::AudioIO &io, long seed = -1,
	                            float deltaFreq = 20, float maxFreqDev = 10,
	                            float maxTau = 1.0,
	                            float startPhase = 0.0, float phaseDev = 0.0);

	/**
	 * @brief getCurrentSeed returns the randon seed used to generate the current IRs
	 */
	long getCurrentSeed();

	virtual void onAudioCB(AudioIOData &io);

	/**
	 * @brief Get the decorration filter IR for index
	 * @param index The index of the IR. Must be < 0 and >= mNumOuts
	 * @return A pointer to the array containing the IR. The number of samples can be quieried with getSize(). Returns NULL when invalid
	 */
	float *getIR(int index);

	/**
	 * @brief Returns the size of the decorrelation filter IRs
	 */
	int getSize();

//	void processAudio(float *inputBuffer, float* outputBuffer, int index, int numSamples);

private:

	void freeIRs();
	void generateIRs(long seed = -1, float maxjump = -1.0, float phaseFactor = 1.0);
	void generateDeterministicIRs(long seed = -1,
	                              float deltaFreq = 30, float maxFreqDev = 10, float maxTau = 1.0,
	                              float startPhase = 0.0, float phaseDev = 0.0);

	vector<float *>mIRs;
	int mSize;
	int mInChannel;
	int mNumOuts;
	bool mInputsAreBuses;
	Convolver mConv;
	unsigned long mSeed;
};

/** @} */

} // al::

#endif
