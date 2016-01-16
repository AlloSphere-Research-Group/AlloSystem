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
#include "alloaudio/al_OutputMaster.hpp"
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
	SpeakerLayout speakerLayout = OctalSpeakerLayout();
	AmbiFilePlayer player(fullPath, loop, framesPerBuffer, speakerLayout);
	PeakData peakData(speakerLayout.numSpeakers());

	AudioIO io(framesPerBuffer, 44100, NULL, (void *) &peakData, speakerLayout.numSpeakers());
	OutputMaster master(io.channels(true), io.fps(), "", 19375, "localhost", 19376);
	MeteringCallback meterCb;
	AmbiTunedDecoder decoder("/home/andres/Documents/src/Allostuff/adt/decoders/Allosphere-full_3h3p_allrad_5200_rE_max_1_band.ambdec");

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

