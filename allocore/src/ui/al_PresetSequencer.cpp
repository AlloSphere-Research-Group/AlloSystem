
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/ui/al_PresetSequencer.hpp"
#include "allocore/ui/al_SequenceRecorder.hpp"
#include "allocore/ui/al_Composition.hpp"
#include "allocore/io/al_File.hpp"

using namespace al;

void PresetSequencer::playSequence(std::string sequenceName)
{
	stopSequence();
	mSequenceLock.lock();
	//		while (!mSteps.empty()) {
	//			mSteps.pop();
	//		}
	if (sequenceName.size() > 0) {
		std::queue<Step> steps = loadSequence(sequenceName);
		mSteps = steps;
	}
	mRunning = true;
	mSequenceLock.unlock();
	mCurrentSequence = sequenceName;
	{
		std::unique_lock<std::mutex> lk(mPlayWaitLock);
		mSequencerThread = new std::thread(PresetSequencer::sequencerFunction, this);
		mPlayWaitVariable.wait(lk);
	}

//	std::thread::id seq_thread_id = mSequencerThread->get_id();
//	std::cout << "Preset Sequencer thread id: " << std::hex << seq_thread_id << std::endl;
}

void PresetSequencer::stopSequence(bool triggerCallbacks)
{
	if (mRunning == true) {
		mRunning = false;
		bool mCallbackStatus = false;
		if (!triggerCallbacks) {
			mCallbackStatus = mEndCallbackEnabled;
			enableEndCallback(false);
		}
		if (mSequencerThread) {
			std::thread *th = mSequencerThread;
			mSequencerThread = nullptr;
			th->join();
			delete th;
		}
		if (!triggerCallbacks) {
			enableEndCallback(mCallbackStatus);
		}
	}
}

bool PresetSequencer::archiveSequence(std::string sequenceName, bool overwrite)
{
	bool ok = true;
	std::string fullPath = buildFullPath(sequenceName) + "_archive";
	if (mPresetHandler == nullptr) {
		std::cerr << "A Preset Handler must be registered to store sequences. Aborting." << std::endl;
		return false;
	}
	if (overwrite) {
		if(File::isDirectory(fullPath)) {
			if (!Dir::removeRecursively(fullPath)) {
				std::cout << "Error removing directory: " << fullPath << " aborting sequence archiving." << std::endl;
				return false;
			}
		} else {
			if (File::remove(fullPath) != 0) {
				std::cout << "Error removing file: " << fullPath << " aborting sequence archiving." << std::endl;
				return false;
			}
		}
		if (!Dir::make(fullPath)) {
			std::cout << "Error creating sequence archive directory " << fullPath << std::endl;
			return false;
		}
	} else {
		int counter = 0;
		while (File::isDirectory(fullPath)) {
			std::string newName = sequenceName + "_" + std::to_string(counter++);
			fullPath = buildFullPath(newName) + "_archive";
			if (counter == 0) { // We've wrapped and run out of names...
				std::cout << "Out of names for sequence archive." << std::endl;
				return false;
			}
		}
		if (!Dir::make(fullPath)) {
			std::cout << "Error creating sequence archive directory " << fullPath << std::endl;
			return false;
		}
	}
	std::queue<Step> steps = loadSequence(sequenceName);
	while(steps.size() > 0) {
		Step &step = steps.front();
		std::string presetFilename = mPresetHandler->getCurrentPath() + step.presetName + ".preset";
		if (!File::copy(presetFilename, fullPath)) {
			std::cout << "Error copying preset " << presetFilename << " when archiving." << std::endl;
			ok = false;
		}
		steps.pop();
	}
	if (!File::copy(buildFullPath(sequenceName), fullPath)) {
		std::cout << "Error copying sequence " << sequenceName << " when archiving." << std::endl;
		ok = false;
	}

	return ok;
}

std::vector<std::string> al::PresetSequencer::getSequenceList()
{
	std::vector<std::string> sequenceList;
	std::string path = mDirectory;
	if (mPresetHandler) {
		path = mPresetHandler->getCurrentPath();
	}
    if (!File::isDirectory(path)) {
        Dir::make(path, true);
    }
	Dir presetDir(path);
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
	{
		std::lock_guard<std::mutex> lk(sequencer->mPlayWaitLock);
		if (sequencer->mBeginCallbackEnabled && sequencer->mBeginCallback != nullptr) {
			sequencer->mBeginCallback(sequencer, sequencer->mBeginCallbackData);
		}
		sequencer->mPlayWaitVariable.notify_one();
	}
	const int granularity = 10; // milliseconds
	sequencer->mSequenceLock.lock();
	auto sequenceStart = std::chrono::high_resolution_clock::now();
	auto targetTime = sequenceStart;
	while(sequencer->running() && sequencer->mSteps.size() > 0) {
		Step &step = sequencer->mSteps.front();
		if (step.type == PRESET) {
			if (sequencer->mPresetHandler) {
				sequencer->mPresetHandler->setMorphTime(step.delta);
				sequencer->mPresetHandler->recallPreset(step.presetName);
			} else {
				std::cerr << "No preset handler registered. Ignoring preset change." << std::endl;
			}
//			std::cout << "PresetSequencer loading:" << step.presetName << std::endl;
//			std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - sequenceStart).count() / 1000.0 << std::endl;
			float totalWaitTime = step.delta + step.duration;
			targetTime += std::chrono::microseconds((int) (totalWaitTime*1.0e6 - (granularity * 1.5 * 1.0e3)));

			while (std::chrono::high_resolution_clock::now() < targetTime) { // Granularity to allow more responsive stopping of composition playback
				//std::cout << std::chrono::high_resolution_clock::to_time_t(targetTime)
				//	    << "---" << std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now()) << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(granularity));
				if (sequencer->mRunning == false) {
					targetTime = std::chrono::high_resolution_clock::now();
					break;
				}
			}
		} else if (step.type == EVENT) {
			float totalWaitTime = step.delta + step.duration;
			targetTime += std::chrono::microseconds((int) (totalWaitTime*1.0e6 - (granularity * 1.5 * 1.0e3)));

			while (std::chrono::high_resolution_clock::now() < targetTime) { // Granularity to allow more responsive stopping of composition playback
				//std::cout << std::chrono::high_resolution_clock::to_time_t(targetTime)
				//	    << "---" << std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now()) << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(granularity));
				if (sequencer->mRunning == false) {
					targetTime = std::chrono::high_resolution_clock::now();
					break;
				}
			}
			if (sequencer->mRunning) {
				for (auto eventCallback: sequencer->mEventCallbacks) {
					if (eventCallback.eventName == step.presetName) {
						eventCallback.callback(eventCallback.callbackData, step.params);
						break;
					}
				}
			}
		}
		sequencer->mSteps.pop();
//		std::this_thread::sleep_until(targetTime);
		// std::this_thread::sleep_for(std::chrono::duration<float>(totalWaitTime));
	}
	if (sequencer->mPresetHandler) {
		sequencer->mPresetHandler->stopMorph();
	}
//	std::cout << "Sequence finished." << std::endl;
	sequencer->mRunning = false;
	sequencer->mSequenceLock.unlock();
	if (sequencer->mEndCallbackEnabled && sequencer->mEndCallback != nullptr) {
		bool finished = sequencer->mSteps.size() == 0;
		sequencer->mEndCallback(finished, sequencer, sequencer->mEndCallbackData);
	}
}


void PresetSequencer::setHandlerSubDirectory(std::string subDir)
{
	if (mPresetHandler) {
		mPresetHandler->setSubDirectory(subDir);
	} else {
		std::cerr << "Error in PresetSequencer::setHandlerSubDirectory. PresetHandler not registered." << std::endl;
	}
}

std::queue<PresetSequencer::Step> PresetSequencer::loadSequence(std::string sequenceName)
{
	std::queue<Step> steps;
	std::string fullName = buildFullPath(sequenceName);
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
		//FIXME here and in other sequencers white space should be stripped out
		std::stringstream ss(line);
		std::string name, delta,duration;
		std::getline(ss, name, ':');
		std::getline(ss, delta, ':');
		std::getline(ss, duration, ':');
		std::cout << line << std::endl;
		if (name.size() > 0 && name[0] == '@') {
			Step step;
			step.type = EVENT;
			step.presetName = name.substr(1); // chop initial '@'
			step.delta = std::stof(delta);
			step.duration = std::stof(duration);

			//FIXME allow any number or parameters
			std::string next;
			std::getline(ss, next, ':');
			step.params.push_back(std::stof(next));
			steps.push(step);
//			 std::cout << name  << ":" << delta << ":" << duration << std::endl;
		} else if (name.size() > 0 && name[0] != '#') {
			Step step;
			step.presetName = name;
			step.delta = std::stof(delta);
			step.duration = std::stof(duration);
			steps.push(step);
			// std::cout << name  << ":" << delta << ":" << duration << std::endl;
		}
	}
	if (f.bad()) {
		std::cout << "Error reading:" << sequenceName << std::endl;
	}
	return steps;
}

void PresetSequencer::registerEventCommand(std::string eventName,
                                           std::function<void (void *, std::vector<float> &params)> callback,
                                           void *data)
{
	EventCallback cb;
	cb.eventName = eventName;
	cb.callback = callback;
	cb.callbackData = data;

	mEventCallbacks.push_back(cb);
}

void PresetSequencer::registerBeginCallback(std::function<void(PresetSequencer *sender, void *userData)> beginCallback,
                                            void *userData)
{
	mBeginCallback = beginCallback;
	mBeginCallbackData = userData;
	mBeginCallbackEnabled = true;
}

void PresetSequencer::registerEndCallback(std::function<void (bool, al::PresetSequencer *, void *)> endCallback,
                                          void *userData)
{
	// FIXME this data needs to be protected with a mutex
	mEndCallback = endCallback;
	mEndCallbackData = userData;
	mEndCallbackEnabled = true;
}

float PresetSequencer::getSequenceTotalDuration(std::string sequenceName)
{
	std::queue<Step> steps = loadSequence(sequenceName);
	float duration = 0.0f;
	while (steps.size() > 0) {
		const Step &step = steps.front();
		duration += step.delta + step.duration;
		steps.pop();
	}
	return duration;
}

void PresetSequencer::clearSteps()
{
	stopSequence();
	mSequenceLock.lock();
	while (!mSteps.empty()) {
		mSteps.pop();
	}
	mSequenceLock.unlock();
}

void PresetSequencer::appendStep(PresetSequencer::Step &newStep)
{
	mSequenceLock.lock();
	mSteps.push(newStep);
	mSequenceLock.unlock();
}

bool PresetSequencer::consumeMessage(osc::Message &m, std::string rootOSCPath)
{
	std::string basePath = rootOSCPath;
	if (mOSCsubPath.size() > 0) {
		basePath += "/" + mOSCsubPath;
	}
	if(m.addressPattern() == basePath && m.typeTags() == "s"){
		std::string val;
		m >> val;
		std::cout << "start sequence " << val << std::endl;
		playSequence(val);
		return true;
	} else if(m.addressPattern() == basePath + "/stop" ){
		std::cout << "stop sequence " << std::endl;
		stopSequence();
		return true;
	}
	return false;
}

std::string PresetSequencer::buildFullPath(std::string sequenceName)
{
	std::string fullName = mDirectory;
	if (mPresetHandler) {
		fullName = mPresetHandler->getCurrentPath();
	}
	if (fullName.back() != '/') {
		fullName += "/";
	}
	if (sequenceName.size() < 9 || sequenceName.substr(sequenceName.size() - 9) != ".sequence") {
		fullName += sequenceName + ".sequence";
	}
	return fullName;
}


// SequenceServer ----------------------------------------------------------------

SequenceServer::SequenceServer(std::string oscAddress, int oscPort) :
    mServer(nullptr), mRecorder(nullptr),
    mParamServer(nullptr),
    mOSCpath("/sequence")
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
    mServer(nullptr),
    mParamServer(&paramServer),
    mOSCpath("/sequence")
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
	if(m.addressPattern() == mOSCpath + "/last"){
		if (mSequencer && mRecorder) {
			std::cout << "start last recorder sequence " << mRecorder->lastSequenceName() << std::endl;
			mSequencer->setHandlerSubDirectory(mRecorder->lastSequenceSubDir());
			mSequencer->playSequence( mRecorder->lastSequenceName());
		} else {
			std::cerr << "SequenceRecorder and PresetSequencer must be registered to enable /*/last." << std::endl;
		}
	} else {
		for(osc::MessageConsumer *consumer: mConsumers) {
			if (consumer->consumeMessage(m, mOSCpath)) {
				break;
			}
		}
	}
}

SequenceServer &SequenceServer::registerMessageConsumer(osc::MessageConsumer &consumer) {
	mConsumers.push_back(&consumer);
	return *this;
}

SequenceServer &SequenceServer::registerRecorder(SequenceRecorder &recorder) {
	mRecorder = &recorder;
	mConsumers.push_back(static_cast<osc::MessageConsumer *>(&recorder));
	return *this;
}

SequenceServer &SequenceServer::registerSequencer(PresetSequencer &sequencer) {
	mSequencer = &sequencer;
	mConsumers.push_back(&sequencer);
	return *this;
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

