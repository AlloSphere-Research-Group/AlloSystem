/* Ambsonics File player example
 *
 * By: Andrés Cabrera
*/

#include <iostream>
#include <iomanip> // For setprecision()
#include <chrono>
#include <thread>

#include "allocore/io/al_AudioIO.hpp"
#include "alloutil/al_AlloSphereSpeakerLayout.hpp"
#include "alloaudio/al_AmbiFilePlayer.hpp"
#include "alloaudio/al_AmbiTunedDecoder.hpp"

using namespace al;

class PeakData {
public:
	PeakData(int numChannels_) :
	    peakCounter(0),
	    numChannels(numChannels_),
	    peakAccumulator(new float[numChannels]) { }

	int peakCounter;
	int numChannels;
	std::unique_ptr<float[]> peakAccumulator;
};

class MeteringCallback : public AudioCallback {
public:
	virtual void onAudioCB(AudioIOData& io)  {
		PeakData *pd = static_cast<PeakData *>(io.user());
		while (io()) {
			for (int i = 0; i < pd->numChannels; i++) {
				float absValue = fabs(io.out(i));
				if (pd->peakAccumulator[i] < absValue) {
					pd->peakAccumulator[i] = absValue;
				}
			}
			pd->peakCounter++;
			if (pd->peakCounter == 44100) {
				for (int i = 0; i < pd->numChannels; i++) {
					std::cout << std::setprecision(3) << 20.0 * log10(pd->peakAccumulator[i]) << "  ";
					pd->peakAccumulator[i] = 0;
				}
				std::cout << std::endl;
				pd->peakCounter = 0;
			}
		}
	}
};

int main(int argc, char *argv[])
{
	std::string fullPath = "/media/part/Music/Ambisonics/Ambisonia/PWH_Purcell-Passacaglia_(How_Happy).amb";
	bool loop = false;
	int framesPerBuffer = 1024;
	string decoderConfig = "alloaudio/examples/Allosphere.ambdec";
	if (argc > 1) {
		decoderConfig = argv[1];
	}
	SpeakerLayout layout = OctalSpeakerLayout();
	AmbiFilePlayer player(fullPath, loop, framesPerBuffer * 4, layout);
//	AmbiFilePlayer player(fullPath, loop, framesPerBuffer * 4, decoderConfig);
	PeakData peakData(layout.numSpeakers());

	AudioIO io(framesPerBuffer, 44100, NULL, (void *) &peakData, 32);
	MeteringCallback meterCb;

	// An AmbiFilePlayer object can be "appended" to an AudioIO object
	// because it inherits from AudioCallback.
	io.append(player);
	io.append(meterCb);
	if (io.start()) {
		while (!player.done()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	} else {
		std::cout << "Error starting Audio." << std::endl;
	}
	return 0;
}

