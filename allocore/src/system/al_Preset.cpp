
#include <iostream>
#include <fstream>
#include <sstream>

#include "allocore/system/al_Preset.hpp"
#include "allocore/io/al_File.hpp"

using namespace al;


// PresetHandler --------------------------------------------------------------

PresetHandler::PresetHandler(std::string rootDirectory, bool verbose) :
    mRootDir(rootDirectory), mVerbose(verbose)
{
	if (!File::exists(rootDirectory)) {
		if (!Dir::make(rootDirectory, true)) {
			std::cout << "Error creating directory: " << rootDirectory << std::endl;
		}
	}
	loadPresetMap();
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
					if (param->getFullAddress() == address) {
						if (type == "f") {
							param->set(std::stof(value));
							parameterFound = true;
							break;
						}
					}
				}
				if (!parameterFound && mVerbose) {
					std::cout << "Preset in parameter not present: " << address;
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
	for(int i = 0; i < mCallbacks.size(); ++i) {
		if (mCallbacks[i]) {
			mCallbacks[i](4, this, mCallbackUdata[i]);
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

std::vector<std::string> PresetHandler::availablePresets()
{
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
	return presets;
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
		std::vector<std::string> presetList = availablePresets();
		for(int i = 0; i < presetList.size(); ++i) {
			mPresetsMap[i] = presetList[i];
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


// PresetServer ----------------------------------------------------------------



PresetServer::PresetServer(std::string oscAddress, int oscPort) : mServer(nullptr), mPresetHandler(nullptr)
{
	mServer = new osc::Recv(oscPort, oscAddress.c_str());
	if (mServer) {
		mServer->handler(*this);
		mServer->start();
	} else {
		std::cout << "Error starting OSC server." << std::endl;
	}
}

PresetServer::~PresetServer()
{
	if (mServer) {
		mServer->stop();
		delete mServer;
	}
}

void PresetServer::changeCallback(int value, void *sender, void *userData)
{
	PresetServer *server = static_cast<PresetServer *>(userData);
	Parameter *parameter = static_cast<Parameter *>(sender);
	server->notifyListeners("/preset", value);
}
