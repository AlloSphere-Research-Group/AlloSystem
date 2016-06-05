
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/ui/al_PresetSequencer.hpp"
#include "allocore/io/al_File.hpp"

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

std::vector<std::string> al::PresetSequencer::getSequenceList()
{
	std::vector<std::string> sequenceList;
	Dir presetDir(mDirectory);
	while(presetDir.read()) {
		FileInfo info = presetDir.entry();
		if (info.type() == FileInfo::REG) {
			std::string fileName = info.name();
			if (fileName.find(".sequence") == fileName.size() - 9) {
				// Should do better checks, what if '.sequence' is not at the end...
				sequenceList.push_back(fileName.substr(0, fileName.size() - 9));
			}
		}
	}
	return sequenceList;
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
			sequencer->mPresetHandler->setMorphTime(step.delta);
			sequencer->mPresetHandler->recallPreset(step.presetName);
			al::wait(step.delta + step.duration);
			sequencer->mSteps.pop();
		}
		sequencer->mRunning = false;
		//		std::cout << "Sequence finished." << std::endl;
	}
}

// SequenceServer ----------------------------------------------------------------

SequenceServer::SequenceServer(std::string oscAddress, int oscPort) :
    mServer(nullptr), mSequencer(nullptr),
    mOSCpath("/sequence"),
    mParamServer(nullptr)
{
	mServer = new osc::Recv(oscPort, oscAddress.c_str(), 0.001); // Is this 1ms wait OK?
	if (mServer) {
		mServer->handler(*this);
		mServer->start();
	} else {
		std::cout << "Error starting OSC server." << std::endl;
	}
}


SequenceServer::SequenceServer(ParameterServer &paramServer) :
    mServer(nullptr), mSequencer(nullptr),
    mOSCpath("/sequence"),
    mParamServer(&paramServer)
{
	paramServer.registerOSCListener(this);
}

SequenceServer::~SequenceServer()
{
//	std::cout << "~SequenceServer()" << std::endl;;
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

void SequenceServer::onMessage(osc::Message &m)
{
	if(m.addressPattern() == mOSCpath && m.typeTags() == "s"){
		std::string val;
		m >> val;
		std::cout << "start sequence " << val << std::endl;
		if (mSequencer) {
			mSequencer->playSequence(val);
		} else {
			std::cout << "Sequence Server. OSC received, but PresetSequencer not registered." << std::endl;
		}
	}
}

void SequenceServer::print()
{
	if (mServer) {
		std::cout << "Sequence server listening on: " << mServer->address() << ":" << mServer->port() << std::endl;
		std::cout << "Communicating on path: " << mOSCpath << std::endl;
	}
	for (auto sender:mOSCSenders) {
		std::cout << sender->address() << ":" << sender->port() << std::endl;
	}
}

void SequenceServer::stopServer()
{
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

void SequenceServer::setAddress(std::string address)
{
	mOSCpath = address;
}

std::string SequenceServer::getAddress()
{
	return mOSCpath;
}

void SequenceServer::changeCallback(int value, void *sender, void *userData)
{
	SequenceServer *server = static_cast<SequenceServer *>(userData);
	Parameter *parameter = static_cast<Parameter *>(sender);
	server->notifyListeners(server->mOSCpath, value);
}


