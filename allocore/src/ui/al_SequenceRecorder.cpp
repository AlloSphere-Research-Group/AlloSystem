
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

#include "allocore/ui/al_SequenceRecorder.hpp"
#include "allocore/io/al_File.hpp"

using namespace al;

SequenceRecorder::SequenceRecorder() :
    mRecorderThread(nullptr), mMaxRecordTime(60.0)
{
}

//{
//	stopSequence();
//	std::string fullName = mDirectory;
//	if (fullName.back() != '/') {
//		fullName += "/";
//	}
//	fullName += sequenceName + ".sequence";
//	std::ifstream f(fullName);
//	if (!f.is_open()) {
//		std::cout << "Could not open:" << fullName << std::endl;
//		return;
//	}
//	while (!mSteps.empty()) {
//		mSteps.pop();
//	}
//	std::string line;
//	while (getline(f, line)) {
//		if (line.substr(0, 2) == "::") {
//			break;
//		}
//		std::stringstream ss(line);
//		std::string name, delta,duration;
//		std::getline(ss, name, ':');
//		std::getline(ss, delta, ':');
//		std::getline(ss, duration, ':');
//		if (name.size() > 0 && name[0] != '#') {
//			Step step;
//			step.presetName = name;
//			step.delta = std::stof(delta);
//			step.duration = std::stof(duration);
//			mSteps.push(step);
//			std::cout << name  << ":" << delta << ":" << duration << std::endl;
//		}
//	}
//	if (f.bad()) {
//		std::cout << "Error reading:" << sequenceName << std::endl;
//		return;
//	}
//	mSequenceConditionVar.notify_one();
//	//		mSequenceLock.unlock();
//	mRunning = true;
//}

//void SequenceRecorder::stopSequence()
//{
//	mRunning = false;
//	//		mSequenceLock.lock(); // Waits until the sequencer thread is done and back at the condition variable
//}

//std::vector<std::string> SequenceRecorder::getSequenceList()
//{
//	std::vector<std::string> sequenceList;
//	Dir presetDir(mDirectory);
//	while(presetDir.read()) {
//		FileInfo info = presetDir.entry();
//		if (info.type() == FileInfo::REG) {
//			std::string fileName = info.name();
//			if (fileName.find(".sequence") == fileName.size() - 9) {
//				// Should do better checks, what if '.sequence' is not at the end...
//				sequenceList.push_back(fileName.substr(0, fileName.size() - 9));
//			}
//		}
//	}
//	return sequenceList;
//}

void SequenceRecorder::startRecord(std::string name, bool overwrite)
{
	if (mRecorderThread) {
		stopRecord();
	}
	mPresetName = "";
	mOverwrite = overwrite;
	mRecording = true;
	mRecorderThread = new std::thread(SequenceRecorder::recorderFunction, this, name);

}

void SequenceRecorder::stopRecord()
{
	if (mRecorderThread) {
		mRecording = false;
		mSequenceConditionVar.notify_one();
		mRecorderThread->join();
		mRecorderThread = nullptr;
	}
}

void SequenceRecorder::presetChanged(int index, void *sender, void *userData)
{
	SequenceRecorder *recorder = static_cast<SequenceRecorder *>(userData);
	PresetHandler *presets = static_cast<PresetHandler *>(sender);
	std::lock_guard<std::mutex> lk(recorder->mSequenceLock);
	recorder->mPresetName = presets->getCurrentPresetName();
	recorder->mSequenceConditionVar.notify_one();
}

void SequenceRecorder::recorderFunction(SequenceRecorder *recorder, std::string sequenceName)
{
	std::vector<Step> steps;
	al_sec startTime = al::timeNow();
	while(recorder->mRecording && (al::timeNow() - startTime) < recorder->mMaxRecordTime) {
		al_sec previousTime = al::timeNow();
		{
			std::unique_lock<std::mutex> lk(recorder->mSequenceLock);
			recorder->mSequenceConditionVar.wait(lk);
			if (!recorder->mRecording) {
				break; // Don't write a step when closing or stopping record.
			}
		}
		al_sec currentTime = al::timeNow();
		al_sec timeDelta = currentTime - previousTime;
		if (steps.size() > 0) {
			Step &previousStep = steps.back();
			previousStep.duration = timeDelta - previousStep.delta;
		}
		Step step;
		step.presetName = recorder->mPresetName;
		step.delta =  recorder->mPresetHandler->getMorphTime();
		step.duration = 0.0;
		steps.push_back(step);
//		std::cout << step.presetName << ":" << step.duration << std::endl;
	}
	if (steps.size() < 2) {
		return;
	}
	if (sequenceName.size() < 1) {
		sequenceName = "new_seq";
	}
	std::string fileText;
	for(Step step: steps) {
		fileText += step.presetName + ":" +  std::to_string(step.delta) + ":" + std::to_string(step.duration) + "\n";
	}

	std::string path = recorder->mPresetHandler->getCurrentPath();
	if (path.back() != '/') {
		path += "/";
	}
	std::string fileName = path + sequenceName + ".sequence";

	std::string newSequenceName = sequenceName;
	if (!recorder->mOverwrite) {
		std::string newFileName = fileName;
		int counter = 0;
		while (File::exists(newFileName)) {
			newSequenceName = sequenceName + "_" + std::to_string(counter++);
			newFileName =  path + newSequenceName + ".sequence";
		}
		fileName = newFileName;
	}
	std::ofstream f(fileName);
	if (!f.is_open()) {
		std::cout << "Error while opening sequence file: " << fileName << std::endl;
		return;
	}
	f << fileText << "::" << std::endl;
	if (f.bad()) {
		std::cout << "Error while writing sequence file: " << fileName << std::endl;
	}
	f.close();

	std::cout << "Recorded: " << fileName << std::endl;
	recorder->mLastSequenceName = newSequenceName;
}



std::string SequenceRecorder::lastSequenceName()
{
	return mLastSequenceName;
}

bool SequenceRecorder::consumeMessage(osc::Message &m, std::string rootOSCPath)
{
	if(m.addressPattern() == rootOSCPath + "/startRecord" && m.typeTags() == "s"){
		std::string val;
		m >> val;
		std::cout << "/startRecord" << val << std::endl;
		startRecord(val);
		return true;
	} else if(m.addressPattern() == rootOSCPath + "/stopRecord") {
		std::cout << "/stopRecord" << std::endl;
		stopRecord();
		return true;
	} else if(m.addressPattern() == rootOSCPath + "/record" && m.typeTags() == "f"){
		float val;
		m >> val;
//		std::cout << "record sequence " << val << std::endl;
		if (val == 0.0f) {
			stopRecord();
			std::cout << "record sequence " << val << std::endl;
		} else {
			bool overwrite = (val > 0.0f);
			startRecord("NewSequence", overwrite);
		}
		return true;
	}
	return false;
}
