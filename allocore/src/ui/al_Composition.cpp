#include <fstream>
#include <sstream>
#include <iostream>

#include "allocore/ui/al_Composition.hpp"
#include "allocore/system/al_Time.hpp"
#include "allocore/io/al_File.hpp"

using namespace al;

Composition::Composition(std::string fileName, std::string path) :
    mPath(path), mSubDirectory(""), mCompositionName(fileName), mPlayThread(nullptr),
    mSequencer(nullptr)
{
	mCompositionSteps = loadCompositionSteps(mCompositionName);
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
	mPlayerLock.lock();
	stop();
	mCompositionSteps = loadCompositionSteps(mCompositionName);
	mPlaying = true;
	mPlayThread = new std::thread(Composition::playbackThread, this);
	mPlayerLock.unlock();
}

void Composition::stop()
{
	if (mPlayThread) {
		mPlaying = false;
		mPlayThread->join();
		delete mPlayThread;
		mPlayThread = nullptr;
	}
	mSequencer->stopSequence(); // Sequencer should also stop if in the middle of a sequence.
}

bool Composition::playArchive(std::string archiveName)
{
	std::string compositionName = archiveName;
	if (compositionName.size() > 20 && compositionName.substr(compositionName.size() - 20) == ".composition_archive") {
		compositionName = compositionName.substr(0, compositionName.size() - 20);
	} else {
		archiveName += ".composition_archive";
	}
	std::cout << getRootPath() + archiveName << "--" << getRootPath() + archiveName + "/" + compositionName << std::endl;
	if (File::isDirectory(getRootPath() + archiveName) && File::exists(getRootPath() + archiveName + "/" + compositionName + ".composition")) {
		mPlayerLock.lock();
		stop();
		setSubDirectory(archiveName);
		mCompositionName = compositionName;

		mCompositionSteps = loadCompositionSteps(mCompositionName);
		mPlayerLock.unlock();
		play();
	}
	return true;
}

std::string Composition::getName()
{
	return mCompositionName;
}

bool Composition::archiveComposition()
{
	std::string path = getCurrentPath();
	std::string compositionName = mCompositionName;
	bool ok = true;

	std::string fullPath = path;
	fullPath += compositionName + ".composition";

	std::string archivePath = fullPath;
	if (archivePath.size() > 8 && archivePath.substr(archivePath.size() - 8) != "_archive") {
		archivePath += "_archive";
	}

	if (File::isDirectory(archivePath) ) {
		if (!Dir::removeRecursively(archivePath)) {
			std::cerr << "Error removing directory: " << fullPath << " aborting composition archiving." << std::endl;
			return false;
		}
		if (!Dir::make(archivePath)) {
			std::cerr << "Error creating composition archive directory " << fullPath << std::endl;
			return false;
		}

	} else if (!File::exists(archivePath)) {
		if (!Dir::make(archivePath)) {
			std::cerr << "Error creating compostion archive directory " << archivePath << std::endl;
			return false;
		}

	} else {
		// TODO generate new name instead of error
		std::cerr << "compostion directory name taken by file: " << archivePath << std::endl;
		return false;
	}

	auto steps = loadCompositionSteps(compositionName);

	for (auto step: steps) {
		std::cout << step.sequenceName << ":" << step.deltaTime << std::endl;
		PresetSequencer sequencer;
		sequencer.setDirectory(path);
		std::queue<PresetSequencer::Step> sequenceSteps = sequencer.loadSequence(step.sequenceName);
		if (!File::copy(path + step.sequenceName + ".sequence",
		                archivePath + "/" + step.sequenceName + ".sequence")) {
			std::cerr << "Error copying sequence: " << step.sequenceName << " when archiving composition." << std::endl;
			ok = false;
		}
		while(sequenceSteps.size() > 0) {
			PresetSequencer::Step &sequenceStep = sequenceSteps.front();
			if (!File::exists(archivePath + "/" + sequenceStep.presetName + ".preset")
			        && !File::copy(path + sequenceStep.presetName + ".preset",
			                archivePath + "/" + sequenceStep.presetName + ".preset")) {
				std::cerr << "Error copying preset: " << sequenceStep.presetName << " when archiving composition." << std::endl;
				ok = false;
			}
			sequenceSteps.pop();
		}
	}
	if (!File::copy(fullPath,
	                archivePath + "/" + compositionName + ".composition")) {
		 std::cerr << "Error copying composition: " << compositionName << " when archiving composition." << std::endl;
		 ok = false;
	 }

	return true;
}

bool Composition::archive(std::string compositionName, std::string path, bool overwrite)
{
	std::cout << "archive() not implemented" <<std::endl;
	// TODO implement
	return false;

}

bool Composition::restore(std::string compositionName, std::string path, bool overwrite)
{
	std::cout << "restore() not implemented" <<std::endl;
	// TODO implement
	return false;
}

int Composition::size()
{
	mPlayerLock.lock();
	int size = mCompositionSteps.size();
	mPlayerLock.unlock();
	return size;
}

const CompositionStep Composition::getStep(int index)
{
	mPlayerLock.lock();
	CompositionStep step;
	if (index > 0 && index < size()) {
		step = mCompositionSteps[index];
	}
	mPlayerLock.unlock();
	return step;
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

	std::string path = getCurrentPath();
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

void Composition::registerBeginCallback(std::function<void (Composition *, void *)> beginCallback,
                                        void *userData)
{

	mBeginCallback = beginCallback;
	mBeginCallbackData = userData;
	mBeginCallbackEnabled = true;
}

void Composition::registerEndCallback(std::function<void (bool, Composition *, void *)> endCallback,
                                      void *userData)
{
	mEndCallback = endCallback;
	mEndCallbackData = userData;
	mEndCallbackEnabled = true;
}

bool Composition::consumeMessage(osc::Message &m, std::string rootOSCPath)
{
	if(m.addressPattern() == rootOSCPath + "/composition/playArchive" && m.typeTags() == "s"){
		std::string val;
		m >> val;std::cout << "play composition archive" << val << std::endl;
		playArchive(val);
		return true;
	} else if(m.addressPattern() == rootOSCPath + "/composition" && m.typeTags() == "s"){
		std::string val;
		m >> val;
		if (val == getName()) {
			std::cout << "play composition " << val << std::endl;
			play();
			return true;
		}
	} else if(m.addressPattern() == rootOSCPath + "/composition/stop" && m.typeTags() == "s"){
		std::string val;
		m >> val;
		if (val == getName()) {
			std::cout << "stop composition " << val << std::endl;
			stop();
			return true;
		}
	} else if(m.addressPattern() == rootOSCPath + "/composition/stop"){
		stop();
		return true;
	}
	return false;
}

std::vector<al::CompositionStep> Composition::loadCompositionSteps(std::string compositionName)
{
	std::vector<al::CompositionStep> steps;
	std::string fullName = getCurrentPath();

	if (fullName.size() > 12 && !(fullName.substr(fullName.size() - 12) == ".composition") ) {
		fullName += compositionName + ".composition";
	}
	std::ifstream f(fullName);
	if (!f.is_open()) {
		std::cout << "Could not open:" << fullName << std::endl;
		return steps;
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
			steps.push_back(step);
			// std::cout << name  << ":" << delta << std::endl;
		}
	}
	if (f.bad()) {
		std::cout << "Error reading:" << mCompositionName << std::endl;
	}
	return steps;
}

std::string Composition::getRootPath()
{
	std::string path = mPath;
	if (path.back() != '/' && path.size() > 0) {
		path += "/";
	}
	return path;
}

std::string Composition::getCurrentPath()
{
	std::string path = getRootPath();
	path += mSubDirectory;
	if (path.back() != '/' && path.size() > 0) {
		path += "/";
	}
	return path;
}

void Composition::playbackThread(Composition *composition)
{
	size_t index = 0;
	if (composition->mCompositionSteps.size() == 0) {
		std::cout << "Composition has no steps. Done." << std::endl;
		return;
	}
	std::cout << "Composition started." << std::endl;

	composition->mSequencer->toggleEnableBeginCallback();
	composition->mSequencer->toggleEnableEndCallback();
	if (composition->mBeginCallbackEnabled && composition->mBeginCallback != nullptr) {
		composition->mBeginCallback(composition, composition->mBeginCallbackData);
	}
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
	if (composition->mEndCallbackEnabled && composition->mEndCallback != nullptr) {
		bool finished = index == composition->mCompositionSteps.size();
		composition->mEndCallback(finished, composition, composition->mEndCallbackData);
	}
	composition->mSequencer->toggleEnableBeginCallback();
	composition->mSequencer->toggleEnableEndCallback();
	std::cout << "Composition done." << std::endl;
}
