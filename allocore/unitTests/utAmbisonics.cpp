
#include <cstdlib>

#include "utAllocore.h"

void testFirstOrder2D() {
	const int bufferSize = 16;
	float signal[bufferSize];
	float ambiBuffer[bufferSize*3];
	memset(ambiBuffer, 0, sizeof(float) * bufferSize * 3);
	for (int i = 0; i < bufferSize; i++) {
		signal[i] = 0.5;
	}
	// Encoding
	AmbiEncode encoder(2, 1);
	encoder.direction(1, 0, 0);
	for (int i = 0; i < bufferSize; i++) {
		encoder.encode(ambiBuffer, bufferSize, i, signal[i]);
	}

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
	decoder.setSpeakers(&(speakerLayout.speakers()));

	decoder.decode(speakerSignals, ambiBuffer, bufferSize);

	for (int spkr = 1; spkr < speakerLayout.numSpeakers(); spkr++) {
		for (int i = 0; i < bufferSize; i++) {
			assert(speakerSignals[i] > speakerSignals[spkr * bufferSize + i]);
		}
	}
}

void testMatrixMultiplicationFirstOrder()
{
	const int bufferSize = 16;

	SpeakerLayout speakerLayout = SpeakerRingLayout<8>();
	AmbiDecode decoder(3, 1, speakerLayout.numSpeakers(), 0);  // Flavor 0 for no weighing
	float bformat[bufferSize*4];
	float speakerSignals[bufferSize*8];
	memset(bformat, 0, sizeof(float) * bufferSize * 4);
	memset(speakerSignals, 0, sizeof(float) * bufferSize * 8);
	decoder.setSpeakers(&(speakerLayout.speakers()));

	for (int harmonic = 0; harmonic < 4; harmonic++) {
		for(int samp = 0; samp < bufferSize; samp ++) {
			// Harmonics set to sample index + harm/16.0 (first harmonic  = 1/16, last harmonic = 1.0)
			bformat[harmonic * bufferSize + samp] = samp + (harmonic + 1.0)/16.0;
		}
	}

	Array decodeMatrix(1, AlloFloat32Ty, 4, 8);
	for (int harmonic = 0; harmonic < 4; harmonic++) {
		for(int speaker = 0; speaker < 8; speaker ++) {
			float *coeff = decodeMatrix.cell<float>(harmonic, speaker);
			*coeff = harmonic + 1.0/ (speaker + 1);
		}
	}
	decoder.setDecodeMatrix(decodeMatrix);

	decoder.decode(speakerSignals, bformat, bufferSize);

	for (int spkr = 0; spkr < speakerLayout.numSpeakers(); spkr++) {
		for (int i = 0; i < bufferSize; i++) {
			float decoded = 0;
			for (int harmonic = 0; harmonic < 4; harmonic++) {
				float sample  = i + (harmonic + 1.0)/16.0;
				float *coeff = decodeMatrix.cell<float>(harmonic, spkr);
				decoded += sample * *coeff;
			}
			assert(almostEqual(speakerSignals[spkr * bufferSize + i], decoded ) );
		}
	}
}


void testMatrixMultiplicationThirdOrder()
{

	const int bufferSize = 16;

	SpeakerLayout speakerLayout = SpeakerRingLayout<24>();
	AmbiDecode decoder(3, 3, 24, 0);
	float bformat[bufferSize*16];
	float speakerSignals[bufferSize*24];
	memset(bformat, 0, sizeof(float) * bufferSize * 16);
	memset(speakerSignals, 0, sizeof(float) * bufferSize * 24);
	decoder.setSpeakers(&(speakerLayout.speakers()));

	for (int harmonic = 0; harmonic < 16; harmonic++) {
		for(int samp = 0; samp < bufferSize; samp ++) {
			// Harmonics set to sample index + harm/16.0 (first harmonic  = 1/16, last harmonic = 1.0)
			bformat[harmonic * bufferSize + samp] = samp + (harmonic + 1.0)/16.0;
		}
	}

	Array decodeMatrix(1, AlloFloat32Ty, 16, 24);
	for (int harmonic = 0; harmonic < 16; harmonic++) {
		for(int speaker = 0; speaker < 24; speaker ++) {
			float *coeff = decodeMatrix.cell<float>(harmonic, speaker);
			*coeff = harmonic + 1.0/ (speaker + 1);
		}
	}
	decoder.setDecodeMatrix(decodeMatrix);

	decoder.decode(speakerSignals, bformat, bufferSize);

	for (int spkr = 0; spkr < speakerLayout.numSpeakers(); spkr++) {
		for (int i = 0; i < bufferSize; i++) {
			float decoded = 0;
			for (int harmonic = 0; harmonic < 16; harmonic++) {
				float sample  = i + (harmonic + 1.0)/16.0;
				float *coeff = decodeMatrix.cell<float>(harmonic, spkr);
				decoded += sample * *coeff;
			}
			assert(almostEqual(speakerSignals[spkr * bufferSize + i], decoded ) );
		}
	}
}

int utAmbisonics() {
	testFirstOrder2D();
	testMatrixMultiplicationFirstOrder();
	testMatrixMultiplicationThirdOrder();
	return 0;
}
