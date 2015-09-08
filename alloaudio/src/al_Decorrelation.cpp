/*	Decorrelation

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

	For details about the process to generate the IRs for decorrelation, see:

	Kendall, G. (1995). The Decorrelation of Audio Signals an Its Impact on Spatial Imagery. Computer Music Journal, 19:4, 71–87.

	and:

	Zotter, F., Frank, M., Marentakis, G., & Sontacchi, A. (2011). Phantom Source Widening with Deterministic Frequency Dependent Time Delays. DAFx-11, 307–312.

	File author(s):
	Andres Cabrera, mantaraya36@gmail.com
*/

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <cassert>

#include "alloaudio/al_Decorrelation.hpp"
#include <Gamma/FFT.h>

using namespace al;
using namespace std;

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

Decorrelation::Decorrelation(int size, int inChannel, int numOuts,
							 bool inputsAreBuses) :
	mSize(size), mInChannel(inChannel), mNumOuts(numOuts),
	mInputsAreBuses(inputsAreBuses)
{
}

Decorrelation::~Decorrelation()
{
	freeIRs();
}

long al::Decorrelation::getCurrentSeed()
{
	return mSeed;
}

void Decorrelation::generateIRs(long seed, float maxjump)
{
	float *ampSpectrum = (float *) calloc(mSize, sizeof(float));
	float *phsSpectrum = (float *) calloc(mSize, sizeof(float));
	float *complexSpectrum = (float *) calloc(mSize * 2, sizeof(float));;
	//	#    max_jump -  is the maximum phase difference (in radians) between bins
	//	#             if -1, the random numbers are used directly (no jumping).

	freeIRs();

	int n = mSize/2; // before mirroring

	// Seed random number generator
	if (seed >= 0) {
		mSeed = seed;
	} else {
		mSeed = time(0);
	}
	srand(mSeed);
	for (int irIndex = 0; irIndex < mNumOuts; irIndex++) {
		// Fill in DC and Nyquist
		ampSpectrum[0] = 1.0;
		phsSpectrum[0] = 0.0;
		complexSpectrum[0] = ampSpectrum[0] * cos(phsSpectrum[0]);
		complexSpectrum[1] = ampSpectrum[0] * sin(phsSpectrum[0]);
		complexSpectrum[(n*2)] = ampSpectrum[0] * cos(phsSpectrum[0]);
		complexSpectrum[(n*2) + 1] = ampSpectrum[0] * sin(phsSpectrum[0]);

		float old_phase = 0;
		for (int i=1; i < n; i++) {
			ampSpectrum[i] = 1.0;
			if (maxjump == -1.0) {
				phsSpectrum[i] = ((rand() / (float) RAND_MAX) * M_PI)- (M_PI/2.0);
			} else {
				// make phase only move +- limit
				float delta = ((rand() / ((float) RAND_MAX)) * 2.0 * maxjump) - maxjump;
//				std::cout << "delta " << delta << std::endl;
				float new_phase = old_phase + delta;
				phsSpectrum[i] = new_phase;
				old_phase = new_phase;
			}
			
			complexSpectrum[i*2] = ampSpectrum[i] * cos(phsSpectrum[i]); // Real part
			complexSpectrum[i*2 + 1] = ampSpectrum[i] * sin(phsSpectrum[i]); // Imaginary
//			std::cout << complexSpectrum[i*2] << ", " << complexSpectrum[i*2 + 1] << ",";
		}

//		std::cout << ".... " <<std::endl;
		gam::RFFT<float> fftObj(mSize);

		fftObj.inverse(complexSpectrum, true);
		float *irdata = (float *) calloc(mSize, sizeof(float));

		for (int i=1; i <= mSize; i++) {
			irdata[i - 1] = complexSpectrum[i]/mSize;
//			std::cout << complexSpectrum[i]/mSize << "," << std::endl;
		}
		mIRs.push_back(irdata);
	}
	free(ampSpectrum);
	free(phsSpectrum);
	free(complexSpectrum);
}

void Decorrelation::generateDeterministicIRs(long seed, float deltaFreq, float maxFreqDev,
											 float maxTau)
{
	float *ampSpectrum = (float *) calloc(mSize, sizeof(float));
	float *phsSpectrum = (float *) calloc(mSize, sizeof(float));
	float *complexSpectrum = (float *) calloc(mSize * 2, sizeof(float));;
	//	#    max_jump -  is the maximum phase difference (in radians) between bins
	//	#             if -1, the random numbers are used directly (no jumping).

	freeIRs();

	int n = mSize/2; // before mirroring

	// Seed random number generator
	if (seed >= 0) {
		mSeed = seed;
	} else {
		mSeed = time(0);
	}
	srand(mSeed);

	for (int irIndex = 0; irIndex < mNumOuts; irIndex++) {
		float freq = deltaFreq + ((2.0 * maxFreqDev * rand() / (float) RAND_MAX) - maxFreqDev);
		std::cout << "freq " << irIndex << ":" << freq << std::endl;
		for (int i=0; i < n + 1; i++) {
			ampSpectrum[i] = 1.0;
			phsSpectrum[i] = maxTau * sin(2 * M_PI * i * freq / n);

			complexSpectrum[i*2] = ampSpectrum[i] * cos(phsSpectrum[i]); // Real part
			complexSpectrum[i*2 + 1] = ampSpectrum[i] * sin(phsSpectrum[i]); // Imaginary
//			std::cout << complexSpectrum[i*2] << ", " << complexSpectrum[i*2 + 1] << ",";
		}

//		std::cout << ".... " <<std::endl;
		gam::RFFT<float> fftObj(mSize);

		fftObj.inverse(complexSpectrum, true);
		float *irdata = (float *) calloc(mSize, sizeof(float));

		for (int i=1; i <= mSize; i++) {
			irdata[i - 1] = complexSpectrum[i]/mSize;
//			std::cout << complexSpectrum[i]/mSize << "," << std::endl;
		}
		mIRs.push_back(irdata);
	}
	free(ampSpectrum);
	free(phsSpectrum);
	free(complexSpectrum);
}

void Decorrelation::onAudioCB(al::AudioIOData &io)
{
	mConv.onAudioCB(io);
}

float *al::Decorrelation::getIR(int index)
{
	if (index < 0 || index >= mSize) {
		return NULL;
	}
	return mIRs[index];
}

int al::Decorrelation::getSize()
{
	return mSize;
}

void al::Decorrelation::freeIRs()
{
	for (unsigned int i = 0; i < mIRs.size(); i++){
		free(mIRs[i]);
	}
	mIRs.clear();
}

void Decorrelation::configure(al::AudioIO &io, long seed, float maxjump)
{
	mSeed = seed;
	if (mSize > 16 && mNumOuts != 0) {
		generateIRs(seed, maxjump);
	} else {
		mSize = 0;
		cout << "Invalid size: " << mSize << " numOuts: " << mNumOuts << endl;
		return;
	}
	if (mSize >= 64) {
		int options = 2; //vector mode
		mConv.configure(io, mIRs, mSize, mInChannel, mInputsAreBuses, vector<int>(),
						io.framesPerBuffer(), options);
	}
}

void Decorrelation::configureDeterministic(AudioIO &io, long seed, float deltaFreq,
										   float deltaFreqDev, float maxTau)
{
	generateDeterministicIRs(seed, deltaFreq, deltaFreqDev, maxTau);
	if (mSize >= 64) {
		int options = 2; //vector mode
		mConv.configure(io, mIRs, mSize, mInChannel, mInputsAreBuses, vector<int>(),
						io.framesPerBuffer(), options);
	}
}
