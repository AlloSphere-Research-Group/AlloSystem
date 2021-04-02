#include <cstdlib> // memset
#include "utAllocore.h"
#include "allocore/sound/al_Ambisonics.hpp"
using namespace al;

void testFirstOrder2D() {
	const int bufferSize = 16;
	float signal[bufferSize];
	float ambiBuffer[bufferSize*3];
	memset(ambiBuffer, 0, sizeof(float) * bufferSize * 3);
	for (int i = 0; i < bufferSize; i++) {
		signal[i] = 0.5;
	}
	AmbiEncode encoder(2, 1);
	encoder.direction(1, 0, 0);

	// Encoding sample by sample
	for (int i = 0; i < bufferSize; i++) {
		encoder.encode(ambiBuffer, bufferSize, i, signal[i]);
	}
	for (int i = 0; i < bufferSize; i++) {
		assert(almostEqual(ambiBuffer[i], sqrt(2.0)/4.0)); // First harmonic (W)
		assert(almostEqual(ambiBuffer[bufferSize + i], 0.5)); // Second harmonic (X)
		assert(almostEqual(ambiBuffer[(2*bufferSize) + i], 0.0)); // Third harmonic (Y)
	}

	// Encoding by buffer
	memset(ambiBuffer, 0, sizeof(float) * bufferSize * 3);
	encoder.encode(ambiBuffer, signal, bufferSize);
	for (int i = 0; i < bufferSize; i++) {
		assert(almostEqual(ambiBuffer[i], sqrt(2.0)/4.0)); // First harmonic (W)
		assert(almostEqual(ambiBuffer[bufferSize + i], 0.5)); // Second harmonic (X)
		assert(almostEqual(ambiBuffer[(2*bufferSize) + i], 0.0)); // Third harmonic (Y)
	}

	//Decoding
	SpeakerLayout speakerLayout = OctalSpeakerLayout();
	AmbiDecode decoder(2, 1, 8, 1);
	float speakerSignals[bufferSize*8];
	memset(speakerSignals, 0, sizeof(float) * bufferSize * 8);
	decoder.setSpeakers(speakerLayout.speakers());

	//decoding by sample
	for (int i = 0; i < bufferSize; i++) {
		decoder.decode(speakerSignals, ambiBuffer, bufferSize, i);
	}

	for (int spkr = 1; spkr < speakerLayout.numSpeakers(); spkr++) {
		for (int i = 0; i < bufferSize; i++) {
			// speaker 0 should be the loudest
			assert(speakerSignals[i] > speakerSignals[spkr * bufferSize + i]);
		}
	}

	//decoding by buffer
	memset(speakerSignals, 0, sizeof(float) * bufferSize * 8);
	decoder.decode(speakerSignals, ambiBuffer, bufferSize);

	for (int spkr = 1; spkr < speakerLayout.numSpeakers(); spkr++) {
		for (int i = 0; i < bufferSize; i++) {
			// speaker 0 should be the loudest
			assert(speakerSignals[i] > speakerSignals[spkr * bufferSize + i]);
		}
	}
}

int utAmbisonics() {
	testFirstOrder2D();

	return 0;
}
