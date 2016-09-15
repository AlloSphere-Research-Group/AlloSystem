
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/ui/al_Preset.hpp"
#include "allocore/io/al_File.hpp"

using namespace al;


// PresetHandler --------------------------------------------------------------

PresetHandler::PresetHandler(std::string rootDirectory, bool verbose) :
    mRootDir(rootDirectory), mVerbose(verbose),
    mRunning(true), mMorphingThread(PresetHandler::morphingFunction, this),
    mMorphInterval(0.05), mMorphTime("morphTime", "", 0.0, "", 0.0, 20.0)
{
	if (!File::exists(rootDirectory)) {
		if (!Dir::make(rootDirectory, true)) {
			std::cout << "Error creating directory: " << rootDirectory << std::endl;
		}
	}
	loadPresetMap();
}

PresetHandler::~PresetHandler()
{
	mRunning = false;
//	std::lock_guard<std::mutex> lk(mTargetLock);
	mMorphConditionVar.notify_one();
	mMorphingThread.join();
}

PresetHandler &PresetHandler::registerParameter(Parameter &parameter)
{
	mParameters.push_back(&parameter);
	return *this;
}

void PresetHandler::setSubDirectory(std::string directory)
{
	mSubDir = directory;
	std::string path = getCurrentPath();
	if (!File::exists(path)) {
		if (!Dir::make(path, true)) {
			std::cout << "Error creating directory: " << mRootDir << std::endl;
		}
	}
	loadPresetMap();
}

std::vector<std::string> PresetHandler::availableSubDirectories()
{
	std::vector<std::string> subDirList;
	Dir presetDir(mRootDir);
	while(presetDir.read()) {
		FileInfo info = presetDir.entry();
		if (info.type() == FileInfo::DIR) {
			subDirList.push_back(info.name());
		}
	}
	return subDirList;
}

void PresetHandler::registerPresetCallback(PresetHandler::PresetChangeCallback cb, void *userData)
{
	mCallbacks.push_back(cb);
	mCallbackUdata.push_back(userData);
}

void PresetHandler::registerMorphTimeCallback(Parameter::ParameterChangeCallback cb,
                                              void *userData)
{
	mMorphTime.registerChangeCallback(cb, userData);
}

void PresetHandler::storePreset(std::string name)
{
	int index = -1;
	for (auto preset: mPresetsMap) {
		if (preset.second == name) {
			index = preset.first;
			break;
		}
	}
	if (index < 0) {
		for (auto preset: mPresetsMap) {
			if (index <= preset.first) {
				index = preset.first + 1;
			}
		} // FIXME this should look for the first "empty" preset.
	}
	storePreset(index, name);
}

void PresetHandler::storePreset(int index, std::string name)
{
	// TODO need to check if name contains ':' and remove as this causes
	// issues with the text format for saving
	std::lock_guard<std::mutex> lock(mFileLock);
	std::string path = getCurrentPath();
	if (path.back() != '/') {
		path += "/";
	}
	std::ofstream f(path + name + ".preset");
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset file: " << mFileName << std::endl;
		}
		return;
	}
	f << "::" + name << std::endl;
	for(Parameter *parameter: mParameters) {
		std::string line = parameter->getFullAddress() + " f " + std::to_string(parameter->get());
		f << line << std::endl;
	}
	f << "::" << std::endl;
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset file: " << mFileName << std::endl;
		}
	}
	f.close();
	mPresetsMap[index] = name;
	storePresetMap();
}

void PresetHandler::recallPreset(std::string name)
{
	{
		std::lock_guard<std::mutex> lk(mTargetLock);
		mTargetValues = loadPresetValues(name);
		mMorphRemainingSteps =  1 + mMorphTime.get() / mMorphInterval;
	}
	mMorphConditionVar.notify_one();

	int index = -1;
	for (auto preset: mPresetsMap) {
		if (preset.second == name) {
			index = preset.first;
			break;
		}
	}
	mCurrentPresetName = name;
	for(int i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](index, this, mCallbackUdata[i]);
		}
	}
}

std::string PresetHandler::recallPreset(int index)
{
	auto presetNameIt = mPresetsMap.find(index);
	if (presetNameIt != mPresetsMap.end()) {
		recallPreset(presetNameIt->second);
		return presetNameIt->second;
	} else {
		std::cout << "No preset index " << index << std::endl;
	}
	return "";
}

std::map<int, std::string> PresetHandler::availablePresets()
{
	return mPresetsMap;
}

std::string al::PresetHandler::getPresetName(int index)
{
	return mPresetsMap[index];
}

float al::PresetHandler::getMorphTime()
{
	return mMorphTime.get();
}

void PresetHandler::setMorphTime(float time)
{
	mMorphTime.set(time);
}

std::string PresetHandler::getCurrentPath()
{
	std::string relPath = mRootDir;
	if (relPath.back() != '/') {
		relPath += "/";
	}
	relPath += mSubDir;
	return relPath;
}

void PresetHandler::loadPresetMap()
{
	std::string mapFileName = getCurrentPath();
	if (mapFileName.back() != '/') {
		mapFileName += "/";
	}
	mapFileName += "_presetMap.txt";
	mPresetsMap.clear();
	if (!File::exists(mapFileName)) {
		std::cout << "No preset map. Creating default." << std::endl;
		std::vector<std::string> presets;
		Dir presetDir(getCurrentPath());
		while(presetDir.read()) {
			FileInfo info = presetDir.entry();
			if (info.type() == FileInfo::REG) {
				std::string name = info.name();
				if (name.substr(name.size()-7) == ".preset") {
					presets.push_back(info.name().substr(0, name.size()-7));
				}
			}
		}
		for(int i = 0; i < presets.size(); ++i) {
			mPresetsMap[i] = presets[i];
		}
		storePresetMap();
	} else {
		std::ifstream f(mapFileName);
		if (!f.is_open()) {
			if (mVerbose) {
				std::cout << "Error while opening preset map file for reading: " << mapFileName << std::endl;
			}
			return;
		}
		std::string line;
		while (getline(f, line)) {
			if (line.substr(0, 2) == "::") {
				if (mVerbose) {
					std::cout << "End preset map."<< std::endl;
				}
				break;
			}
			std::stringstream ss(line);
			std::string index, name;
			std::getline(ss, index, ':');
			std::getline(ss, name, ':');
			mPresetsMap[std::stoi(index)] = name;
//			std::cout << index << ":" << name << std::endl;
		}
		if (f.bad()) {
			if (mVerbose) {
				std::cout << "Error while writing preset map file for reading: " << mFileName << std::endl;
			}
		}

		std::cout << "Reading preset map." << std::endl;
	}
}

void PresetHandler::storePresetMap()
{
	std::string mapFileName = getCurrentPath();
	if (mapFileName.back() != '/') {
		mapFileName += "/";
	}
	mapFileName += "_presetMap.txt";
	std::ofstream f(mapFileName);
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset map file: " << mapFileName << std::endl;
		}
		return;
	}
	for(auto const &preset : mPresetsMap) {
		std::string line = std::to_string(preset.first) + ":" + preset.second;
		f << line << std::endl;
	}
	f << "::" << std::endl;
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset map file: " << mFileName << std::endl;
		}
	}
	f.close();
}

void PresetHandler::morphingFunction(al::PresetHandler *handler) {
	while(handler->mRunning) {
		{
			std::unique_lock<std::mutex> lk(handler->mTargetLock);
			handler->mMorphConditionVar.wait(lk);
//			lk.unlock();
		}
		while (--handler->mMorphRemainingSteps >= 0) {
			for (Parameter *param: handler->mParameters) {
				float paramValue = param->get();
				if (handler->mTargetValues.find(param->getFullAddress()) != handler->mTargetValues.end()) {
					float difference =  handler->mTargetValues[param->getFullAddress()] - paramValue;
					if (handler->mMorphRemainingSteps == 0) {
						difference = difference;
					} else {
						difference = difference/(handler->mMorphRemainingSteps + 1);
					}
					float newVal = paramValue + difference;
					param->set(newVal);
				}
			}
			al::wait(handler->mMorphInterval);
		}
//		// Set final values
//		for (Parameter param: mParameters) {
//			if (preset.find(param.getName()) != preset.end()) {
//				param.set(preset[param.getName()]);
//			}
//		}
	}
}

std::map<std::string, float> PresetHandler::loadPresetValues(std::string name)
{
	std::map<std::string, float> preset;
	std::lock_guard<std::mutex> lock(mFileLock);
	std::string path = getCurrentPath();
	if (path.back() != '/') {
		path += "/";
	}
	std::string line;
	std::ifstream f(path + name + ".preset");
	if (!f.is_open()) {
		if (mVerbose) {
			std::cout << "Error while opening preset file: " << mFileName << std::endl;
		}
	}
	while(getline(f, line)) {
		if (line.substr(0, 2) == "::") {
			if (mVerbose) {
				std::cout << "Found preset : " << line << std::endl;
			}
			while (getline(f, line)) {
				if (line.substr(0, 2) == "::") {
					if (mVerbose) {
						std::cout << "End preset."<< std::endl;
					}
					break;
				}
				bool parameterFound = false;
				std::stringstream ss(line);
				std::string address, type, value;
				std::getline(ss, address, ' ');
				std::getline(ss, type, ' ');
				std::getline(ss, value, ' ');
				for(Parameter *param: mParameters) {
					if (mVerbose) {
						std::cout << "Checking for parameter match: " << param->getFullAddress() << std::endl;
					}
					if (param->getFullAddress() == address) {
						if (type == "f") {
							preset.insert(std::pair<std::string,float>(address,std::stof(value)));
							parameterFound = true;
							break;
						}
					}
				}
				if (!parameterFound && mVerbose) {
					std::cout << "Preset in parameter not present: " << address << std::endl;
				}
			}
		}
	}
	if (f.bad()) {
		if (mVerbose) {
			std::cout << "Error while writing preset file: " << mFileName << std::endl;
		}
	}
	f.close();
	return preset;
}


// PresetServer ----------------------------------------------------------------



PresetServer::PresetServer(std::string oscAddress, int oscPort) :
    mServer(nullptr), mPresetHandler(nullptr), mOSCpath("/preset"), mParamServer(nullptr)
{
	mServer = new osc::Recv(oscPort, oscAddress.c_str(), 0.001); // Is this 1ms wait OK?
	if (mServer) {
		mServer->handler(*this);
		mServer->start();
	} else {
		std::cout << "Error starting OSC server." << std::endl;
	}
}


PresetServer::PresetServer(ParameterServer &paramServer) :
    mServer(nullptr), mPresetHandler(nullptr), mOSCpath("/preset"), mParamServer(&paramServer)
{
	paramServer.registerOSCListener(this);
//	paramServer.mServer->stop();
//	paramServer.mServer->handler(*this);
//	paramServer.mServer->start();
}

PresetServer::~PresetServer()
{
//	std::cout << "~PresetServer()" << std::endl;;
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

void PresetServer::onMessage(osc::Message &m)
{
	if(m.addressPattern() == mOSCpath && m.typeTags() == "f"){
		float val;
		m >> val;
		mPresetHandler->recallPreset((int) val);
		//			std::cout << "ParameterServer::onMessage" << val << std::endl;
	} else if(m.addressPattern() == mOSCpath && m.typeTags() == "i"){
		int val;
		m >> val;
		mPresetHandler->recallPreset(val);
	} else if (m.addressPattern() == mOSCpath + "/morphTime" && m.typeTags() == "f")  {
		float val;
		m >> val;
		mPresetHandler->setMorphTime(val);
	} else if (m.addressPattern().substr(0, mOSCpath.size() + 1) == mOSCpath + "/") {
		int index = std::stoi(m.addressPattern().substr(mOSCpath.size() + 1));
		if (m.typeTags() == "f") {
			float val;
			m >> val;
			if (val == 1) {
				mPresetHandler->recallPreset(index);
			}
		}
	}
//	else if (mParamServer) {
//		m.resetStream();
//		mParamServer->onMessage(m);
//	}
}

void PresetServer::print()
{
	if (mServer) {
		std::cout << "Preset server listening on: " << mServer->address() << ":" << mServer->port() << std::endl;
		std::cout << "Communicating on path: " << mOSCpath << std::endl;
		std::cout << "Registered listeners: " << std::endl;

	} else {
		std::cout << "Preset Server Connected to shared server." << std::endl;
	}
	for (auto sender:mOSCSenders) {
		std::cout << sender->address() << ":" << sender->port() << std::endl;
	}
}

void PresetServer::stopServer()
{
	if (mServer) {
		mServer->stop();
		delete mServer;
		mServer = nullptr;
	}
}

void PresetServer::setAddress(std::string address)
{
	mOSCpath = address;
}

std::string PresetServer::getAddress()
{
	return mOSCpath;
}

void PresetServer::changeCallback(int value, void *sender, void *userData)
{
	PresetServer *server = static_cast<PresetServer *>(userData);
	Parameter *parameter = static_cast<Parameter *>(sender);
	server->notifyListeners(server->mOSCpath, value);
}
