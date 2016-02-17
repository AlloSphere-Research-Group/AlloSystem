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
#include "alloaudio/al_Decorrelation.hpp"
#include "alloaudio/al_BassManager.hpp"

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

//	string decoderConfig = "Allosphere.ambdec";
//	if (argc > 1) {
//		decoderConfig = argv[1];
//	}
//	AmbiFilePlayer player(fullPath, loop, framesPerBuffer * 4, decoderConfig);

	SpeakerLayout layout = HeadsetSpeakerLayout();
	AmbiFilePlayer player(fullPath, loop, framesPerBuffer * 4, layout);

	PeakData peakData(player.numDeviceChannels() + 1);

	AudioIO io(framesPerBuffer, 44100, NULL, (void *) &peakData, player.numDeviceChannels() + 1, 0);

	MeteringCallback meterCb;
	Decorrelation decor(1024, -1, io.channels(true));
	BassManager bassManager;
	bassManager.configure(io.channels(true) + 1,
	                      io.framesPerSecond(),
	                      io.framesPerBuffer(),
	                      150.0,
	                      BassManager::BASSMODE_FULL);
	bassManager.setSwIndeces(2);

	decor.configure(io);

	// An AmbiFilePlayer object can be "appended" to an AudioIO object
	// because it inherits from AudioCallback.
	io.append(player);
	io.append(meterCb);
	io.append(decor);
	io.append(bassManager);

	player.play(); // Start playback of the audio file

	bool play = true;
	std::cout << "Press 'q' to quit, 'p' to pause, 'c' to continue, 'r' to rewind." << std::endl;
	if (io.start()) {
		struct timeval timeout;

		fd_set rset;
#define STDIN 0
		int maxfpd1 = al::max(STDIN, 0) + 1;
		while (!player.done() && play) {
			timeout.tv_sec = 1;
		    timeout.tv_usec = 0;

			FD_ZERO(&rset);
			FD_SET(STDIN, &rset);
			int retval = select(maxfpd1, &rset, NULL, NULL, &timeout);
			if (retval < 0)  {
				play = false;
			}
			else if (retval > 0) {
				std::string input;
				std::getline(std::cin, input);
				if (input == "q") {
					play = false;
				}
				if (input == "p") {
					player.pause();
					std::cout << "Paused." << std::endl;
				}
				if (input == "c") {
					player.play();
					std::cout << "Unpaused." << std::endl;
				}
				if (input == "r") {
					player.rewind();
					std::cout << "Rewinded." << std::endl;
				}
			}
//			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	} else {
		std::cout << "Error starting Audio." << std::endl;
	}
	return 0;
}

