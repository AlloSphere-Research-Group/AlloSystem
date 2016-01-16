
#include <iostream>
#include <cassert>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "allocore/sound/al_Ambisonics.hpp"
#include "alloaudio/al_AmbiTunedDecoder.hpp"

using namespace al;
using namespace std;

class AmbisonicsDecoderConfig
{
public:
	string mDescription;
	int mVersion;

	string mDecChan_mask;
	int mDecfreq_bands;
	int mDecSpeakers;
	string mDecCoeff_scale;

	string mOptInput_scale;
	string mOptNfeff_comp;
	string mOptDelay_comp;
	string mOptLevel_comp;
	double mOptXover_freq;
	double mOptXover_ratio;

	SpeakerLayout mLayout;

	vector<double> mOrderGains;
	vector<double> mDecodeMatrix;
	int mMatrixColumns;
};

AmbiTunedDecoder::AmbiTunedDecoder(string configFile) :
    AmbiDecode(3, 3, 8)
{
	std::ifstream file(configFile);
	AmbisonicsDecoderConfig config;
	if (file) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		std::string line;

		while(std::getline(buffer,line,'\n')){
			if (!line.empty() && line.at(0) == '/') {
				stringstream ssin(line);
				string token;
				if (ssin.good()){
					ssin >> token;
					if (token == "/description") {
						ssin >> token;
						config.mDescription = token;
					} else if (token == "/version") {
						ssin >> token;
						config.mVersion = std::stoi(token);
					} else if (token == "/dec/chan_mask") {
						ssin >> token;
						config.mDecChan_mask = token;
					} else if (token == "/dec/freq_bands") {
						ssin >> token;
						config.mDecChan_mask = std::stoi(token);
					} else if (token == "/dec/speakers") {
						ssin >> token;
						config.mDecSpeakers = std::stoi(token);
					} else if (token == "/dec/coeff_scale") {
						ssin >> token;
						config.mDecCoeff_scale = token;
					} else if (token == "/opt/input_scale") {
						ssin >> token;
						config.mOptInput_scale = token;
					} else if (token == "/opt/nfeff_comp") {
						ssin >> token;
						config.mOptNfeff_comp = token;
					} else if (token == "/opt/delay_comp") {
						ssin >> token;
						config.mOptDelay_comp = token;
					} else if (token == "/opt/level_comp") {
						ssin >> token;
						config.mOptLevel_comp = token;
					} else if (token == "/opt/xover_freq") {
						ssin >> token;
						config.mOptXover_freq = std::stod(token);
					} else if (token == "/opt/xover_ratio") {
						ssin >> token;
						config.mOptXover_ratio = std::stod(token);
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
									config.mLayout.addSpeaker(Speaker(deviceChannel, azim, elev, dist));
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
										config.mOrderGains.push_back(gain);
										mWOrder[i] = gain;
									}
								} else if (token == "add_row") {
									config.mMatrixColumns = 0;
									while (matst.good()) {
										double coeff;
										matst >> coeff;
										config.mDecodeMatrix.push_back(coeff);
										config.mMatrixColumns++;
									}
								}
							}
							std::getline(buffer,line,'\n');
						}
					}
				}
			}
		}

		order(channelsToUniformOrder(config.mMatrixColumns));
//		mDim = 3;
		resizeArrays(channels(), config.mLayout.numSpeakers());

	}
}
