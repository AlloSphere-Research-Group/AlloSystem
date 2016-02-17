
#include <iostream>
#include <cassert>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/io/al_File.hpp"
#include "alloaudio/al_AmbiTunedDecoder.hpp"

using namespace al;
using namespace std;


void AmbiTunedDecoder::setConfiguration(string configFile)
{
	SearchPaths paths;
	paths.addSearchPath("alloaudio/data");
	paths.addSearchPath("../alloaudio/data");
	string filepath = paths.find(configFile).filepath();
	if (filepath.length() > 0) {
		std::ifstream file(filepath);
//		file.open(filepath, ios::in);
		if (!file) {
			std::cout << "ERROR: Could not OPEN ambdec configuration file: " << configFile << std::endl;
//			return;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		std::string line;
		mAmbisonicsConfig.mDecodeMatrix.clear();

		while(std::getline(buffer,line,'\n')){
			if (!line.empty() && line.at(0) == '/') {
				stringstream ssin(line);
				string token;
				if (ssin.good()){
					ssin >> token;
					if (token == "/description") {
						ssin >> token;
						mAmbisonicsConfig.mDescription = token;
					} else if (token == "/version") {
						ssin >> token;
						mAmbisonicsConfig.mVersion = std::stoi(token);
					} else if (token == "/dec/chan_mask") {
						ssin >> token;
						mAmbisonicsConfig.mDecChan_mask = token;
					} else if (token == "/dec/freq_bands") {
						ssin >> token;
						mAmbisonicsConfig.mDecChan_mask = std::stoi(token);
					} else if (token == "/dec/speakers") {
						ssin >> token;
						mAmbisonicsConfig.mDecSpeakers = std::stoi(token);
					} else if (token == "/dec/coeff_scale") {
						ssin >> token;
						mAmbisonicsConfig.mDecCoeff_scale = token;
					} else if (token == "/opt/input_scale") {
						ssin >> token;
						mAmbisonicsConfig.mOptInput_scale = token;
					} else if (token == "/opt/nfeff_comp") {
						ssin >> token;
						mAmbisonicsConfig.mOptNfeff_comp = token;
					} else if (token == "/opt/delay_comp") {
						ssin >> token;
						mAmbisonicsConfig.mOptDelay_comp = token;
					} else if (token == "/opt/level_comp") {
						ssin >> token;
						mAmbisonicsConfig.mOptLevel_comp = token;
					} else if (token == "/opt/xover_freq") {
						ssin >> token;
						mAmbisonicsConfig.mOptXover_freq = std::stod(token);
					} else if (token == "/opt/xover_ratio") {
						ssin >> token;
						mAmbisonicsConfig.mOptXover_ratio = std::stod(token);
					} else if (token == "/speakers/{") {
						std::getline(buffer,line,'\n');
						while(line.find("/}") == std::string::npos) {
							std::getline(buffer,line,'\n');
							if (line.at(0) != '#') {
								stringstream sprkst(line);
								sprkst >> token;
								if (token == "add_spkr") {
									string id;
									float dist, azim, elev;
									int deviceChannel = 0;
									sprkst >> id >> dist >> azim >> elev;
									if (sprkst.good()) {
										string channel;
										sprkst >> channel;
										deviceChannel = std::stoi(channel);
									}
									mAmbisonicsConfig.mLayout.addSpeaker(Speaker(deviceChannel, azim, elev, dist));
								}
							}
						}
					} else if (token == "/matrix/{") {
						std::getline(buffer,line,'\n');
						while(line.find("/}") == std::string::npos) {
							if (line.at(0) != '#') {
								stringstream matst(line);
								matst >> token;
								if (token == "order_gain") {
									double gain;
									int i = 0;
									while(matst.good()) {
										matst >> gain;
										mAmbisonicsConfig.mOrderGains.push_back(gain);
										mWOrder[i++] = gain;
									}
								} else if (token == "add_row") {
									mAmbisonicsConfig.mMatrixColumns = 0;
									while (matst.good()) {
										double coeff;
										matst >> coeff;
										mAmbisonicsConfig.mDecodeMatrix.push_back(coeff);
										mAmbisonicsConfig.mMatrixColumns++;
									}
								}
							}
							std::getline(buffer,line,'\n');
						}
					}
				}
			}
		}

		order(channelsToUniformOrder(mAmbisonicsConfig.mMatrixColumns));
		resizeArrays(channels(), mAmbisonicsConfig.mLayout.numSpeakers());
		updateChanWeights();
		for (int speaker = 0; speaker < mAmbisonicsConfig.mLayout.numSpeakers(); speaker++) {
			for (int channel = 0; channel < channels(); channel++) {
				mDecodeMatrix[speaker * channels() + channel] = mAmbisonicsConfig.mDecodeMatrix[speaker * channels() + channel ];
			}
		}
	} else {
		std::cout << "ERROR: Could not find ambdec configuration file: " << configFile << std::endl;
	}
	mSpeakers = &mAmbisonicsConfig.mLayout.speakers();
}
