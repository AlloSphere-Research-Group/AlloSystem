#include <fstream>
#include <sstream>

#include "allocore/ui/al_Composition.hpp"
#include "allocore/system/al_Time.hpp"

using namespace al;

Composition::Composition(std::string fileName, std::string path) :
    mPath(path), mCompositionName(fileName), mPlayThread(nullptr),
    mSequencer(nullptr)
{
	std::string fullName = mPath;
	if (fullName.back() != '/') {
		fullName += "/";
	}
	fullName += mCompositionName + ".composition";
	std::ifstream f(fullName);
	if (!f.is_open()) {
		std::cout << "Could not open:" << fullName << std::endl;
		return;
	}
	std::string line;
	while (getline(f, line)) {
		if (line.substr(0, 2) == "::") {
			break;
		}
		std::stringstream ss(line);
		std::string name, delta;
		std::getline(ss, name, ':');
		std::getline(ss, delta, ':');
		if (name.size() > 0) {
			CompositionStep step;
			step.sequenceName = name;
			step.deltaTime = std::stof(delta);
			mCompositionSteps.push_back(step);
			// std::cout << name  << ":" << delta << std::endl;
		}
	}
	if (f.bad()) {
		std::cout << "Error reading:" << mCompositionName << std::endl;
		return;
	}
}

Composition::~Composition()
{
	mPlaying = false;
	if (mPlayThread) {
		mPlayThread->join();
		delete mPlayThread;
	}
}

void Composition::play()
{
	stop();
	mPlaying = true;
	mPlayThread = new std::thread(Composition::playbackThread, this);
}

void Composition::stop()
{
	if (mPlayThread) {
		mPlaying = false;
		mPlayThread->join();
		delete mPlayThread;
		mPlayThread = nullptr;
	}
	mSequencer->stopSequence();
}

std::string Composition::getName()
{
  return mCompositionName;
}

int Composition::size()
{
	return mCompositionSteps.size();
}

const CompositionStep Composition::getStep(int index)
{
	return mCompositionSteps[index];
}

void Composition::insertStep(std::string name, float deltaTime, int index)
{
	//TODO implement
}

void Composition::deleteStep(int index)
{
	//TODO implement
}

void Composition::write()
{
	std::string fileText;
	for(CompositionStep step: mCompositionSteps) {
		fileText += step.sequenceName + ":" +  std::to_string(step.deltaTime) + "\n";
	}
//	std::cout << fileText << std::endl;

	std::string path = mPath;
	if (path.back() != '/') {
		path += "/";
	}
	std::string fileName = path + mCompositionName + ".composition";
	std::ofstream f(fileName);
	if (!f.is_open()) {
		std::cout << "Error while opening composition file: " << fileName << std::endl;
		return;
	}
	f << fileText << "::" << std::endl;
	if (f.bad()) {
		std::cout << "Error while writing composition file: " << fileName << std::endl;
	}
	f.close();
}

void Composition::playbackThread(Composition *composition)
{
	size_t index = 0;
	if (composition->mCompositionSteps.size() == 0) {
		std::cout << "Composition has no steps. Done." << std::endl;
		return;
	}
	std::cout << "Composition started." << std::endl;

	const int granularity = 10; // milliseconds
	auto sequenceStart = std::chrono::high_resolution_clock::now();
	auto targetTime = sequenceStart;
	while (composition->mPlaying) {
		CompositionStep &step = composition->mCompositionSteps[index];
		//auto duration = //std::chrono::time_point_cast<std::chrono::nanoseconds>(step.deltaTime/1e9);
		targetTime += std::chrono::microseconds((int) (step.deltaTime*1e6 - (granularity * 1.5 * 1000.0)));
		while (std::chrono::high_resolution_clock::now() < targetTime) { // Granularity to allow more responsive stopping of composition playback
			std::this_thread::sleep_for(std::chrono::milliseconds(granularity));
			if (composition->mPlaying == false) {
				targetTime = std::chrono::high_resolution_clock::now();
				break;
			}
		}
		std::this_thread::sleep_until(targetTime);
		std::cout << "Composition step:" << step.sequenceName << ":" << step.deltaTime << std::endl;
		// std::cout << "Composition step:" << step.sequenceName << ":" << step.deltaTime << std::endl;
		composition->mSequencer->playSequence(step.sequenceName);
		index++;
		if (index == composition->mCompositionSteps.size()) {
			composition->mPlaying = false;
		}
	}
	std::cout << "Composition done." << std::endl;
}
