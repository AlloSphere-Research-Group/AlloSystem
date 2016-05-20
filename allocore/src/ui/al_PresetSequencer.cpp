
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/ui/al_PresetSequencer.hpp"

using namespace al;

void PresetSequencer::playSequence(std::string sequenceName)
{
	stopSequence();
	std::string fullName = mDirectory;
	if (fullName.back() != '/') {
		fullName += "/";
	}
	fullName += sequenceName + ".sequence";
	std::ifstream f(fullName);
	if (!f.is_open()) {
		std::cout << "Could not open:" << fullName << std::endl;
		return;
	}
	while (!mSteps.empty()) {
		mSteps.pop();
	}
	std::string line;
	while (getline(f, line)) {
		if (line.substr(0, 2) == "::") {
			break;
		}
		std::stringstream ss(line);
		std::string name, delta,duration;
		std::getline(ss, name, ':');
		std::getline(ss, delta, ':');
		std::getline(ss, duration, ':');
		if (name.size() > 0 && name[0] != '#') {
			Step step;
			step.presetName = name;
			step.delta = std::stof(delta);
			step.duration = std::stof(duration);
			mSteps.push(step);
			std::cout << name  << ":" << delta << ":" << duration << std::endl;
		}
	}
	if (f.bad()) {
		std::cout << "Error reading:" << sequenceName << std::endl;
		return;
	}
	mSequenceConditionVar.notify_one();
	//		mSequenceLock.unlock();
	mRunning = true;
}

void PresetSequencer::stopSequence()
{
	mRunning = false;
	//		mSequenceLock.lock(); // Waits until the sequencer thread is done and back at the condition variable
}

void PresetSequencer::sequencerFunction(al::PresetSequencer *sequencer)
{
	while(sequencer->mSequencerActive) {
		{
			std::unique_lock<std::mutex> lk(sequencer->mSequenceLock);
			sequencer->mSequenceConditionVar.wait(lk);
		}
		while(sequencer->running() && sequencer->mSteps.size() > 0) {
			Step &step = sequencer->mSteps.front();
			sequencer->mSteps.pop();
			sequencer->mPresetHandler->setMorphTime(step.delta);
			sequencer->mPresetHandler->recallPreset(step.presetName);
			//			std::cout << "Playing preset " << step.first << std::endl;
			al::wait(step.delta + step.duration);
		}
		sequencer->mRunning = false;
		//		std::cout << "Sequence finished." << std::endl;
	}
}

